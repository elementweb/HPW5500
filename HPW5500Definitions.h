#ifndef CLASS_HPW5500_DEFINITIONS_H
#define CLASS_HPW5500_DEFINITIONS_H

// Includes
#include <stdint.h>

/**
 * Register access
 */

#define HPW5500_REGISTER_BLOCK_COMMON                   0
#define HPW5500_REGISTER_BLOCK_SOCK(s)                  ((s) * 4 + 1)
#define HPW5500_REGISTER_BLOCK_SOCK_TX(s)               ((s) * 4 + 2)
#define HPW5500_REGISTER_BLOCK_SOCK_RX(s)               ((s) * 4 + 3)

/**
 * General
 */

#define HPW5500_VERSION                                 0x04

#define HPW5500_CLOCK_SPEED_MAX                         80000UL * 1000UL
#define HPW5500_CLOCK_SPEED_GUARANTEED                  33300UL * 1000UL
#define HPW5500_CLOCK_SPEED_OPTIMUM                     16000UL * 1000UL

#define HPW5500_PHY_RESET_BIT                           0x80
#define HPW5500_CMD_SIZE                                3
#define HPW5500_PHYCFGR_BASE                            0b10111000
#define HPW5500_PHYCFGR_OPMDC_MASK                      0b00111000
#define HPW5500_PHYCFGR_OPMDC                           0x40
#define HPW5500_MTU                                     1514
#define HPW5500_UDP_HEADER_LENGTH                       8
#define HPW5500_MAX_RX_PACKETS_AT_ONCE                  512

#define HPW5500_ALL_SOCKETS_BUSY                        0xFFFF
#define HPW5500_MACRAW_SOCKET_BUSY                      0xFFFE
#define HPW5500_MACRAW_ONLY_SOCKET_0                    0xFFFD

#define HPW5500_SNMR_PROTOCOL_MASK                      0x7
#define HPW5500_PORT_AUTOSELECT                         0x0000
#define HPW5500_PORT_AUTOSELECT_START                   10539

#define HPW5500_SOCKET_MAX                              8
#define HPW5500_SOCKET_AS_HANDLE(s)                     (HPW5500_socket_handle_t)(1 << s)

#define HPW5500_S0                                      0
#define HPW5500_S1                                      1
#define HPW5500_S2                                      2
#define HPW5500_S3                                      3
#define HPW5500_S4                                      4
#define HPW5500_S5                                      5
#define HPW5500_S6                                      6
#define HPW5500_S7                                      7

#define HPW5500_UNASSOCIATED                            0xFF

/**
 * Constant expressions
 */

constexpr uint8_t HPW5500_bitOffset(uint32_t mask) { return __builtin_ctz(mask); }

/**
 * Macros
 */

#define HPW5500_REG_WRITE(subject, mask, modifier) (((subject) & ~(mask)) | (((modifier) << HPW5500_bitOffset(mask)) & (mask)))
#define HPW5500_REG_READ(subject, mask) (((subject) & (mask)) >> HPW5500_bitOffset(mask))

#define HPW5500_REG_MASKED_WRITE(subject, mask, data) (((subject) & ~(mask)) | ((data) & (mask)))
#define HPW5500_REG_MASKED_READ(subject, mask) ((subject) & (mask))

#define HPW5500_SAFE_TO_EXECUTE(function_name, ...) if(function_name != nullptr) { function_name(__VA_ARGS__); }

#endif // CLASS_HPW5500_DEFINITIONS_H
