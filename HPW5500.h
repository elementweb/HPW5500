#ifndef CLASS_HPW5500_H
#define CLASS_HPW5500_H

// General C++ includes
#include <stdint.h>
#include <cstring>

// More includes
#include <HPW5500Definitions.h>
#include <HPW5500Layers.h>
#include <HPW5500Registers.h>
#include <HPW5500Types.h>

// TODO: clean up Arduino dependency
// #include <Arduino.h>

// Access config
#ifdef HPW5500_EXPOSE_ALL
    #define HPW5500_SECURE_PROPS                            public
    #define HPW5500_SECURE_METHODS                          public
#else
    #define HPW5500_SECURE_PROPS                            private
    #define HPW5500_SECURE_METHODS                          private
#endif

/**
 * HPW5500
 */

class HPW5500 {

    /** 
     * General
     */

    public: HPW5500(void (*callback)(uint8_t *cmd, uint8_t *dataIn, uint8_t *dataOut, uint16_t size, HPW5500 *obj) = nullptr);
    public: void begin(void (*error)(HPW5500_status_t status, HPW5500 * obj) = nullptr, void (*success)(HPW5500_status_t status, HPW5500 * obj) = nullptr);
    public: bool end();
    public: void softResetW5500(bool resetPHY = true);
    public: void softResetPHY();
    public: uint8_t version(bool force = false);

    /**
     * Configuration
     */

    public: HPW5500_configuration_t configuration;
    public: HPW5500_statistics_t statistics;
    public: HPW5500_PHY_status_t status;

    public: bool configure();
    public: bool connected();
    public: void overrideConfiguration(bool (*callback)(HPW5500 *obj));
    public: bool checkConfiguration();
    public: bool readWriteTest(bool force = false);
    public: void resetConfiguration();
    public: void dataTransfer(void (*callback)(uint8_t *cmd, uint8_t *dataIn, uint8_t *dataOut, uint16_t size, HPW5500 *obj));
    public: bool configureBufferSizes(uint16_t reg, uint8_t *sizes);
    public: bool configurePHY(HPW5500_PHY_config_t opmcd, bool force = false);
    public: bool configureModeRegister();
    public: bool enableWOL(bool enable = true, bool force = false);
    public: bool blockPing(bool block = true, bool force = false);
    public: bool enablePPPoEMode(bool enable = true, bool force = false);
    public: bool forceARP(bool enable = true, bool force = false);

    public: void configureMAC(const HPW5500_MAC_t &mac);
    public: void configureMAC(uint8_t mac0, uint8_t mac1, uint8_t mac2, uint8_t mac3, uint8_t mac4, uint8_t mac5);
    public: void configureIP(const HPW5500_IP_t &ip);
    public: void configureIP(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3);
    public: void configureSubnet(const HPW5500_IP_t &subnet);
    public: void configureSubnet(uint8_t sn0, uint8_t sn1, uint8_t sn2, uint8_t sn3);
    public: void configureGateway(const HPW5500_IP_t &gateway);
    public: void configureGateway(uint8_t gw0, uint8_t gw1, uint8_t gw2, uint8_t gw3);

    HPW5500_SECURE_METHODS: HPW5500_PHY_status_t checkPHYStatus(bool force = false);
    public: bool linkActive();

    /**
     * Socket operations
     */

    HPW5500_SECURE_PROPS: HPW5500_socket_t sockets[HPW5500_SOCKET_MAX];

    public: bool configureSocket(uint8_t number, HPW5500_socketProtocol_t protocol, uint16_t port, bool MULTI_MFEN = false, bool BCASTB = false, bool ND_MC_MMB = false, bool UCASTB_MIP6B = false, bool force = false);
    public: HPW5500_socket_init_attempt_t open(HPW5500_socket_handle_t *handle, HPW5500_socketProtocol_t protocol, uint16_t port = HPW5500_PORT_AUTOSELECT, uint8_t socketCount = 1, bool reopen_on_closed = true);
    public: HPW5500_socket_handle_t close(HPW5500_socket_handle_t *handle);
    public: HPW5500_socket_init_attempt_t connect(HPW5500_socket_handle_t *handle, uint8_t *ip, uint16_t port, bool reconnect_on_closed = false, bool reconnect_on_timeout = true, uint8_t reconnect_on_close_x = 3, uint8_t reconnect_on_timeout_x = 3, uint16_t local_port = HPW5500_PORT_AUTOSELECT);
    public: HPW5500_socket_handle_t disconnect(HPW5500_socket_handle_t *handle);

    HPW5500_SECURE_METHODS: uint16_t selectFreeSockets(uint8_t socketCount = 1, bool macRaw = false);
    HPW5500_SECURE_METHODS: uint8_t freeSockets();
    HPW5500_SECURE_METHODS: uint16_t portAutoSelect();
    public: uint8_t socketStatus(uint8_t socket);

    public: HPW5500_socket_handle_t killAll();

    /**
     * Core functions
     */

    public: void process(bool force = false);

    HPW5500_SECURE_METHODS: void handleSocketInterrupts();
    HPW5500_SECURE_METHODS: void handleDeviceInterrupts();
    HPW5500_SECURE_METHODS: void handleSocketEvents(uint8_t socket);

    HPW5500_SECURE_METHODS: void handleSocketReceiveEventUDPorMACRAW(uint8_t socket, uint16_t *pointer, uint8_t *header, HPW5500_packet_t *packet, bool copyData = true);
    HPW5500_SECURE_METHODS: void handleSocketReceiveEventTCP(uint8_t socket, uint16_t *pointer, HPW5500_packet_t *packet, uint16_t size, bool copyData = true);
    HPW5500_SECURE_METHODS: void handleSocketReceiveEvent(uint8_t socket);
    HPW5500_SECURE_METHODS: void socketAttemptToReconnectTCP(uint8_t socket, bool restart = false, uint8_t tryMoreTimes = 0);

    public: void writePacket(uint8_t socket, uint8_t *data, uint16_t length);
    public: uint16_t currentPacketSize(uint8_t socket);
    public: bool sendPacket(uint8_t socket, uint8_t *ip = nullptr, uint16_t port = 0);
    HPW5500_SECURE_METHODS: HPW5500_socketProtocol_t socketProtocol(uint8_t socket);

    /** 
     * Register access methods
     */

    HPW5500_SECURE_METHODS: void exchange(uint16_t addr, uint8_t block, uint8_t *dataIn, uint8_t *dataOut, uint16_t size);
    HPW5500_SECURE_METHODS: void read(uint16_t addr, uint8_t block, uint8_t *data, uint16_t size);
    HPW5500_SECURE_METHODS: uint8_t read8(uint16_t addr, uint8_t block = HPW5500_REGISTER_BLOCK_COMMON);
    HPW5500_SECURE_METHODS: uint16_t read16(uint16_t addr, uint8_t block = HPW5500_REGISTER_BLOCK_COMMON);
    HPW5500_SECURE_METHODS: void write(uint16_t addr, uint8_t block, uint8_t *data, uint16_t size);
    HPW5500_SECURE_METHODS: void write8(uint16_t addr, uint8_t block = HPW5500_REGISTER_BLOCK_COMMON, uint8_t data = 0x00);
    HPW5500_SECURE_METHODS: void write16(uint16_t addr, uint8_t block, uint16_t data);

    /**
     * Callback registration
     */

    public: void onDeviceEvent(HPW5500_device_event_t event, HPW5500_void_callback_t callback = nullptr);
    public: void onDestinationUnreachableEvent(HPW5500_unreachable_callback_t callback = nullptr);
    public: void unlinkDeviceCallbacks();

    public: void onSendOk(HPW5500_socket_callback_t callback = nullptr);
    public: void onSendOk(uint8_t socket, HPW5500_socket_callback_t callback = nullptr);
    public: void onSendOk(HPW5500_socket_handle_t *handle, HPW5500_socket_callback_t callback = nullptr);
    public: void onTimeout(HPW5500_socket_callback_t callback = nullptr);
    public: void onTimeout(uint8_t socket, HPW5500_socket_callback_t callback = nullptr);
    public: void onTimeout(HPW5500_socket_handle_t *handle, HPW5500_socket_callback_t callback = nullptr);
    public: void onMessage(HPW5500_socket_receive_callback_t callback = nullptr);
    public: void onMessage(uint8_t socket, HPW5500_socket_receive_callback_t callback = nullptr);
    public: void onMessage(HPW5500_socket_handle_t *handle, HPW5500_socket_receive_callback_t callback = nullptr);
    public: void onConnect(HPW5500_socket_connect_callback_t callback = nullptr);
    public: void onConnect(uint8_t socket, HPW5500_socket_connect_callback_t callback = nullptr);
    public: void onConnect(HPW5500_socket_handle_t *handle, HPW5500_socket_connect_callback_t callback = nullptr);
    public: void onDisconnect(HPW5500_socket_callback_t callback = nullptr);
    public: void onDisconnect(uint8_t socket, HPW5500_socket_callback_t callback = nullptr);
    public: void onDisconnect(HPW5500_socket_handle_t *handle, HPW5500_socket_callback_t callback = nullptr);
    public: void onReopen(HPW5500_socket_reopen_callback_t callback = nullptr);
    public: void onReopen(uint8_t socket, HPW5500_socket_reopen_callback_t callback = nullptr);
    public: void onReopen(HPW5500_socket_handle_t *handle, HPW5500_socket_reopen_callback_t callback = nullptr);
    public: void onConnectionTimeoutGiveUp(HPW5500_socket_giveup_callback_t callback = nullptr);
    public: void onConnectionTimeoutGiveUp(uint8_t socket, HPW5500_socket_giveup_callback_t callback = nullptr);
    public: void onConnectionTimeoutGiveUp(HPW5500_socket_handle_t *handle, HPW5500_socket_giveup_callback_t callback = nullptr);
    public: void unlinkGlobalSocketCallbacks();
    public: void unlinkSocketCallbacks(uint8_t socket);
    public: void unlinkSocketCallbacks(HPW5500_socket_handle_t *handle);

    /**
     * Class properties
     */

    HPW5500_SECURE_PROPS: void (*transfer)(uint8_t *cmd, uint8_t *dataIn, uint8_t *dataOut, uint16_t size, HPW5500 *obj) = nullptr;
    HPW5500_SECURE_PROPS: bool (*userConfiguration)(HPW5500 *obj) = nullptr;
    HPW5500_SECURE_PROPS: bool deviceConnected = false;
    HPW5500_SECURE_PROPS: HPW5500_event_callbacks_t device_events;
    HPW5500_SECURE_PROPS: HPW5500_socket_event_callbacks_t global_socket_events;

};

#endif // CLASS_HPW5500_H
