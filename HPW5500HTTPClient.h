// HTTP client for HPW5500

#ifndef CLASS_HPW5500_HTTP_CLIENT_H
#define CLASS_HPW5500_HTTP_CLIENT_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"
#include "HPW5500HTTPServer.h"

struct HPW5500HttpResponse {
    uint16_t status_code = 0;
    const uint8_t *body = nullptr;
    uint16_t body_length = 0;
};

class HPW5500HTTPClient {
    public: explicit HPW5500HTTPClient(HPW5500 *device = nullptr,
                                       HPW5500_delay_callback_t delay_cb = nullptr,
                                       HPW5500_millis_callback_t millis_cb = nullptr);

    public: void attach(HPW5500 *device);
    public: void setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb = nullptr);

    public: bool get(const uint8_t *ip, uint16_t port, const char *path, HPW5500HttpResponse &response, uint16_t timeout_ms = 2000);
    public: bool post(const uint8_t *ip, uint16_t port, const char *path, const uint8_t *body, uint16_t body_length, HPW5500HttpResponse &response, uint16_t timeout_ms = 2000);
    public: bool put(const uint8_t *ip, uint16_t port, const char *path, const uint8_t *body, uint16_t body_length, HPW5500HttpResponse &response, uint16_t timeout_ms = 2000);
    public: bool patch(const uint8_t *ip, uint16_t port, const char *path, const uint8_t *body, uint16_t body_length, HPW5500HttpResponse &response, uint16_t timeout_ms = 2000);
    public: bool del(const uint8_t *ip, uint16_t port, const char *path, HPW5500HttpResponse &response, uint16_t timeout_ms = 2000);

    private: bool request(HPW5500HttpMethod method,
                          const uint8_t *ip,
                          uint16_t port,
                          const char *path,
                          const uint8_t *body,
                          uint16_t body_length,
                          HPW5500HttpResponse &response,
                          uint16_t timeout_ms);

    private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
    private: void processPacket(const HPW5500_packet_t &packet);

    private: bool openSocket(const uint8_t *ip, uint16_t port);
    private: void closeSocket();
    private: bool waitForConnect(uint16_t timeout_ms);
    private: bool sendRequest(HPW5500HttpMethod method, const char *path, const uint8_t *body, uint16_t body_length);
    private: bool waitForResponse(uint16_t timeout_ms);
    private: bool parseResponse(uint16_t length, HPW5500HttpResponse &response);

    private: HPW5500 *device = nullptr;
    private: HPW5500_socket_handle_t socket_handle = 0x00;
    private: uint8_t socket_index = HPW5500_UNASSOCIATED;
    private: HPW5500_socket_receive_callback_t previous_callback = nullptr;

    private: HPW5500_delay_callback_t delay_callback = nullptr;
    private: HPW5500_millis_callback_t millis_callback = nullptr;

    private: static constexpr uint16_t HPW5500_HTTP_MAX_RESPONSE = 1024;
    private: uint8_t rx_buffer[HPW5500_HTTP_MAX_RESPONSE];
    private: uint16_t rx_length = 0;
    private: bool response_ready = false;

    private: static HPW5500HTTPClient *instance_by_socket[HPW5500_SOCKET_MAX];
};

#endif // CLASS_HPW5500_HTTP_CLIENT_H
