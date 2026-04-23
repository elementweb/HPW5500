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
    char cookie[64] = { 0 };
    const uint8_t *body = nullptr;
    uint16_t body_length = 0;
    const uint8_t *raw = nullptr;
    uint16_t raw_length = 0;
    uint32_t content_length = 0; // parsed from Content-Length header
};

class HPW5500HTTPServer {
    public: typedef void (*HPW5500_http_handler_t)(uint8_t socket, const HPW5500HttpRequest &request, HPW5500HTTPServer *server);
    // Stream handler — called once per TCP chunk while streaming a large POST body.
    // offset = bytes already delivered before this chunk; done = true on last chunk.
    public: typedef void (*HPW5500_http_stream_t)(uint8_t socket, const uint8_t *data, uint16_t len, uint32_t offset, uint32_t total, bool done, HPW5500HTTPServer *server);

    // Opaque token returned by schedule_fn and passed back to cancel_fn.
    // schedule_fn(delay_ms, cb, arg): schedules cb(arg) after delay_ms; returns token.
    // cancel_fn(token): cancels a pending timer. No-op if token is nullptr or already fired.
    typedef void *HPW5500_http_timer_token_t;
    typedef HPW5500_http_timer_token_t (*HPW5500_http_schedule_t)(uint32_t delay_ms, void(*cb)(void *), void *arg);
    typedef void (*HPW5500_http_cancel_t)(HPW5500_http_timer_token_t token);

    public: explicit HPW5500HTTPServer(HPW5500 *device = nullptr);

    public: bool begin(uint16_t port, uint8_t socket_count = 1, uint32_t idle_ms = 3000);
    public: bool begin(uint16_t port, uint8_t socket_count, uint32_t idle_ms, uint8_t socket_mask);
    public: void end();
    public: void attach(HPW5500 *device);
    public: void setTimerCallbacks(HPW5500_http_schedule_t schedule, HPW5500_http_cancel_t cancel);

    public: bool addRoute(HPW5500HttpMethod method, const char *path, HPW5500_http_handler_t handler);
    public: void setNotFoundHandler(HPW5500_http_handler_t handler);
    public: void registerExampleRoutes();

    public: bool sendResponse(uint8_t socket, uint16_t status_code, const char *content_type, const uint8_t *body, uint16_t body_length, const char *extra_headers = nullptr, bool keepAlive = false);
    public: bool sendText(uint8_t socket, uint16_t status_code, const char *text);
    public: bool sendNotFound(uint8_t socket);
    public: bool sendBadRequest(uint8_t socket);
    // Register a streaming handler for large POST bodies. Call from inside a route handler
    // BEFORE returning. Subsequent TCP data on this socket bypasses HTTP parsing and goes
    // directly to handler until 'total' bytes have been delivered.
    public: void beginStream(uint8_t socket, uint32_t total, HPW5500_http_stream_t handler);

    private: struct Route {
        bool used = false;
        HPW5500HttpMethod method = HPW5500HttpMethod::UNKNOWN;
        const char *path = nullptr;
        HPW5500_http_handler_t handler = nullptr;
    };

    private: static constexpr uint16_t HPW5500_HTTP_MAX_REQUEST = 1024;
    private: static constexpr uint8_t  HPW5500_HTTP_MAX_ROUTES  = 16;

    private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
    private: static void onIdleTimer(void *arg);
    private: void processPacket(uint8_t socket, const HPW5500_packet_t &packet);

    private: bool parseRequest(const uint8_t *buffer, uint16_t length, HPW5500HttpRequest &request);
    private: HPW5500_http_handler_t findHandler(HPW5500HttpMethod method, const char *path) const;

    private: void cancelIdleTimer(uint8_t socket);
    private: void scheduleIdleTimer(uint8_t socket);

    private: HPW5500 *device = nullptr;
    private: HPW5500_socket_handle_t socket_handle = 0x00;
    private: uint8_t socket_indices[HPW5500_SOCKET_MAX] = { };
    private: uint8_t socket_count = 0;
    private: HPW5500_socket_receive_callback_t previous_callbacks[HPW5500_SOCKET_MAX] = { };

    private: Route routes[HPW5500_HTTP_MAX_ROUTES];
    private: HPW5500_http_handler_t not_found_handler = nullptr;

    private: uint8_t rx_buffer[HPW5500_HTTP_MAX_REQUEST];

    private: uint32_t _idle_ms = 3000;
    private: HPW5500_http_schedule_t _schedule_fn = nullptr;
    private: HPW5500_http_cancel_t   _cancel_fn   = nullptr;
    private: HPW5500_http_timer_token_t idle_tokens[HPW5500_SOCKET_MAX] = { };

    private: struct SocketStream {
        bool active = false;
        HPW5500_http_stream_t handler = nullptr;
        uint32_t total = 0;
        uint32_t received = 0;
    };
    private: SocketStream streams[HPW5500_SOCKET_MAX];

    private: static HPW5500HTTPServer *instance_by_socket[HPW5500_SOCKET_MAX];
};

#endif // CLASS_HPW5500_HTTP_SERVER_H
