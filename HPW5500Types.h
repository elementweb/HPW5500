#ifndef CLASS_HP_W5500_TYPES_H
#define CLASS_HP_W5500_TYPES_H

// Includes
#include <stdint.h>

// This what begin() method uses to report status
typedef enum: uint8_t {
    HPW5500_STATUS_UNKNOWN = 0xFF,
    HPW5500_STATUS_OK = 0x00,
    HPW5500_STATUS_MISSING_CALLBACK = 0x10,
    HPW5500_STATUS_NO_CONNECTION = 0x11,
    HPW5500_STATUS_READ_WRITE_TEST_FAILED = 0x12,
    HPW5500_STATUS_CONFIGURATION_FAILED = 0x13,
} HPW5500_status_t;

// Socket protocol type definition
typedef enum: uint8_t {
    HPW5500_SOCKET_PROTOCOL_CLOSED = HPW5500_VAL_SN_MR_PROTOCOL_CLOSED,
    HPW5500_SOCKET_PROTOCOL_TCP = HPW5500_VAL_SN_MR_PROTOCOL_TCP,
    HPW5500_SOCKET_PROTOCOL_UDP = HPW5500_VAL_SN_MR_PROTOCOL_UDP,
    HPW5500_SOCKET_PROTOCOL_MACRAW = HPW5500_VAL_SN_MR_PROTOCOL_MACRAW,
} HPW5500_socketProtocol_t;

// Socket status
struct HPW5500_SOCKET_STATUS { enum uint8_t {
    CLOSED = HPW5500_VAL_SN_SR_SOCK_CLOSED,
    INIT = HPW5500_VAL_SN_SR_SOCK_INIT,
    LISTEN = HPW5500_VAL_SN_SR_SOCK_LISTEN,
    ESTABLISHED = HPW5500_VAL_SN_SR_SOCK_ESTABLISHED,
    CLOSE_WAIT = HPW5500_VAL_SN_SR_SOCK_CLOSE_WAIT,
    UDP = HPW5500_VAL_SN_SR_SOCK_UDP,
    MACRAW = HPW5500_VAL_SN_SR_SOCK_MACRAW
}; };

// Socket commands
struct HPW5500_SOCKET_CMD { enum uint8_t {
    OPEN = HPW5500_VAL_SN_CR_COMMAND_OPEN,
    LISTEN = HPW5500_VAL_SN_CR_COMMAND_LISTEN,
    CONNECT = HPW5500_VAL_SN_CR_COMMAND_CONNECT,
    DISCON = HPW5500_VAL_SN_CR_COMMAND_DISCON,
    CLOSE = HPW5500_VAL_SN_CR_COMMAND_CLOSE,
    SEND = HPW5500_VAL_SN_CR_COMMAND_SEND,
    SEND_MAC = HPW5500_VAL_SN_CR_COMMAND_SEND_MAC,
    SEND_KEEP = HPW5500_VAL_SN_CR_COMMAND_SEND_KEEP,
    RECV = HPW5500_VAL_SN_CR_COMMAND_RECV,
}; };

// Socket handle
typedef uint8_t HPW5500_socket_handle_t;

// typedef struct {
//     uint8_t socket_array = 0x00;
//     uint16_t port = 0x0000;
// } HPW5500_socket_handle_t;

// Socket open attempt result
typedef enum: uint8_t {
    HPW5500_SOCKET_OPEN_SUCCESS = 0x00,
    HPW5500_SOCKET_CONNECT_SUCCESS = 0xFF,

    HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS = 0xFE,

    HPW5500_SOCKET_OPEN_FAILED_ALL_SOCKETS_BUSY = 0x01,
    HPW5500_SOCKET_OPEN_FAILED_MACRAW_ONLY_SOCKET_0 = 0x02,
    HPW5500_SOCKET_OPEN_FAILED_MACRAW_SOCKET_BUSY = 0x03,
    HPW5500_SOCKET_OPEN_FAILED_OUT_OF_RANGE = 0x04,
    HPW5500_SOCKET_OPEN_FAILED_NO_SOCKETS_OPENED = 0x05,
    HPW5500_SOCKET_OPEN_FAILED_MACRAW_UDP_MULTIPLE_SOCKETS = 0x06,

    HPW5500_SOCKET_OPEN_FAILED_CONFIGURATION = 0x07,
}
HPW5500_socket_init_attempt_t;


// HPW5500 socket buffer size
typedef enum: uint8_t {
    HPW5500_SIZE_0KB = HPW5500_VAL_SN_RXBUF_SIZE_0KB,
    HPW5500_SIZE_1KB = HPW5500_VAL_SN_RXBUF_SIZE_1KB,
    HPW5500_SIZE_2KB = HPW5500_VAL_SN_RXBUF_SIZE_2KB,
    HPW5500_SIZE_4KB = HPW5500_VAL_SN_RXBUF_SIZE_4KB,
    HPW5500_SIZE_8KB = HPW5500_VAL_SN_RXBUF_SIZE_8KB,
    HPW5500_SIZE_16KB = HPW5500_VAL_SN_RXBUF_SIZE_16KB,
} HPW5500_bufferSize_t;

// PHY OPMDC config
typedef enum: uint8_t {
    HALF_DUPLEX_10 = HPW5500_VAL_PHYCFGR_OPMDC_10BT_HD_AND,
    FULL_DUPLEX_10 = HPW5500_VAL_PHYCFGR_OPMDC_10BT_FD_AND,
    HALF_DUPLEX_100 = HPW5500_VAL_PHYCFGR_OPMDC_100BT_HD_AND,
    FULL_DUPLEX_100 = HPW5500_VAL_PHYCFGR_OPMDC_100BT_FD_AND,
    HALF_DUPLEX_100_AN = HPW5500_VAL_PHYCFGR_OPMDC_100BT_HD_ANE,
    NOT_USED = HPW5500_VAL_PHYCFGR_OPMDC_NOT_USED,
    POWER_DOWN = HPW5500_VAL_PHYCFGR_OPMDC_POWER_DOWN_MODE,
    ALL_CAPABLE_AN = HPW5500_VAL_PHYCFGR_OPMDC_ALL_CAPABLE_ANE,
    CONFIG_USING_HW_PINS = 0xFF,
} HPW5500_PHY_config_t;

// MR config bits
struct HPW5500_MR_CONFIG { enum uint8_t {
    RST = 0x80, WOL = 0x20, PB = 0x10, PPPoE = 0x8, FARP = 0x1, RESERVED = 0x45,
}; };

typedef enum: uint8_t {
    HPW5500_DEVICE_IP_CONFLICT,
    HPW5500_DEVICE_DEST_UNREACHABLE,
    HPW5500_DEVICE_PPPOE_CLOSE,
    HPW5500_DEVICE_WOL, // Magic Packet received
} HPW5500_device_event_t;

typedef enum: uint8_t {
    HPW5500_SOCKET_SEND_OK,
    HPW5500_SOCKET_TIMEOUT,
    HPW5500_SOCKET_DISCONNECT,
    HPW5500_SOCKET_REOPEN,
    HPW5500_SOCKET_CONNECT,
    HPW5500_SOCKET_MESSAGE_RECEIVE,
    HPW5500_SOCKET_CONNECTION_TIMEOUT_GIVEUP,
} HPW5500_socket_event_t;

// PHY status
typedef struct {
    bool full_duplex = false;
    bool speed_100 = false;
    bool link_up = false;
    uint8_t opmcd = 0x0;
} HPW5500_PHY_status_t;

// Ethernet packet
typedef struct {
    uint8_t payload[HPW5500_MTU];
    uint8_t ip[4];
    uint16_t port;
    uint16_t length;
} HPW5500_packet_t;

// IP
typedef uint8_t HPW5500_IP_t[4];
typedef uint8_t HPW5500_MAC_t[6];

// Interfaces for callback function pointers
typedef void (*HPW5500_void_callback_t) ();
typedef void (*HPW5500_unreachable_callback_t) (uint8_t *ip, uint16_t port);
typedef void (*HPW5500_socket_callback_t) (uint8_t socket);
typedef void (*HPW5500_socket_reopen_callback_t) (uint8_t socket, bool dueToTimeout);
typedef void (*HPW5500_socket_connect_callback_t) (uint8_t socket, uint8_t *ip, uint16_t port);
typedef void (*HPW5500_socket_receive_callback_t) (uint8_t socket, HPW5500_packet_t packet);
typedef uint8_t (*HPW5500_socket_giveup_callback_t) (uint8_t socket, uint8_t attempts);

// Define HPW5500_USE_MILLIS64 before including this header to use 64-bit millis timestamps
#ifdef HPW5500_USE_MILLIS64
typedef uint64_t HPW5500_millis_t;
#else
typedef uint32_t HPW5500_millis_t;
#endif

typedef HPW5500_millis_t (*HPW5500_millis_callback_t)();
typedef void (*HPW5500_delay_callback_t)(uint16_t ms);

// Callback pointer storage for general events
typedef struct {
    HPW5500_void_callback_t wakeUpViaWoL = nullptr, IPConflict = nullptr, PPPoEClose = nullptr;
    HPW5500_unreachable_callback_t destinationUnreachable = nullptr;
} HPW5500_event_callbacks_t;

// Callback pointer storage for sockets
typedef struct {
    HPW5500_socket_callback_t socketSendOK = nullptr, socketTimeout = nullptr, socketDisconnect = nullptr;
    HPW5500_socket_reopen_callback_t socketReopen = nullptr;
    HPW5500_socket_connect_callback_t socketConnect = nullptr;
    HPW5500_socket_receive_callback_t socketMessageReceive = nullptr;
    HPW5500_socket_giveup_callback_t socketConnectionTimeoutGiveUp = nullptr;
} HPW5500_socket_event_callbacks_t;

// Sockets
typedef struct {
    // Buffer sizes with default 2KB socket buffer size
    uint16_t buffer_size = 2048;
    uint16_t buffer_mask = 0x7ff;

    // Events
    HPW5500_socket_event_callbacks_t events;

    // Protocol
    HPW5500_socketProtocol_t socket_protocol;
    
    // Port
    uint16_t port = 0x0000;

    // Offset tracer for outgoing packets
    uint16_t offset_tracer;

    // Flags
    bool tcp_reopen_onclose;
    bool tcp_reopen_ontimeout;
    bool tcp_server;

    // Counters
    uint8_t tcp_reopen_ontimeout_times;
    uint8_t tcp_reopen_ontimeout_attempts;
    uint8_t tcp_reopen_onclose_times;
    uint8_t tcp_reopen_onclose_attempts;

    // Temporary storage for outgoing data
    struct {
        uint8_t *data;
        uint16_t length;
    } temp;

    // Socket handling
    uint8_t socket_array = 0x00;
} HPW5500_socket_t;

// Configuration structure
typedef struct {
    // Source MAC address
    HPW5500_MAC_t mac = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // Source IP address
    HPW5500_IP_t ip = { 0, 0, 0, 0 };

    // Subnet IP address
    HPW5500_IP_t subnet = { 0, 0, 0, 0 };

    // Gateway IP address
    HPW5500_IP_t gateway = { 0, 0, 0, 0 };

    // TX buffer sizes
    HPW5500_bufferSize_t tx_buffer_sizes[HPW5500_SOCKET_MAX] = {
        HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB,
        HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB
    };

    // RX buffer sizes
    HPW5500_bufferSize_t rx_buffer_sizes[HPW5500_SOCKET_MAX] = {
        HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB,
        HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB, HPW5500_SIZE_2KB
    };

    // PHY OPMDC configuration
    HPW5500_PHY_config_t opmcd = HPW5500_PHY_config_t::ALL_CAPABLE_AN;

    // Disable WOL by default; warning, enabling WOL will prevent receiving packets unless a Magic Packet is received; you may see ACT LED blink during sleep mode, but no data will be put into rx buffer
    bool wol_enabled = false;

    // Allow ping requests by default
    bool ping_block = false;

    // PPPoE mode disabled by default
    bool pppoe_mode = false;

    // Keep Force-ARP disabled by default
    bool force_arp = false;
} HPW5500_configuration_t;

// I/O statistics
typedef struct {
    uint32_t tx_packets = 0;
    uint32_t rx_packets = 0;

    uint64_t tx_bytes = 0;
    uint64_t rx_bytes = 0;

    uint32_t timeout_count = 0;
    uint32_t send_ok_count = 0;

    uint32_t tx_overruns = 0;
    uint32_t rx_overruns = 0;
} HPW5500_io_statistics_t;

// Overall statistics
typedef struct {
    uint32_t ip_conflict_count = 0;
    uint32_t dest_unreachable_count = 0;
    uint32_t pppoe_close_count = 0;
    uint32_t magic_packet_count = 0;

    HPW5500_io_statistics_t overall;
    HPW5500_io_statistics_t sockets[HPW5500_SOCKET_MAX];
} HPW5500_statistics_t;

#endif // CLASS_HP_W5500_TYPES_H
