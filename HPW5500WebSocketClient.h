// WebSocket client for HPW5500

#ifndef CLASS_HPW5500_WEBSOCKET_CLIENT_H
#define CLASS_HPW5500_WEBSOCKET_CLIENT_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"
#include "HPW5500WebSocketUtils.h"

class HPW5500WebSocketClient {
    public: typedef void (*HPW5500_ws_message_t)(bool isBinary, const uint8_t *data, uint16_t length);
    public: typedef void (*HPW5500_ws_conn_t)();

    public: explicit HPW5500WebSocketClient(HPW5500 *device = nullptr,
                                            HPW5500_delay_callback_t delay_cb = nullptr,
                                            HPW5500_millis_callback_t millis_cb = nullptr,
                                            uint16_t max_frame = 2048);

    public: void attach(HPW5500 *device);
    public: void setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb = nullptr);

    public: bool connect(const uint8_t *ip, uint16_t port, const char *path = "/", const char *host = "w5500", uint16_t timeout_ms = 5000);
    public: void disconnect();

    public: void onMessage(HPW5500_ws_message_t callback);
    public: void onConnect(HPW5500_ws_conn_t callback);
    public: void onDisconnect(HPW5500_ws_conn_t callback);

    public: bool sendText(const char *text);
    public: bool sendBinary(const uint8_t *data, uint16_t length);
    public: bool sendPing(const uint8_t *data = nullptr, uint16_t length = 0);
    public: bool sendPong(const uint8_t *data = nullptr, uint16_t length = 0);
    public: bool sendClose();

    private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
    private: void processPacket(const HPW5500_packet_t &packet);

    private: bool openSocket(const uint8_t *ip, uint16_t port);
    private: void closeSocket();
    private: bool waitForConnect(uint16_t timeout_ms);
    private: bool sendHandshake(const char *path, const char *host);
    private: bool waitForHandshake(uint16_t timeout_ms);
    private: bool validateHandshake(const uint8_t *data, uint16_t length);
    private: bool handleFrame(const uint8_t *data, uint16_t length);
    private: bool sendFrame(uint8_t opcode, const uint8_t *data, uint16_t length, bool mask);

    private: static constexpr uint16_t HPW5500_WS_MAX_FRAME = 2048;

    private: HPW5500 *device = nullptr;
    private: HPW5500_socket_handle_t socket_handle = 0x00;
    private: uint8_t socket_index = HPW5500_UNASSOCIATED;
    private: HPW5500_socket_receive_callback_t previous_callback = nullptr;

    private: HPW5500_delay_callback_t delay_callback = nullptr;
    private: HPW5500_millis_callback_t millis_callback = nullptr;

    private: uint16_t max_frame = HPW5500_WS_MAX_FRAME;

    private: HPW5500_ws_message_t message_callback = nullptr;
    private: HPW5500_ws_conn_t connect_callback = nullptr;
    private: HPW5500_ws_conn_t disconnect_callback = nullptr;

    private: bool handshake_done = false;
    private: uint8_t frag_opcode = 0;
    private: uint16_t frag_len = 0;
    private: uint8_t frag_buffer[HPW5500_WS_MAX_FRAME];

    private: char sec_key[32] = { 0 };

    private: static HPW5500WebSocketClient *instance_by_socket[HPW5500_SOCKET_MAX];
};

#endif // CLASS_HPW5500_WEBSOCKET_CLIENT_H
