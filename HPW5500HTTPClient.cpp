#include "HPW5500HTTPClient.h"

#include <cstdio>

HPW5500HTTPClient *HPW5500HTTPClient::instance_by_socket[HPW5500_SOCKET_MAX] = { nullptr };

namespace {
    const char *methodToString(HPW5500HttpMethod method) {
        if(method == HPW5500HttpMethod::GET) return "GET";
        if(method == HPW5500HttpMethod::POST) return "POST";
        if(method == HPW5500HttpMethod::PUT) return "PUT";
        if(method == HPW5500HttpMethod::PATCH) return "PATCH";
        if(method == HPW5500HttpMethod::DELETE_) return "DELETE";
        return "GET";
    }
}

HPW5500HTTPClient::HPW5500HTTPClient(HPW5500 *device, HPW5500_delay_callback_t delay_cb, HPW5500_millis_callback_t millis_cb) {
    this->device = device;
    delay_callback = delay_cb;
    millis_callback = millis_cb;
}

void HPW5500HTTPClient::attach(HPW5500 *device) {
    this->device = device;
}

void HPW5500HTTPClient::setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb) {
    millis_callback = millis_cb;
    delay_callback = delay_cb;
}

bool HPW5500HTTPClient::get(const uint8_t *ip, uint16_t port, const char *path, HPW5500HttpResponse &response, uint16_t timeout_ms) {
    return request(HPW5500HttpMethod::GET, ip, port, path, nullptr, 0, response, timeout_ms);
}

bool HPW5500HTTPClient::post(const uint8_t *ip, uint16_t port, const char *path, const uint8_t *body, uint16_t body_length, HPW5500HttpResponse &response, uint16_t timeout_ms) {
    return request(HPW5500HttpMethod::POST, ip, port, path, body, body_length, response, timeout_ms);
}

bool HPW5500HTTPClient::put(const uint8_t *ip, uint16_t port, const char *path, const uint8_t *body, uint16_t body_length, HPW5500HttpResponse &response, uint16_t timeout_ms) {
    return request(HPW5500HttpMethod::PUT, ip, port, path, body, body_length, response, timeout_ms);
}

bool HPW5500HTTPClient::patch(const uint8_t *ip, uint16_t port, const char *path, const uint8_t *body, uint16_t body_length, HPW5500HttpResponse &response, uint16_t timeout_ms) {
    return request(HPW5500HttpMethod::PATCH, ip, port, path, body, body_length, response, timeout_ms);
}

bool HPW5500HTTPClient::del(const uint8_t *ip, uint16_t port, const char *path, HPW5500HttpResponse &response, uint16_t timeout_ms) {
    return request(HPW5500HttpMethod::DELETE_, ip, port, path, nullptr, 0, response, timeout_ms);
}

bool HPW5500HTTPClient::request(HPW5500HttpMethod method,
                                const uint8_t *ip,
                                uint16_t port,
                                const char *path,
                                const uint8_t *body,
                                uint16_t body_length,
                                HPW5500HttpResponse &response,
                                uint16_t timeout_ms) {
    if(device == nullptr || !device->connected()) return false;
    if(ip == nullptr || path == nullptr) return false;

    response = HPW5500HttpResponse();

    if(!openSocket(ip, port)) return false;
    if(!waitForConnect(timeout_ms)) {
        closeSocket();
        return false;
    }

    if(!sendRequest(method, path, body, body_length)) {
        closeSocket();
        return false;
    }

    if(!waitForResponse(timeout_ms)) {
        closeSocket();
        return false;
    }

    bool ok = parseResponse(rx_length, response);
    closeSocket();

    return ok;
}

void HPW5500HTTPClient::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
    if(socket >= HPW5500_SOCKET_MAX) return;

    HPW5500HTTPClient *instance = instance_by_socket[socket];
    if(instance == nullptr) return;

    instance->processPacket(packet);
}

void HPW5500HTTPClient::processPacket(const HPW5500_packet_t &packet) {
    if(packet.length == 0) return;

    uint16_t copy_len = packet.length > HPW5500_HTTP_MAX_RESPONSE ? HPW5500_HTTP_MAX_RESPONSE : packet.length;
    memcpy(rx_buffer, packet.payload, copy_len);
    rx_length = copy_len;
    response_ready = true;
}

bool HPW5500HTTPClient::openSocket(const uint8_t *ip, uint16_t port) {
    if(socket_index != HPW5500_UNASSOCIATED) return true;
    if(device == nullptr) return false;

    HPW5500_socket_handle_t handle = 0x00;
    HPW5500_socket_init_attempt_t result = device->connect(&handle, const_cast<uint8_t *>(ip), port, false, false, 0, 0, HPW5500_PORT_AUTOSELECT);
    if(result != HPW5500_SOCKET_CONNECT_SUCCESS) return false;

    socket_handle = handle;
    socket_index = HPW5500_bitOffset(handle);
    previous_callback = device->swapMessageCallback(socket_index, HPW5500HTTPClient::handlePacket);
    instance_by_socket[socket_index] = this;

    return true;
}

void HPW5500HTTPClient::closeSocket() {
    if(device == nullptr || socket_index == HPW5500_UNASSOCIATED) return;

    device->swapMessageCallback(socket_index, previous_callback);
    instance_by_socket[socket_index] = nullptr;
    device->disconnect(&socket_handle);

    socket_handle = 0x00;
    socket_index = HPW5500_UNASSOCIATED;
    previous_callback = nullptr;
}

bool HPW5500HTTPClient::waitForConnect(uint16_t timeout_ms) {
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

bool HPW5500HTTPClient::sendRequest(HPW5500HttpMethod method, const char *path, const uint8_t *body, uint16_t body_length) {
    if(device == nullptr || path == nullptr) return false;

    char header[256];
    int header_len = std::snprintf(header, sizeof(header),
        "%s %s HTTP/1.1\r\nHost: w5500\r\nContent-Length: %u\r\nConnection: close\r\n\r\n",
        methodToString(method),
        path,
        static_cast<unsigned>(body_length));

    if(header_len <= 0) return false;

    device->writePacket(socket_index, reinterpret_cast<uint8_t *>(header), static_cast<uint16_t>(header_len));
    if(body != nullptr && body_length > 0) device->writePacket(socket_index, const_cast<uint8_t *>(body), body_length);

    return device->sendPacket(socket_index);
}

bool HPW5500HTTPClient::waitForResponse(uint16_t timeout_ms) {
    response_ready = false;
    rx_length = 0;

    HPW5500_millis_t start = millis_callback != nullptr ? millis_callback() : 0;
    uint32_t elapsed = 0;
    const uint16_t step = 10;

    while(true) {
        if(device != nullptr) device->process(true);

        if(response_ready) return true;

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

bool HPW5500HTTPClient::parseResponse(uint16_t length, HPW5500HttpResponse &response) {
    if(length < 12) return false;

    const char *data = reinterpret_cast<const char *>(rx_buffer);
    const char *line_end = std::strstr(data, "\r\n");
    if(line_end == nullptr) return false;

    int status_code = 0;
    if(std::sscanf(data, "HTTP/1.%*c %d", &status_code) != 1) return false;

    const char *body_start = std::strstr(data, "\r\n\r\n");
    if(body_start == nullptr) return false;

    body_start += 4;
    const uint8_t *body_ptr = reinterpret_cast<const uint8_t *>(body_start);
    uint16_t body_len = static_cast<uint16_t>(length - (body_ptr - rx_buffer));

    response.status_code = static_cast<uint16_t>(status_code);
    response.body = body_ptr;
    response.body_length = body_len;

    return true;
}
