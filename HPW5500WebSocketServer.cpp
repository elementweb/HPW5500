#include "HPW5500WebSocketServer.h"

#include <cstdio>

HPW5500WebSocketServer *HPW5500WebSocketServer::instance_by_socket[HPW5500_SOCKET_MAX] = { nullptr };

namespace {
    constexpr const char *HPW5500_WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    uint16_t build_accept_key(const char *client_key, char *out, uint16_t out_max) {
        if(client_key == nullptr || out == nullptr) return 0;

        char key_buffer[128];
        std::snprintf(key_buffer, sizeof(key_buffer), "%s%s", client_key, HPW5500_WS_GUID);

        uint8_t sha1[20];
        if(!HPW5500_ws_sha1(reinterpret_cast<const uint8_t *>(key_buffer), static_cast<uint32_t>(std::strlen(key_buffer)), sha1)) return 0;

        return HPW5500_ws_base64_encode(sha1, sizeof(sha1), out, out_max);
    }

    bool parse_header_value(const char *data, const char *key, char *out, uint16_t out_max) {
        const char *pos = std::strstr(data, key);
        if(pos == nullptr) return false;

        pos += std::strlen(key);
        while(*pos == ' ') pos++;

        const char *end = std::strstr(pos, "\r\n");
        if(end == nullptr) return false;

        uint16_t len = static_cast<uint16_t>(end - pos);
        if(len + 1 > out_max) return false;

        std::memcpy(out, pos, len);
        out[len] = '\0';

        return true;
    }
}

HPW5500WebSocketServer::HPW5500WebSocketServer(HPW5500 *device, uint16_t max_frame) {
    this->device = device;
    if(max_frame == 0 || max_frame > HPW5500_WS_MAX_FRAME) {
        this->max_frame = HPW5500_WS_MAX_FRAME;
    } else {
        this->max_frame = max_frame;
    }
}

bool HPW5500WebSocketServer::begin(uint16_t port, uint8_t socket_count) {
    if(device == nullptr || !device->connected()) return false;
    if(socket_count == 0) return false;

    HPW5500_socket_handle_t handle = 0x00;
    HPW5500_socket_init_attempt_t result = device->open(&handle, HPW5500_SOCKET_PROTOCOL_TCP, port, socket_count, true);
    if(result != HPW5500_SOCKET_OPEN_SUCCESS && result != HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS) return false;

    socket_handle = handle;
    for(uint8_t socket = 0; socket < HPW5500_SOCKET_MAX; socket++) {
        if(((handle >> socket) & 0x1) != 0x1) continue;
        previous_callbacks[socket] = device->swapMessageCallback(socket, HPW5500WebSocketServer::handlePacket);
        instance_by_socket[socket] = this;
        connections[socket] = ConnectionState();
    }

    // Register disconnect handler to reset connection state when TCP drops
    device->onDisconnect(&socket_handle, [](uint8_t socket) {
        if(socket >= HPW5500_SOCKET_MAX) return;
        HPW5500WebSocketServer *inst = instance_by_socket[socket];
        if(inst == nullptr) return;
        // Only fire callback if handshake was done and not already cleaned up by close frame handler
        if(inst->connections[socket].handshake_done) {
            if(inst->disconnect_callback != nullptr) inst->disconnect_callback(socket);
        }
        inst->connections[socket] = ConnectionState();
    });

    return true;
}

void HPW5500WebSocketServer::end() {
    if(device == nullptr || socket_handle == 0x00) return;

    for(uint8_t socket = 0; socket < HPW5500_SOCKET_MAX; socket++) {
        if(((socket_handle >> socket) & 0x1) != 0x1) continue;
        device->swapMessageCallback(socket, previous_callbacks[socket]);
        previous_callbacks[socket] = nullptr;
        instance_by_socket[socket] = nullptr;
        connections[socket] = ConnectionState();
    }

    device->close(&socket_handle);
    socket_handle = 0x00;
}

void HPW5500WebSocketServer::attach(HPW5500 *device) {
    this->device = device;
}

void HPW5500WebSocketServer::onMessage(HPW5500_ws_message_t callback) {
    message_callback = callback;
}

void HPW5500WebSocketServer::onConnect(HPW5500_ws_conn_t callback) {
    connect_callback = callback;
}

void HPW5500WebSocketServer::onDisconnect(HPW5500_ws_conn_t callback) {
    disconnect_callback = callback;
}

bool HPW5500WebSocketServer::sendText(uint8_t socket, const char *text) {
    if(text == nullptr) text = "";
    return sendFrame(socket, 0x1, reinterpret_cast<const uint8_t *>(text), static_cast<uint16_t>(std::strlen(text)), false);
}

bool HPW5500WebSocketServer::sendBinary(uint8_t socket, const uint8_t *data, uint16_t length) {
    return sendFrame(socket, 0x2, data, length, false);
}

bool HPW5500WebSocketServer::sendPing(uint8_t socket, const uint8_t *data, uint16_t length) {
    return sendFrame(socket, 0x9, data, length, false);
}

bool HPW5500WebSocketServer::sendPong(uint8_t socket, const uint8_t *data, uint16_t length) {
    return sendFrame(socket, 0xA, data, length, false);
}

bool HPW5500WebSocketServer::sendClose(uint8_t socket) {
    uint8_t status[2] = { 0x03, 0xE8 }; // 1000 = normal closure
    return sendFrame(socket, 0x8, status, 2, false);
}

void HPW5500WebSocketServer::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
    if(socket >= HPW5500_SOCKET_MAX) return;

    HPW5500WebSocketServer *instance = instance_by_socket[socket];
    if(instance == nullptr) return;

    instance->processPacket(socket, packet);
}

void HPW5500WebSocketServer::processPacket(uint8_t socket, const HPW5500_packet_t &packet) {
    if(packet.length == 0) return;

    ConnectionState &state = connections[socket];

    if(!state.handshake_done) {
        if(!handleHandshake(socket, packet.payload, packet.length)) {
            sendClose(socket);
        }
        return;
    }

    handleFrame(socket, packet.payload, packet.length);
}

bool HPW5500WebSocketServer::handleHandshake(uint8_t socket, const uint8_t *data, uint16_t length) {
    // Copy to null-terminated buffer for safe string operations
    if(length >= max_frame) return false;
    char text[HPW5500_WS_MAX_FRAME];
    std::memcpy(text, data, length);
    text[length] = '\0';

    if(std::strstr(text, "Upgrade: websocket") == nullptr && std::strstr(text, "Upgrade: WebSocket") == nullptr) return false;

    // Extract request path from "GET /path HTTP/1.1"
    connections[socket].path[0] = '/';
    connections[socket].path[1] = '\0';
    if(std::strncmp(text, "GET ", 4) == 0) {
        const char *pstart = text + 4;
        const char *pend = std::strchr(pstart, ' ');
        if(pend != nullptr) {
            uint16_t plen = static_cast<uint16_t>(pend - pstart);
            if(plen >= sizeof(connections[socket].path)) plen = sizeof(connections[socket].path) - 1;
            std::memcpy(connections[socket].path, pstart, plen);
            connections[socket].path[plen] = '\0';
        }
    }

    char key[64] = { 0 };
    if(!parse_header_value(text, "Sec-WebSocket-Key:", key, sizeof(key))) return false;

    char accept[64] = { 0 };
    if(build_accept_key(key, accept, sizeof(accept)) == 0) return false;

    char response[200];
    int response_len = std::snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n",
        accept);

    if(response_len <= 0) return false;

    device->writePacket(socket, reinterpret_cast<uint8_t *>(response), static_cast<uint16_t>(response_len));
    device->sendPacket(socket);

    ConnectionState &state = connections[socket];
    state.handshake_done = true;
    state.frag_len = 0;
    state.frag_opcode = 0;

    if(connect_callback != nullptr) connect_callback(socket);

    return true;
}

bool HPW5500WebSocketServer::handleFrame(uint8_t socket, const uint8_t *data, uint16_t length) {
    uint16_t offset = 0;
    ConnectionState &state = connections[socket];

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
        } else {
            return false;
        }

        if(offset + payload_len > length) return false;

        uint8_t payload[HPW5500_WS_MAX_FRAME];
        for(uint16_t i = 0; i < payload_len; i++) {
            payload[i] = data[offset + i] ^ mask_key[i % 4];
        }
        offset += static_cast<uint16_t>(payload_len);

        if(opcode == 0x8) {
            sendClose(socket);
            if(disconnect_callback != nullptr) disconnect_callback(socket);
            connections[socket] = ConnectionState();
            return true;
        }

        if(opcode == 0x9) {
            sendPong(socket, payload, static_cast<uint16_t>(payload_len));
            continue;
        }

        if(opcode == 0xA) continue;

        uint8_t message_opcode = opcode;
        if(opcode == 0x0) {
            if(state.frag_opcode == 0) return false;
            message_opcode = state.frag_opcode;
        } else if(!fin) {
            state.frag_opcode = opcode;
        }

        if(!fin) {
            if(state.frag_len + payload_len > max_frame) return false;
            std::memcpy(state.frag_buffer + state.frag_len, payload, payload_len);
            state.frag_len += static_cast<uint16_t>(payload_len);
            continue;
        }

        const uint8_t *message_ptr = payload;
        uint16_t message_len = static_cast<uint16_t>(payload_len);

        if(state.frag_opcode != 0) {
            if(state.frag_len + payload_len > max_frame) return false;
            std::memcpy(state.frag_buffer + state.frag_len, payload, payload_len);
            state.frag_len += static_cast<uint16_t>(payload_len);
            message_ptr = state.frag_buffer;
            message_len = state.frag_len;
            state.frag_len = 0;
            state.frag_opcode = 0;
        }

        if(message_callback != nullptr) message_callback(socket, message_opcode == 0x2, message_ptr, message_len);
    }

    return true;
}

const char *HPW5500WebSocketServer::getPath(uint8_t socket) const {
    if(socket >= HPW5500_SOCKET_MAX) return "/";
    if(((socket_handle >> socket) & 0x1) != 0x1) return "/";
    return connections[socket].path;
}

bool HPW5500WebSocketServer::sendFrame(uint8_t socket, uint8_t opcode, const uint8_t *data, uint16_t length, bool mask) {
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
        mask_key[0] = 0x12;
        mask_key[1] = 0x34;
        mask_key[2] = 0x56;
        mask_key[3] = 0x78;
        std::memcpy(header + header_len, mask_key, 4);
        header_len += 4;
    }

    device->writePacket(socket, header, header_len);

    if(data != nullptr && length > 0) {
        if(mask) {
            uint8_t masked[HPW5500_WS_MAX_FRAME];
            for(uint16_t i = 0; i < length; i++) masked[i] = data[i] ^ mask_key[i % 4];
            device->writePacket(socket, masked, length);
        } else {
            device->writePacket(socket, const_cast<uint8_t *>(data), length);
        }
    }

    return device->sendPacket(socket);
}
