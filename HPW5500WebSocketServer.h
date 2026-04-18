// WebSocket server for HPW5500

#ifndef CLASS_HPW5500_WEBSOCKET_SERVER_H
#define CLASS_HPW5500_WEBSOCKET_SERVER_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"
#include "HPW5500WebSocketUtils.h"

class HPW5500WebSocketServer {
    public: typedef void (*HPW5500_ws_message_t)(uint8_t socket, bool isBinary, const uint8_t *data, uint16_t length);
    public: typedef void (*HPW5500_ws_conn_t)(uint8_t socket);

    public: explicit HPW5500WebSocketServer(HPW5500 *device = nullptr, uint16_t max_frame = 2048);

    public: bool begin(uint16_t port, uint8_t socket_count = 1);
    public: void end();
    public: void attach(HPW5500 *device);

    public: void onMessage(HPW5500_ws_message_t callback);
    public: void onConnect(HPW5500_ws_conn_t callback);
    public: void onDisconnect(HPW5500_ws_conn_t callback);

    public: bool sendText(uint8_t socket, const char *text);
    public: bool sendBinary(uint8_t socket, const uint8_t *data, uint16_t length);
    public: bool sendPing(uint8_t socket, const uint8_t *data = nullptr, uint16_t length = 0);
    public: bool sendPong(uint8_t socket, const uint8_t *data = nullptr, uint16_t length = 0);
    public: bool sendClose(uint8_t socket);

    public: const char *getPath(uint8_t socket) const;
    public: const char *getCookie(uint8_t socket) const;

    private: struct ConnectionState {
        bool handshake_done = false;
        uint8_t frag_opcode = 0;
        uint16_t frag_len = 0;
        uint8_t frag_buffer[2048];
        char path[48];
        char cookie[64];
    };

    private: static constexpr uint16_t HPW5500_WS_MAX_FRAME = 2048;

    private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
    private: void processPacket(uint8_t socket, const HPW5500_packet_t &packet);

    private: bool handleHandshake(uint8_t socket, const uint8_t *data, uint16_t length);
    private: bool handleFrame(uint8_t socket, const uint8_t *data, uint16_t length);

    private: bool sendFrame(uint8_t socket, uint8_t opcode, const uint8_t *data, uint16_t length, bool mask);

    private: HPW5500 *device = nullptr;
    private: HPW5500_socket_handle_t socket_handle = 0x00;
    private: HPW5500_socket_receive_callback_t previous_callbacks[HPW5500_SOCKET_MAX] = { nullptr };

    private: uint16_t max_frame = HPW5500_WS_MAX_FRAME;
    private: ConnectionState connections[HPW5500_SOCKET_MAX];
    private: HPW5500_ws_message_t message_callback = nullptr;
    private: HPW5500_ws_conn_t connect_callback = nullptr;
    private: HPW5500_ws_conn_t disconnect_callback = nullptr;

    private: static HPW5500WebSocketServer *instance_by_socket[HPW5500_SOCKET_MAX];
};

#endif // CLASS_HPW5500_WEBSOCKET_SERVER_H
