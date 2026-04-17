// HTTP server for HPW5500

#ifndef CLASS_HPW5500_HTTP_SERVER_H
#define CLASS_HPW5500_HTTP_SERVER_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"

enum class HPW5500HttpMethod : uint8_t {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE_,
    UNKNOWN,
};

struct HPW5500HttpRequest {
    HPW5500HttpMethod method = HPW5500HttpMethod::UNKNOWN;
    char path[128] = { 0 };
    const uint8_t *body = nullptr;
    uint16_t body_length = 0;
    const uint8_t *raw = nullptr;
    uint16_t raw_length = 0;
};

class HPW5500HTTPServer {
    public: typedef void (*HPW5500_http_handler_t)(uint8_t socket, const HPW5500HttpRequest &request, HPW5500HTTPServer *server);

    public: explicit HPW5500HTTPServer(HPW5500 *device = nullptr);

    public: bool begin(uint16_t port, uint8_t socket_count = 1);
    public: void end();
    public: void attach(HPW5500 *device);

    public: bool addRoute(HPW5500HttpMethod method, const char *path, HPW5500_http_handler_t handler);
    public: void setNotFoundHandler(HPW5500_http_handler_t handler);
    public: void registerExampleRoutes();

    public: bool sendResponse(uint8_t socket, uint16_t status_code, const char *content_type, const uint8_t *body, uint16_t body_length, const char *extra_headers = nullptr);
    public: bool sendText(uint8_t socket, uint16_t status_code, const char *text);
    public: bool sendNotFound(uint8_t socket);
    public: bool sendBadRequest(uint8_t socket);

    private: struct Route {
        bool used = false;
        HPW5500HttpMethod method = HPW5500HttpMethod::UNKNOWN;
        const char *path = nullptr;
        HPW5500_http_handler_t handler = nullptr;
    };

    private: static constexpr uint16_t HPW5500_HTTP_MAX_REQUEST = 1024;
    private: static constexpr uint8_t HPW5500_HTTP_MAX_ROUTES = 8;

    private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
    private: void processPacket(uint8_t socket, const HPW5500_packet_t &packet);

    private: bool parseRequest(const uint8_t *buffer, uint16_t length, HPW5500HttpRequest &request);
    private: HPW5500_http_handler_t findHandler(HPW5500HttpMethod method, const char *path) const;

    private: HPW5500 *device = nullptr;
    private: HPW5500_socket_handle_t socket_handle = 0x00;
    private: uint8_t socket_indices[HPW5500_SOCKET_MAX] = { };
    private: uint8_t socket_count = 0;
    private: HPW5500_socket_receive_callback_t previous_callbacks[HPW5500_SOCKET_MAX] = { };

    private: Route routes[HPW5500_HTTP_MAX_ROUTES];
    private: HPW5500_http_handler_t not_found_handler = nullptr;

    private: uint8_t rx_buffer[HPW5500_HTTP_MAX_REQUEST];

    private: static HPW5500HTTPServer *instance_by_socket[HPW5500_SOCKET_MAX];
};

#endif // CLASS_HPW5500_HTTP_SERVER_H
