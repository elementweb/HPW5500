#include "HPW5500DNS.h"
#include <cstdio>
#include <cstring>

namespace {
    constexpr uint16_t HPW5500_DNS_PORT = 53;
    constexpr uint16_t HPW5500_DNS_MAX_MESSAGE = 512;

    constexpr uint16_t HPW5500_DNS_FLAG_RD = 0x0100;

    constexpr uint16_t HPW5500_DNS_QTYPE_A = 0x0001;
    constexpr uint16_t HPW5500_DNS_QCLASS_IN = 0x0001;

    uint16_t read_be16(const uint8_t *buffer) {
        return static_cast<uint16_t>(buffer[0] << 8 | buffer[1]);
    }

    uint32_t read_be32(const uint8_t *buffer) {
        return (static_cast<uint32_t>(buffer[0]) << 24)
            | (static_cast<uint32_t>(buffer[1]) << 16)
            | (static_cast<uint32_t>(buffer[2]) << 8)
            | static_cast<uint32_t>(buffer[3]);
    }

    void write_be16(uint8_t *buffer, uint16_t value) {
        buffer[0] = static_cast<uint8_t>((value >> 8) & 0xFF);
        buffer[1] = static_cast<uint8_t>(value & 0xFF);
    }

    bool skip_name(const uint8_t *buffer, uint16_t length, uint16_t &offset) {
        if(offset >= length) return false;

        while(offset < length) {
            uint8_t len = buffer[offset];
            if(len == 0) {
                offset++;
                return true;
            }

            if((len & 0xC0) == 0xC0) {
                if(offset + 1 >= length) return false;
                offset += 2;
                return true;
            }

            offset++;
            if(offset + len > length) return false;
            offset += len;
        }

        return false;
    }

    bool is_valid_hostname_char(char c) {
        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return true;
        if(c >= '0' && c <= '9') return true;
        if(c == '-' || c == '.') return true;
        return false;
    }

    void log_msg(void (*callback)(const char *), const char *msg) {
        if(callback != nullptr) callback(msg);
    }

    void log_u16(void (*callback)(const char *), const char *label, uint16_t value) {
        if(callback == nullptr) return;
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%s: %u", label, value);
        callback(buffer);
    }

    void log_hex16(void (*callback)(const char *), const char *label, uint16_t value) {
        if(callback == nullptr) return;
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%s: 0x%04x", label, value);
        callback(buffer);
    }
}

HPW5500DNS *HPW5500DNS::active_instance = nullptr;

HPW5500DNS::HPW5500DNS(HPW5500 *device,
                       HPW5500_delay_callback_t delay_cb,
                       HPW5500_millis_callback_t millis_cb,
                       const uint8_t *default_dns,
                       uint8_t cache_size) {
    this->device = device;
    delay_callback = delay_cb;
    millis_callback = millis_cb;

    if(default_dns != nullptr) memcpy(default_dns_server, default_dns, sizeof(default_dns_server));
    if(cache_size == 0 || cache_size > HPW5500_DNS_CACHE_SIZE) {
        this->cache_size = HPW5500_DNS_CACHE_SIZE;
    } else {
        this->cache_size = cache_size;
    }
}

void HPW5500DNS::attach(HPW5500 *device) {
    this->device = device;
}

void HPW5500DNS::setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb) {
    millis_callback = millis_cb;
    delay_callback = delay_cb;
}

void HPW5500DNS::setServer(const HPW5500_IP_t &dns_server) {
    memcpy(dns_server_ip, dns_server, sizeof(dns_server_ip));
}

void HPW5500DNS::setDebug(void (*callback)(const char *msg)) {
    debug_callback = callback;
}

bool HPW5500DNS::resolve(const char *hostname, HPW5500_IP_t &out_ip, uint16_t timeout_ms, uint8_t retries) {
    if(device == nullptr || !device->connected()) return false;
    if(hostname == nullptr || hostname[0] == '\0') return false;

    if(millis_callback != nullptr && readFromCache(hostname, out_ip)) {
        log_msg(debug_callback, "DNS: cache hit");
        return true;
    }

    bool server_set = dns_server_ip[0] || dns_server_ip[1] || dns_server_ip[2] || dns_server_ip[3];
    if(!server_set) memcpy(dns_server_ip, default_dns_server, sizeof(dns_server_ip));

    server_set = dns_server_ip[0] || dns_server_ip[1] || dns_server_ip[2] || dns_server_ip[3];
    if(!server_set) {
        log_msg(debug_callback, "DNS: server not set");
        return false;
    }

    if(!openSocket()) {
        log_msg(debug_callback, "DNS: socket open failed");
        return false;
    }

    active_instance = this;
    bool resolved = false;

    log_msg(debug_callback, "DNS: resolve start");
    for(uint8_t attempt = 0; attempt < retries && !resolved; attempt++) {
        response_ready = false;
        response_ok = false;
        resolved_ttl = 0;
        memset(resolved_ip, 0, sizeof(resolved_ip));

        transaction_id = static_cast<uint16_t>(0xA550 + attempt);
        if(millis_callback != nullptr) transaction_id ^= static_cast<uint16_t>(millis_callback());

        log_u16(debug_callback, "DNS: attempt", attempt + 1);
        if(!sendQuery(hostname)) {
            log_msg(debug_callback, "DNS: send query failed");
            continue;
        }
        if(!waitForResponse(timeout_ms)) {
            log_msg(debug_callback, "DNS: wait timeout or error");
            continue;
        }

        if(response_ready && response_ok) {
            memcpy(out_ip, resolved_ip, sizeof(resolved_ip));
            resolved = true;
            log_msg(debug_callback, "DNS: resolved ok");

            if(millis_callback != nullptr) storeInCache(hostname, resolved_ip, resolved_ttl);
        }
    }

    closeSocket();
    active_instance = nullptr;

    return resolved;
}

void HPW5500DNS::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
    if(active_instance == nullptr) return;
    if(socket != active_instance->socket_index) return;
    active_instance->processPacket(packet);
}

void HPW5500DNS::processPacket(const HPW5500_packet_t &packet) {
    if(packet.length < 12) return;

    const uint8_t *buffer = packet.payload;

    uint16_t id = read_be16(&buffer[0]);
    if(id != transaction_id) return;

    uint16_t flags = read_be16(&buffer[2]);
    bool is_response = (flags & 0x8000) != 0;
    uint8_t rcode = static_cast<uint8_t>(flags & 0x000F);
    if(!is_response || rcode != 0) {
        if(!is_response) log_msg(debug_callback, "DNS: not a response");
        if(rcode != 0) log_u16(debug_callback, "DNS: rcode", rcode);
        response_ready = true;
        response_ok = false;
        return;
    }

    uint16_t qdcount = read_be16(&buffer[4]);
    uint16_t ancount = read_be16(&buffer[6]);

    uint16_t offset = 12;
    for(uint16_t q = 0; q < qdcount; q++) {
        if(!skip_name(buffer, packet.length, offset)) return;
        if(offset + 4 > packet.length) return;
        offset += 4;
    }

    for(uint16_t a = 0; a < ancount; a++) {
        if(!skip_name(buffer, packet.length, offset)) return;
        if(offset + 10 > packet.length) return;

        uint16_t type = read_be16(&buffer[offset]);
        uint16_t cls = read_be16(&buffer[offset + 2]);
        uint32_t ttl = read_be32(&buffer[offset + 4]);
        uint16_t rdlength = read_be16(&buffer[offset + 8]);
        offset += 10;

        if(offset + rdlength > packet.length) return;

        if(type == HPW5500_DNS_QTYPE_A && cls == HPW5500_DNS_QCLASS_IN && rdlength == 4) {
            memcpy(resolved_ip, &buffer[offset], 4);
            resolved_ttl = ttl;
            response_ready = true;
            response_ok = true;
            log_msg(debug_callback, "DNS: A record found");
            return;
        }

        offset += rdlength;
    }

    response_ready = true;
    response_ok = false;
    log_msg(debug_callback, "DNS: no A record found");
}

bool HPW5500DNS::readFromCache(const char *hostname, HPW5500_IP_t &out_ip) {
    if(millis_callback == nullptr) return false;

    HPW5500_millis_t now = millis_callback();
    for(uint8_t i = 0; i < cache_size; i++) {
        CacheEntry &entry = cache[i];
        if(!entry.valid) continue;
        if(entry.expires_at_ms != 0 && now >= entry.expires_at_ms) {
            entry.valid = false;
            continue;
        }
        if(std::strncmp(entry.host, hostname, HPW5500_DNS_CACHE_HOST_MAX) == 0) {
            memcpy(out_ip, entry.ip, sizeof(entry.ip));
            return true;
        }
    }

    return false;
}

void HPW5500DNS::storeInCache(const char *hostname, const HPW5500_IP_t &ip, uint32_t ttl_seconds) {
    if(millis_callback == nullptr) return;
    if(hostname == nullptr || hostname[0] == '\0') return;

    if(ttl_seconds == 0) return;

    if(std::strlen(hostname) >= HPW5500_DNS_CACHE_HOST_MAX) return;

    HPW5500_millis_t now = millis_callback();
    uint32_t ttl_ms = ttl_seconds * 1000u;
    HPW5500_millis_t expires_at = now + ttl_ms;

    for(uint8_t i = 0; i < cache_size; i++) {
        CacheEntry &entry = cache[i];
        if(entry.valid && std::strncmp(entry.host, hostname, HPW5500_DNS_CACHE_HOST_MAX) == 0) {
            memcpy(entry.ip, ip, sizeof(entry.ip));
            entry.expires_at_ms = expires_at;
            return;
        }
    }

    uint8_t index = cache_next_index % cache_size;
    cache_next_index = static_cast<uint8_t>(cache_next_index + 1);

    CacheEntry &entry = cache[index];
    entry.valid = true;
    std::strncpy(entry.host, hostname, HPW5500_DNS_CACHE_HOST_MAX - 1);
    entry.host[HPW5500_DNS_CACHE_HOST_MAX - 1] = '\0';
    memcpy(entry.ip, ip, sizeof(entry.ip));
    entry.expires_at_ms = expires_at;
}

bool HPW5500DNS::openSocket() {
    if(socket_index != HPW5500_UNASSOCIATED) return true;
    if(device == nullptr) return false;

    HPW5500_socket_handle_t handle = 0x00;
    HPW5500_socket_init_attempt_t result = device->open(&handle, HPW5500_SOCKET_PROTOCOL_UDP, HPW5500_PORT_AUTOSELECT, 1, false);
    if(result != HPW5500_SOCKET_OPEN_SUCCESS && result != HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS) return false;

    socket_handle = handle;
    socket_index = HPW5500_bitOffset(handle);
    previous_callback = device->swapMessageCallback(socket_index, HPW5500DNS::handlePacket);

    return true;
}

void HPW5500DNS::closeSocket() {
    if(device == nullptr || socket_index == HPW5500_UNASSOCIATED) return;

    device->swapMessageCallback(socket_index, previous_callback);
    device->close(&socket_handle);

    socket_handle = 0x00;
    socket_index = HPW5500_UNASSOCIATED;
    previous_callback = nullptr;
}

bool HPW5500DNS::sendQuery(const char *hostname) {
    uint8_t buffer[HPW5500_DNS_MAX_MESSAGE];
    uint16_t length = buildQuery(hostname, buffer, sizeof(buffer));
    if(length == 0) {
        log_msg(debug_callback, "DNS: build query failed");
        return false;
    }

    device->writePacket(socket_index, buffer, length);
    return device->sendPacket(socket_index, dns_server_ip, HPW5500_DNS_PORT);
}

bool HPW5500DNS::waitForResponse(uint16_t timeout_ms) {
    HPW5500_millis_t start = millis_callback != nullptr ? millis_callback() : 0;
    uint32_t elapsed = 0;
    const uint16_t step = 10;

    while(true) {
        if(device != nullptr) device->process(true);

        if(response_ready) return response_ok;

        if(millis_callback != nullptr) {
            HPW5500_millis_t now = millis_callback();
            if(static_cast<HPW5500_millis_t>(now - start) >= timeout_ms) return false;
        } else {
            if(elapsed >= timeout_ms) return false;
            elapsed += step;
        }

        if(delay_callback != nullptr) delay_callback(step);
    }
}

uint16_t HPW5500DNS::buildQuery(const char *hostname, uint8_t *buffer, uint16_t max_len) {
    uint16_t name_len = 0;
    uint16_t label_len = 0;

    for(const char *p = hostname; *p != '\0'; p++) {
        if(!is_valid_hostname_char(*p)) return 0;
        name_len++;
        if(*p == '.') {
            if(label_len == 0) return 0;
            label_len = 0;
        } else {
            label_len++;
            if(label_len > 63) return 0;
        }
        if(name_len > 253) return 0;
    }
    if(label_len == 0) return 0;

    uint16_t needed = 12 + name_len + 2 + 4;
    if(needed > max_len) return 0;

    memset(buffer, 0, max_len);
    write_be16(&buffer[0], transaction_id);
    write_be16(&buffer[2], HPW5500_DNS_FLAG_RD);
    write_be16(&buffer[4], 1);

    log_hex16(debug_callback, "DNS: txid", transaction_id);

    uint16_t offset = 12;
    const char *start = hostname;
    while(*start != '\0') {
        const char *dot = strchr(start, '.');
        uint8_t len = dot ? static_cast<uint8_t>(dot - start) : static_cast<uint8_t>(strlen(start));
        buffer[offset++] = len;
        memcpy(&buffer[offset], start, len);
        offset += len;
        if(!dot) break;
        start = dot + 1;
    }

    buffer[offset++] = 0x00;
    write_be16(&buffer[offset], HPW5500_DNS_QTYPE_A);
    offset += 2;
    write_be16(&buffer[offset], HPW5500_DNS_QCLASS_IN);
    offset += 2;

    return offset;
}
