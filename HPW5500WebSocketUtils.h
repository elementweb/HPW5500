// WebSocket utilities for HPW5500

#ifndef CLASS_HPW5500_WEBSOCKET_UTILS_H
#define CLASS_HPW5500_WEBSOCKET_UTILS_H

#include <stdint.h>

bool HPW5500_ws_sha1(const uint8_t *data, uint32_t len, uint8_t out[20]);
uint16_t HPW5500_ws_base64_encode(const uint8_t *data, uint16_t len, char *out, uint16_t out_max);

#endif // CLASS_HPW5500_WEBSOCKET_UTILS_H
