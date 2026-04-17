#include "HPW5500HTTPServer.h"

#include <cstdio>

HPW5500HTTPServer *HPW5500HTTPServer::instance_by_socket[HPW5500_SOCKET_MAX] = { nullptr };

namespace {
    constexpr const char *HPW5500_HTTP_STATUS_TEXT_200 = "OK";
    constexpr const char *HPW5500_HTTP_STATUS_TEXT_400 = "Bad Request";
    constexpr const char *HPW5500_HTTP_STATUS_TEXT_404 = "Not Found";
    constexpr const char *HPW5500_HTTP_STATUS_TEXT_500 = "Server Error";

    const char *statusText(uint16_t status) {
        if(status == 200) return HPW5500_HTTP_STATUS_TEXT_200;
        if(status == 400) return HPW5500_HTTP_STATUS_TEXT_400;
        if(status == 404) return HPW5500_HTTP_STATUS_TEXT_404;
        if(status == 500) return HPW5500_HTTP_STATUS_TEXT_500;
        return "";
    }

    HPW5500HttpMethod parseMethod(const char *method) {
        if(std::strcmp(method, "GET") == 0) return HPW5500HttpMethod::GET;
        if(std::strcmp(method, "POST") == 0) return HPW5500HttpMethod::POST;
        if(std::strcmp(method, "PUT") == 0) return HPW5500HttpMethod::PUT;
        if(std::strcmp(method, "PATCH") == 0) return HPW5500HttpMethod::PATCH;
        if(std::strcmp(method, "DELETE") == 0) return HPW5500HttpMethod::DELETE_;
        return HPW5500HttpMethod::UNKNOWN;
    }
}

HPW5500HTTPServer::HPW5500HTTPServer(HPW5500 *device) {
    this->device = device;
}

bool HPW5500HTTPServer::begin(uint16_t port, uint8_t count) {
    if(device == nullptr || !device->connected()) return false;
    if(count < 1 || count > HPW5500_SOCKET_MAX) count = 1;

    HPW5500_socket_handle_t handle = 0x00;
    HPW5500_socket_init_attempt_t result = device->open(&handle, HPW5500_SOCKET_PROTOCOL_TCP, port, count, true);
    if(result != HPW5500_SOCKET_OPEN_SUCCESS && result != HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS) return false;

    socket_handle = handle;
    socket_count = 0;

    for(uint8_t s = 0; s < HPW5500_SOCKET_MAX; s++) {
        if(!((handle >> s) & 0x01)) continue;
        socket_indices[socket_count] = s;
        previous_callbacks[socket_count] = device->swapMessageCallback(s, HPW5500HTTPServer::handlePacket);
        instance_by_socket[s] = this;
        socket_count++;
    }

    return true;
}

void HPW5500HTTPServer::end() {
    if(device == nullptr || socket_count == 0) return;

    for(uint8_t i = 0; i < socket_count; i++) {
        uint8_t s = socket_indices[i];
        device->swapMessageCallback(s, previous_callbacks[i]);
        instance_by_socket[s] = nullptr;
    }

    device->close(&socket_handle);
    socket_handle = 0x00;
    socket_count = 0;
}

void HPW5500HTTPServer::attach(HPW5500 *device) {
    this->device = device;
}

bool HPW5500HTTPServer::addRoute(HPW5500HttpMethod method, const char *path, HPW5500_http_handler_t handler) {
    if(path == nullptr || handler == nullptr) return false;

    for(uint8_t i = 0; i < HPW5500_HTTP_MAX_ROUTES; i++) {
        if(!routes[i].used) {
            routes[i].used = true;
            routes[i].method = method;
            routes[i].path = path;
            routes[i].handler = handler;
            return true;
        }
    }

    return false;
}

void HPW5500HTTPServer::setNotFoundHandler(HPW5500_http_handler_t handler) {
    not_found_handler = handler;
}

void HPW5500HTTPServer::registerExampleRoutes() {
    addRoute(HPW5500HttpMethod::GET, "/health", [](uint8_t socket, const HPW5500HttpRequest &, HPW5500HTTPServer *server) {
        server->sendText(socket, 200, "OK");
    });

    addRoute(HPW5500HttpMethod::GET, "/info", [](uint8_t socket, const HPW5500HttpRequest &, HPW5500HTTPServer *server) {
        server->sendText(socket, 200, "HPW5500 HTTP Server");
    });

    addRoute(HPW5500HttpMethod::POST, "/echo", [](uint8_t socket, const HPW5500HttpRequest &request, HPW5500HTTPServer *server) {
        server->sendResponse(socket, 200, "text/plain", request.body, request.body_length);
    });
}

bool HPW5500HTTPServer::sendResponse(uint8_t socket, uint16_t status_code, const char *content_type, const uint8_t *body, uint16_t body_length, const char *extra_headers) {
    if(device == nullptr) return false;

    const char *status_text = statusText(status_code);
    char header[384];
    int header_len = std::snprintf(header, sizeof(header),
        "HTTP/1.1 %u %s\r\nContent-Length: %u\r\nContent-Type: %s\r\n%sConnection: close\r\n\r\n",
        status_code,
        status_text,
        static_cast<unsigned>(body_length),
        content_type != nullptr ? content_type : "text/plain",
        extra_headers != nullptr ? extra_headers : "");

    if(header_len <= 0) return false;

    // Determine max chunk size based on socket TX buffer
    uint16_t buf_size = device->bufferSize(socket);
    if(buf_size == 0) buf_size = 2048;

    // Send header first
    device->writePacket(socket, reinterpret_cast<uint8_t *>(header), static_cast<uint16_t>(header_len));

    // Calculate how much body fits alongside the header in the first chunk
    uint16_t first_body = 0;
    uint16_t remaining_buf = (buf_size > static_cast<uint16_t>(header_len)) ? static_cast<uint16_t>(buf_size - header_len) : 0;

    if(body != nullptr && body_length > 0) {
        first_body = (body_length < remaining_buf) ? body_length : remaining_buf;
        if(first_body > 0) device->writePacket(socket, const_cast<uint8_t *>(body), first_body);
    }

    device->sendPacket(socket);

    // Send remaining body in chunks if needed
    uint16_t sent = first_body;
    while(sent < body_length) {
        if(!device->waitForSend(socket)) break;

        uint16_t chunk = body_length - sent;
        if(chunk > buf_size) chunk = buf_size;
        device->writePacket(socket, const_cast<uint8_t *>(body + sent), chunk);
        device->sendPacket(socket);
        sent += chunk;
    }

    device->waitForSend(socket);

    HPW5500_socket_handle_t handle = HPW5500_SOCKET_AS_HANDLE(socket);
    device->disconnect(&handle);

    return true;
}

bool HPW5500HTTPServer::sendText(uint8_t socket, uint16_t status_code, const char *text) {
    if(text == nullptr) text = "";
    return sendResponse(socket, status_code, "text/plain", reinterpret_cast<const uint8_t *>(text), static_cast<uint16_t>(std::strlen(text)));
}

bool HPW5500HTTPServer::sendNotFound(uint8_t socket) {
    return sendText(socket, 404, "Not Found");
}

bool HPW5500HTTPServer::sendBadRequest(uint8_t socket) {
    return sendText(socket, 400, "Bad Request");
}

void HPW5500HTTPServer::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
    if(socket >= HPW5500_SOCKET_MAX) return;

    HPW5500HTTPServer *instance = instance_by_socket[socket];
    if(instance == nullptr) return;

    instance->processPacket(socket, packet);
}

void HPW5500HTTPServer::processPacket(uint8_t socket, const HPW5500_packet_t &packet) {
    if(packet.length == 0 || packet.length > HPW5500_HTTP_MAX_REQUEST) {
        sendBadRequest(socket);
        return;
    }

    memcpy(rx_buffer, packet.payload, packet.length);

    HPW5500HttpRequest request;
    if(!parseRequest(rx_buffer, packet.length, request)) {
        sendBadRequest(socket);
        return;
    }

    HPW5500_http_handler_t handler = findHandler(request.method, request.path);
    if(handler == nullptr) {
        if(not_found_handler != nullptr) {
            not_found_handler(socket, request, this);
            return;
        }
        sendNotFound(socket);
        return;
    }

    handler(socket, request, this);
}

bool HPW5500HTTPServer::parseRequest(const uint8_t *buffer, uint16_t length, HPW5500HttpRequest &request) {
    request.raw = buffer;
    request.raw_length = length;

    const char *data = reinterpret_cast<const char *>(buffer);
    const char *line_end = static_cast<const char *>(std::memchr(data, '\n', length));
    if(line_end == nullptr) return false;

    char method_buf[8] = { 0 };
    char path_buf[128] = { 0 };

    int parsed = std::sscanf(data, "%7s %127s", method_buf, path_buf);
    if(parsed < 2) return false;

    request.method = parseMethod(method_buf);
    if(request.method == HPW5500HttpMethod::UNKNOWN) return false;

    char *query = std::strchr(path_buf, '?');
    if(query != nullptr) *query = '\0';

    std::strncpy(request.path, path_buf, sizeof(request.path) - 1);

    const char *body_start = std::strstr(data, "\r\n\r\n");
    if(body_start == nullptr) {
        request.body = nullptr;
        request.body_length = 0;
        return true;
    }

    body_start += 4;
    const uint8_t *body_ptr = reinterpret_cast<const uint8_t *>(body_start);
    uint16_t body_len = static_cast<uint16_t>(length - (body_ptr - buffer));

    request.body = body_ptr;
    request.body_length = body_len;

    return true;
}

HPW5500HTTPServer::HPW5500_http_handler_t HPW5500HTTPServer::findHandler(HPW5500HttpMethod method, const char *path) const {
    if(path == nullptr) return nullptr;

    for(uint8_t i = 0; i < HPW5500_HTTP_MAX_ROUTES; i++) {
        if(!routes[i].used) continue;
        if(routes[i].method != method) continue;
        if(std::strcmp(routes[i].path, path) == 0) return routes[i].handler;
    }

    return nullptr;
}
