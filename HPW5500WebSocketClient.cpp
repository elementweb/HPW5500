#include "HPW5500WebSocketClient.h"

#include <cstdio>

HPW5500WebSocketClient *HPW5500WebSocketClient::instance_by_socket[HPW5500_SOCKET_MAX] = { nullptr };

namespace {
    constexpr const char *HPW5500_WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    void random_key(char *out, uint16_t out_max, HPW5500_millis_callback_t millis_cb) {
        uint8_t seed[16];
        uint32_t base = millis_cb != nullptr ? millis_cb() : 0x12345678;
        for(uint8_t i = 0; i < sizeof(seed); i++) {
            base = base * 1103515245u + 12345u;
            seed[i] = static_cast<uint8_t>(base >> 24);
        }

        HPW5500_ws_base64_encode(seed, sizeof(seed), out, out_max);
    }

    uint16_t build_accept_key(const char *client_key, char *out, uint16_t out_max) {
        if(client_key == nullptr || out == nullptr) return 0;

        char key_buffer[128];
        std::snprintf(key_buffer, sizeof(key_buffer), "%s%s", client_key, HPW5500_WS_GUID);

        uint8_t sha1[20];
        if(!HPW5500_ws_sha1(reinterpret_cast<const uint8_t *>(key_buffer), static_cast<uint32_t>(std::strlen(key_buffer)), sha1)) return 0;

        return HPW5500_ws_base64_encode(sha1, sizeof(sha1), out, out_max);
    }

    const char *find_header(const char *data, const char *key) {
        const char *pos = std::strstr(data, key);
        if(pos == nullptr) return nullptr;
        pos += std::strlen(key);
        while(*pos == ' ') pos++;
        return pos;
    }
}

HPW5500WebSocketClient::HPW5500WebSocketClient(HPW5500 *device, HPW5500_delay_callback_t delay_cb, HPW5500_millis_callback_t millis_cb, uint16_t max_frame) {
    this->device = device;
    delay_callback = delay_cb;
    millis_callback = millis_cb;

    if(max_frame == 0 || max_frame > HPW5500_WS_MAX_FRAME) {
        this->max_frame = HPW5500_WS_MAX_FRAME;
    } else {
        this->max_frame = max_frame;
    }
}

void HPW5500WebSocketClient::attach(HPW5500 *device) {
    this->device = device;
}

void HPW5500WebSocketClient::setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb) {
    millis_callback = millis_cb;
    delay_callback = delay_cb;
}

bool HPW5500WebSocketClient::connect(const uint8_t *ip, uint16_t port, const char *path, const char *host, uint16_t timeout_ms) {
    if(device == nullptr || !device->connected()) return false;
    if(ip == nullptr || path == nullptr || host == nullptr) return false;

    if(!openSocket(ip, port)) return false;
    if(!waitForConnect(timeout_ms)) {
        closeSocket();
        return false;
    }

    if(!sendHandshake(path, host)) {
        closeSocket();
        return false;
    }

    if(!waitForHandshake(timeout_ms)) {
        closeSocket();
        return false;
    }

    handshake_done = true;
    if(connect_callback != nullptr) connect_callback();
    return true;
}

void HPW5500WebSocketClient::disconnect() {
    sendClose();
    closeSocket();
    if(disconnect_callback != nullptr) disconnect_callback();
}

void HPW5500WebSocketClient::onMessage(HPW5500_ws_message_t callback) {
    message_callback = callback;
}

void HPW5500WebSocketClient::onConnect(HPW5500_ws_conn_t callback) {
    connect_callback = callback;
}

void HPW5500WebSocketClient::onDisconnect(HPW5500_ws_conn_t callback) {
    disconnect_callback = callback;
}

bool HPW5500WebSocketClient::sendText(const char *text) {
    if(text == nullptr) text = "";
    return sendFrame(0x1, reinterpret_cast<const uint8_t *>(text), static_cast<uint16_t>(std::strlen(text)), true);
}

bool HPW5500WebSocketClient::sendBinary(const uint8_t *data, uint16_t length) {
    return sendFrame(0x2, data, length, true);
}

bool HPW5500WebSocketClient::sendPing(const uint8_t *data, uint16_t length) {
    return sendFrame(0x9, data, length, true);
}

bool HPW5500WebSocketClient::sendPong(const uint8_t *data, uint16_t length) {
    return sendFrame(0xA, data, length, true);
}

bool HPW5500WebSocketClient::sendClose() {
    return sendFrame(0x8, nullptr, 0, true);
}

void HPW5500WebSocketClient::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
    if(socket >= HPW5500_SOCKET_MAX) return;

    HPW5500WebSocketClient *instance = instance_by_socket[socket];
    if(instance == nullptr) return;

    instance->processPacket(packet);
}

void HPW5500WebSocketClient::processPacket(const HPW5500_packet_t &packet) {
    if(packet.length == 0) return;

    if(!handshake_done) {
        if(validateHandshake(packet.payload, packet.length)) {
            handshake_done = true;
            if(connect_callback != nullptr) connect_callback();
        }
        return;
    }

    handleFrame(packet.payload, packet.length);
}

bool HPW5500WebSocketClient::openSocket(const uint8_t *ip, uint16_t port) {
    if(socket_index != HPW5500_UNASSOCIATED) return true;
    if(device == nullptr) return false;

    HPW5500_socket_handle_t handle = 0x00;
    HPW5500_socket_init_attempt_t result = device->connect(&handle, const_cast<uint8_t *>(ip), port, false, false, 0, 0, HPW5500_PORT_AUTOSELECT);
    if(result != HPW5500_SOCKET_CONNECT_SUCCESS) return false;

    socket_handle = handle;
    socket_index = HPW5500_bitOffset(handle);
    previous_callback = device->swapMessageCallback(socket_index, HPW5500WebSocketClient::handlePacket);
    instance_by_socket[socket_index] = this;

    return true;
}

void HPW5500WebSocketClient::closeSocket() {
    if(device == nullptr || socket_index == HPW5500_UNASSOCIATED) return;

    device->swapMessageCallback(socket_index, previous_callback);
    instance_by_socket[socket_index] = nullptr;
    device->disconnect(&socket_handle);

    socket_handle = 0x00;
    socket_index = HPW5500_UNASSOCIATED;
    previous_callback = nullptr;
    handshake_done = false;
    frag_len = 0;
    frag_opcode = 0;
}

bool HPW5500WebSocketClient::waitForConnect(uint16_t timeout_ms) {
    HPW5500_millis_t start = millis_callback != nullptr ? millis_callback() : 0;
    uint32_t elapsed = 0;
    const uint16_t step = 10;

    while(true) {
        if(device != nullptr) device->process(true);

        if(device->socketStatus(socket_index) == HPW5500_SOCKET_STATUS::ESTABLISHED) return true;

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

bool HPW5500WebSocketClient::sendHandshake(const char *path, const char *host) {
    random_key(sec_key, sizeof(sec_key), millis_callback);

    char header[256];
    int header_len = std::snprintf(header, sizeof(header),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n",
        path,
        host,
        sec_key);

    if(header_len <= 0) return false;

    device->writePacket(socket_index, reinterpret_cast<uint8_t *>(header), static_cast<uint16_t>(header_len));
    return device->sendPacket(socket_index);
}

bool HPW5500WebSocketClient::waitForHandshake(uint16_t timeout_ms) {
    HPW5500_millis_t start = millis_callback != nullptr ? millis_callback() : 0;
    uint32_t elapsed = 0;
    const uint16_t step = 10;

    while(true) {
        if(device != nullptr) device->process(true);

        if(handshake_done) return true;

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

bool HPW5500WebSocketClient::validateHandshake(const uint8_t *data, uint16_t length) {
    const char *text = reinterpret_cast<const char *>(data);
    if(std::strstr(text, "HTTP/1.1 101") == nullptr) return false;

    const char *accept = find_header(text, "Sec-WebSocket-Accept:");
    if(accept == nullptr) return false;

    const char *end = std::strstr(accept, "\r\n");
    if(end == nullptr) return false;

    char accept_value[64];
    uint16_t accept_len = static_cast<uint16_t>(end - accept);
    if(accept_len + 1 > sizeof(accept_value)) return false;
    std::memcpy(accept_value, accept, accept_len);
    accept_value[accept_len] = '\0';

    char expected[64] = { 0 };
    if(build_accept_key(sec_key, expected, sizeof(expected)) == 0) return false;

    return std::strcmp(accept_value, expected) == 0;
}

bool HPW5500WebSocketClient::handleFrame(const uint8_t *data, uint16_t length) {
    uint16_t offset = 0;

    while(offset + 2 <= length) {
        uint8_t b0 = data[offset++];
        uint8_t b1 = data[offset++];

        bool fin = (b0 & 0x80) != 0;
        uint8_t opcode = b0 & 0x0F;
        bool masked = (b1 & 0x80) != 0;
        uint64_t payload_len = b1 & 0x7F;

        if(payload_len == 126) {
            if(offset + 2 > length) return false;
            payload_len = static_cast<uint16_t>(data[offset] << 8 | data[offset + 1]);
            offset += 2;
        } else if(payload_len == 127) {
            return false;
        }

        if(payload_len > max_frame) return false;

        uint8_t mask_key[4] = { 0, 0, 0, 0 };
        if(masked) {
            if(offset + 4 > length) return false;
            std::memcpy(mask_key, &data[offset], 4);
            offset += 4;
        }

        if(offset + payload_len > length) return false;

        uint8_t payload[HPW5500_WS_MAX_FRAME];
        if(masked) {
            for(uint16_t i = 0; i < payload_len; i++) {
                payload[i] = data[offset + i] ^ mask_key[i % 4];
            }
        } else {
            std::memcpy(payload, &data[offset], payload_len);
        }
        offset += static_cast<uint16_t>(payload_len);

        if(opcode == 0x8) {
            sendClose();
            closeSocket();
            if(disconnect_callback != nullptr) disconnect_callback();
            return true;
        }

        if(opcode == 0x9) {
            sendPong(payload, static_cast<uint16_t>(payload_len));
            continue;
        }

        if(opcode == 0xA) continue;

        uint8_t message_opcode = opcode;
        if(opcode == 0x0) {
            if(frag_opcode == 0) return false;
            message_opcode = frag_opcode;
        } else if(!fin) {
            frag_opcode = opcode;
        }

        if(!fin) {
            if(frag_len + payload_len > max_frame) return false;
            std::memcpy(frag_buffer + frag_len, payload, payload_len);
            frag_len += static_cast<uint16_t>(payload_len);
            continue;
        }

        const uint8_t *message_ptr = payload;
        uint16_t message_len = static_cast<uint16_t>(payload_len);

        if(frag_opcode != 0) {
            if(frag_len + payload_len > max_frame) return false;
            std::memcpy(frag_buffer + frag_len, payload, payload_len);
            frag_len += static_cast<uint16_t>(payload_len);
            message_ptr = frag_buffer;
            message_len = frag_len;
            frag_len = 0;
            frag_opcode = 0;
        }

        if(message_callback != nullptr) message_callback(message_opcode == 0x2, message_ptr, message_len);
    }

    return true;
}

bool HPW5500WebSocketClient::sendFrame(uint8_t opcode, const uint8_t *data, uint16_t length, bool mask) {
    if(device == nullptr) return false;
    if(length > max_frame) return false;

    uint8_t header[10];
    uint16_t header_len = 0;

    header[header_len++] = static_cast<uint8_t>(0x80 | (opcode & 0x0F));

    if(length <= 125) {
        header[header_len++] = static_cast<uint8_t>((mask ? 0x80 : 0x00) | length);
    } else {
        header[header_len++] = static_cast<uint8_t>((mask ? 0x80 : 0x00) | 126);
        header[header_len++] = static_cast<uint8_t>((length >> 8) & 0xFF);
        header[header_len++] = static_cast<uint8_t>(length & 0xFF);
    }

    uint8_t mask_key[4] = { 0, 0, 0, 0 };
    if(mask) {
        HPW5500_millis_t seed = millis_callback != nullptr ? millis_callback() : 0x12345678;
        mask_key[0] = static_cast<uint8_t>(seed & 0xFF);
        mask_key[1] = static_cast<uint8_t>((seed >> 8) & 0xFF);
        mask_key[2] = static_cast<uint8_t>((seed >> 16) & 0xFF);
        mask_key[3] = static_cast<uint8_t>((seed >> 24) & 0xFF);
        std::memcpy(header + header_len, mask_key, 4);
        header_len += 4;
    }

    device->writePacket(socket_index, header, header_len);

    if(data != nullptr && length > 0) {
        if(mask) {
            uint8_t masked[HPW5500_WS_MAX_FRAME];
            for(uint16_t i = 0; i < length; i++) masked[i] = data[i] ^ mask_key[i % 4];
            device->writePacket(socket_index, masked, length);
        } else {
            device->writePacket(socket_index, const_cast<uint8_t *>(data), length);
        }
    }

    return device->sendPacket(socket_index);
}
