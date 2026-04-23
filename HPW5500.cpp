#include "HPW5500.h"

/**
 * General
 */

HPW5500::HPW5500(void (*callback)(uint8_t *cmd, uint8_t *dataIn, uint8_t *dataOut, uint16_t size, HPW5500 *obj)) {
    // Store SPI data transfer callback if provided; this can also be done later via dataExchange()
    if(callback != nullptr) transfer = callback;
}

void HPW5500::begin(void (*error)(HPW5500_status_t status, HPW5500 * obj), void (*success)(HPW5500_status_t status, HPW5500 * obj)) {
    // Reset flag
    deviceConnected = false;

    // Check if data transfer function was externally supplied via dataExchange()
    if(!transfer) return error(HPW5500_STATUS_MISSING_CALLBACK, this);

    // Soft-reset W5500; at this stage it's a bit of a blind shot
    softResetW5500();

    // Test connection by query device for its version
    if(version(true) != HPW5500_VERSION) return error(HPW5500_STATUS_NO_CONNECTION, this);

    // Perform read/write test
    if(!readWriteTest(true)) return error(HPW5500_STATUS_READ_WRITE_TEST_FAILED, this);

    // Set flag
    deviceConnected = true;

    // Configure device
    bool configSuccess = (userConfiguration != nullptr) ? userConfiguration(this) : configure();

    // Success
    return configSuccess ? success(HPW5500_STATUS_OK, this) : error(HPW5500_STATUS_CONFIGURATION_FAILED, this);
}

bool HPW5500::end() {
    // Reset W5500
    softResetW5500();

    // Set flag
    deviceConnected = false;

    // This can't go wrong
    return !deviceConnected;
}

void HPW5500::softResetW5500(bool resetPHY) {
    // Reset W5500
    write8(HPW5500_COM_MR, HPW5500_MR_CONFIG::RST);

    // Reset PHY
    if(resetPHY) softResetPHY();
}

void HPW5500::softResetPHY() {
    // Reset PHY
    write8(HPW5500_COM_PHYCFGR, HPW5500_DEFAULT_PHYCFGR & ~HPW5500_MASK_PHYCFGR_RST);

    // Restore PHYCFGR register
    configurePHY(configuration.opmcd, true);
}

uint8_t HPW5500::version(bool force) {
    // Not connected?
    if(!force && !deviceConnected) return false;

    // Read version
    return read8(HPW5500_ADDR_VERSIONR);
}

/**
 * Configuration
 */

bool HPW5500::configure() {
    // Configure mode register (MR)
    if(!configureModeRegister()) return false;

    // Enable all global interrupts
    write8(HPW5500_COM_IMR, HPW5500_VAL_IMR_ALL_ENABLED);

    // Enable interrupts across SIMR
    write8(HPW5500_COM_SIMR, HPW5500_VAL_SIMR_ALL_ENABLED);

    // Enable interrupts across all sockets
    for(uint8_t socket=0; socket<HPW5500_SOCKET_MAX; socket++) write8(HPW5500_COM_SN_IMR(socket), HPW5500_VAL_SN_IMR_ALL_ENABLED);

    // Configure TX buffers
    if(!configureBufferSizes(HPW5500_ADDR_SN_TXBUF_SIZE, (uint8_t*)configuration.tx_buffer_sizes)) return false;

    // Configure RX buffers
    if(!configureBufferSizes(HPW5500_ADDR_SN_RXBUF_SIZE, (uint8_t*)configuration.rx_buffer_sizes)) return false;

    // Set MAC address
    write(HPW5500_COM_SHAR, configuration.mac, 6);

    // Set IP address
    write(HPW5500_COM_SIPR, configuration.ip, 4);

    // Set network subnet
    write(HPW5500_COM_SUBR, configuration.subnet, 4);

    // Set gateway address
    write(HPW5500_COM_GAR, configuration.gateway, 4);

    // Configure PHY OPMDC
    if(!configurePHY(configuration.opmcd)) return false;

    // Let's say this went fine
    return true;
}

bool HPW5500::applyNetworkConfig(bool force) {
    // Not connected?
    if(!force && !deviceConnected) return false;

    // Apply IP configuration to registers
    write(HPW5500_COM_SIPR, configuration.ip, 4);
    write(HPW5500_COM_SUBR, configuration.subnet, 4);
    write(HPW5500_COM_GAR, configuration.gateway, 4);

    return true;
}

bool HPW5500::connected() {
    return deviceConnected;
}

void HPW5500::overrideConfiguration(bool (*callback)(HPW5500 *obj)) {
    userConfiguration = callback;
}

bool HPW5500::checkConfiguration() {
    // Check MR for reserved dummy bit; returns true if configuration is not lost; false otherwise
    return HPW5500_REG_READ(read8(HPW5500_COM_MR), HPW5500_MASK_MR_RESERVED_BIT0) == 0b1;
}

bool HPW5500::readWriteTest(bool force) {
    // Not connected?
    if(!force && !deviceConnected) return false;

    // Get current MR state
    uint8_t mr = read8(HPW5500_COM_MR);

    // Toggle reserved bit
    write8(HPW5500_COM_MR, mr ^ HPW5500_MASK_MR_RESERVED_BIT2);

    // Check if toggling was successful
    return (read8(HPW5500_COM_MR) & HPW5500_MASK_MR_RESERVED_BIT2) != (mr & HPW5500_MASK_MR_RESERVED_BIT2);
}

void HPW5500::resetConfiguration() {
    // Reset configuration
    configuration = HPW5500_configuration_t();
}

void HPW5500::dataTransfer(void (*callback)(uint8_t *cmd, uint8_t *dataIn, uint8_t *dataOut, uint16_t size, HPW5500 *obj)) {
    // Store SPI data transfer callback; alternatively this can be done via the constructor
    transfer = callback;
}

bool HPW5500::configureBufferSizes(uint16_t reg, uint8_t *sizes) {
    // Check cumulative size to be <= 16
    if((sizes[0] + sizes[1] + sizes[2] + sizes[3] + sizes[4] + sizes[5] + sizes[6] + sizes[7]) > HPW5500_SIZE_16KB) return false;

    // Adjust socket true sizes and set masks
    for(uint8_t socket=0; socket<HPW5500_SOCKET_MAX; socket++) {
        // Size
        sockets[socket].buffer_size = sizes[socket] * 1024;
        
        // Mask
        sockets[socket].buffer_mask = sockets[socket].buffer_size >= 1024 ? (sockets[socket].buffer_size - 1) : 0;

        // Update register
        write8(reg, HPW5500_REGISTER_BLOCK_SOCK(socket), sizes[socket]);
    }

    return true;
}

bool HPW5500::configurePHY(HPW5500_PHY_config_t opmcd, bool force) {
    // Not connected?
    if(!force && !deviceConnected) return false;

    // Init PHYCFGR register value
    uint8_t phycfgr = HPW5500_DEFAULT_PHYCFGR;

    // OMPD configuration
    phycfgr = HPW5500_REG_WRITE(phycfgr, HPW5500_MASK_PHYCFGR_OPMD, opmcd == HPW5500_PHY_config_t::CONFIG_USING_HW_PINS ? HPW5500_VAL_PHYCFGR_OPMD_VIA_HW_PINS : HPW5500_VAL_PHYCFGR_OPMD_VIA_OPMDC);

    // OPMDC configuration
    phycfgr = HPW5500_REG_WRITE(phycfgr, HPW5500_MASK_PHYCFGR_OPMDC, (uint8_t)opmcd);

    // Write OPMDC configuration
    write8(HPW5500_COM_PHYCFGR, phycfgr);

    // Read back PHYCFGR to ensure write completed
    uint8_t read_phycfgr = read8(HPW5500_COM_PHYCFGR);

    // Verify if write was successful
    return HPW5500_REG_READ(read_phycfgr, HPW5500_MASK_PHYCFGR_OPMD) == HPW5500_REG_READ(phycfgr, HPW5500_MASK_PHYCFGR_OPMD)
        && HPW5500_REG_READ(read_phycfgr, HPW5500_MASK_PHYCFGR_OPMDC) == HPW5500_REG_READ(phycfgr, HPW5500_MASK_PHYCFGR_OPMDC);
}

bool HPW5500::configureModeRegister() {
    // Initialize MR value
    uint8_t mr = HPW5500_DEFAULT_MR;

    // WOL
    mr = HPW5500_REG_WRITE(mr, HPW5500_MASK_MR_WOL, configuration.wol_enabled ? HPW5500_VAL_MR_WOL_ENABLED : HPW5500_VAL_MR_WOL_DISABLED);

    // Ping block
    mr = HPW5500_REG_WRITE(mr, HPW5500_MASK_MR_PB, configuration.ping_block ? HPW5500_VAL_MR_PB_ENABLED : HPW5500_VAL_MR_PB_DISABLED);

    // PPPoE mode
    mr = HPW5500_REG_WRITE(mr, HPW5500_MASK_MR_PPPOE, configuration.pppoe_mode ? HPW5500_VAL_MR_PPPOE_ENABLED : HPW5500_VAL_MR_PPPOE_DISABLED);

    // Force ARP
    mr = HPW5500_REG_WRITE(mr, HPW5500_MASK_MR_FARP, configuration.force_arp ? HPW5500_VAL_MR_FARP_ENABLED : HPW5500_VAL_MR_FARP_DISABLED);

    // Dummy reserved bit
    mr = HPW5500_REG_WRITE(mr, HPW5500_MASK_MR_RESERVED_BIT0, 0b1);

    // Write MR
    write8(HPW5500_COM_MR, mr);

    // Verify if write was successful
    return read8(HPW5500_COM_MR) == mr;
}

bool HPW5500::enableWOL(bool condition, bool force) {
    // Update configuration
    configuration.wol_enabled = condition;

    // Reconfigure MR or update configuration object
    return (deviceConnected || force) ? configureModeRegister() : (configuration.wol_enabled == condition);
}

bool HPW5500::blockPing(bool condition, bool force) {
    // Update configuration
    configuration.ping_block = condition;

    // Reconfigure MR or update configuration object
    return (deviceConnected || force) ? configureModeRegister() : (configuration.ping_block == condition);
}

bool HPW5500::enablePPPoEMode(bool condition, bool force) {
    // Update configuration
    configuration.pppoe_mode = condition;

    // Reconfigure MR or update configuration object
    return (deviceConnected || force) ? configureModeRegister() : (configuration.pppoe_mode == condition);
}

bool HPW5500::forceARP(bool condition, bool force) {
    // Update configuration
    configuration.force_arp = condition;

    // Reconfigure MR or update configuration object
    return (deviceConnected || force) ? configureModeRegister() : (configuration.force_arp == condition);
}

void HPW5500::configureMAC(const HPW5500_MAC_t &mac) {
    memcpy(configuration.mac, mac, 6);
}

void HPW5500::configureMAC(uint8_t mac0, uint8_t mac1, uint8_t mac2, uint8_t mac3, uint8_t mac4, uint8_t mac5) {
    HPW5500_MAC_t mac = { mac0, mac1, mac2, mac3, mac4, mac5 };
    configureMAC(mac);
}

void HPW5500::configureIP(const HPW5500_IP_t &ip) {
    memcpy(configuration.ip, ip, 4);
}

void HPW5500::configureIP(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) {
    HPW5500_IP_t ip = { ip0, ip1, ip2, ip3 };
    configureIP(ip);
}

void HPW5500::configureSubnet(const HPW5500_IP_t &subnet) {
    memcpy(configuration.subnet, subnet, 4);
}

void HPW5500::configureSubnet(uint8_t sn0, uint8_t sn1, uint8_t sn2, uint8_t sn3) {
    HPW5500_IP_t subnet = { sn0, sn1, sn2, sn3 };
    configureSubnet(subnet);
}

void HPW5500::configureGateway(const HPW5500_IP_t &gateway) {
    memcpy(configuration.gateway, gateway, 4);
}

void HPW5500::configureGateway(uint8_t gw0, uint8_t gw1, uint8_t gw2, uint8_t gw3) {
    HPW5500_IP_t gateway = { gw0, gw1, gw2, gw3 };
    configureGateway(gateway);
}

HPW5500_PHY_status_t HPW5500::checkPHYStatus(bool force) {
    // Not connected?
    if(!force && !deviceConnected) return status;

    // Read PHY status
    uint8_t phycfgr = read8(HPW5500_COM_PHYCFGR);

    // Update status
    status.opmcd = HPW5500_REG_READ(phycfgr, HPW5500_MASK_PHYCFGR_OPMDC);
    status.link_up = HPW5500_REG_READ(phycfgr, HPW5500_MASK_PHYCFGR_LNK) == HPW5500_VAL_PHYCFGR_LNK_UP;
    status.speed_100 = HPW5500_REG_READ(phycfgr, HPW5500_MASK_PHYCFGR_SPD) == HPW5500_VAL_PHYCFGR_SPD_100;
    status.full_duplex = HPW5500_REG_READ(phycfgr, HPW5500_MASK_PHYCFGR_DPX) == HPW5500_VAL_PHYCFGR_DPX_FD;

    // Return status
    return status;
}

bool HPW5500::linkActive() {
    return checkPHYStatus().link_up;
}

/**
 * Socket operations
 */

bool HPW5500::configureSocket(uint8_t number, HPW5500_socketProtocol_t protocol, uint16_t port, bool MULTI_MFEN, bool BCASTB, bool ND_MC_MMB, bool UCASTB_MIP6B, bool force) {
    // Not connected?
    if(!force && !deviceConnected) return false;

    // Select port automatically
    if(port == HPW5500_PORT_AUTOSELECT) port = portAutoSelect();

    // Validate socket number
    if(number >= HPW5500_SOCKET_MAX) return false;

    // MACRAW can only work with socket 0
    if(protocol == HPW5500_SOCKET_PROTOCOL_MACRAW && number != 0) return false;

    // Configure socket mode
    uint8_t mode = protocol | (UCASTB_MIP6B << 4) | (ND_MC_MMB << 5) | (BCASTB << 6) | (MULTI_MFEN << 7);

    // Write to socket mode register and check if operation succeeded
    write8(HPW5500_COM_SN_MR(number), mode);
    if(mode != read8(HPW5500_COM_SN_MR(number))) return false;

    // Write port and check if port setting succeeded
    write16(HPW5500_COM_SN_PORT(number), port);
    if(port != read16(HPW5500_COM_SN_PORT(number))) return false;

    // Store protocol
    sockets[number].socket_protocol = protocol;

    // Store port
    sockets[number].port = port;

    // Enable all socket interrupts
    write8(HPW5500_COM_SN_IMR(number), HPW5500_VAL_SN_IMR_ALL_ENABLED);

    // Open socket
    write8(HPW5500_COM_SN_CR(number), HPW5500_SOCKET_CMD::OPEN);

    // TCP? Check if socket could be opened in TCP mode
    if(protocol == HPW5500_SOCKET_PROTOCOL_TCP) return socketStatus(number) == HPW5500_SOCKET_STATUS::INIT;

    // UDP? Check if socket could be opened in UDP mode
    if(protocol == HPW5500_SOCKET_PROTOCOL_UDP) return socketStatus(number) == HPW5500_SOCKET_STATUS::UDP;

    // MACRAW? Check if socket could be opened in MACRAW mode
    if(protocol == HPW5500_SOCKET_PROTOCOL_MACRAW) return socketStatus(number) == HPW5500_SOCKET_STATUS::MACRAW;
    
    // Reset statistics
    statistics.sockets[number] = HPW5500_io_statistics_t();

    // Not sure
    return false;
}

HPW5500_socket_init_attempt_t HPW5500::open(HPW5500_socket_handle_t *handle, HPW5500_socketProtocol_t protocol, uint16_t port, uint8_t socketCount, bool reopen_on_closed, uint8_t socket_mask) {
    // Excuse me?
    if(protocol == HPW5500_SOCKET_PROTOCOL_CLOSED) return HPW5500_SOCKET_OPEN_FAILED_CONFIGURATION;
    
    // Can't do this to me
    if(socketCount < 1 || socketCount > 8) return HPW5500_SOCKET_OPEN_FAILED_OUT_OF_RANGE;

    // UDP and MACRAW can't be opened on multiple sockets at once
    if((protocol == HPW5500_SOCKET_PROTOCOL_UDP || protocol == HPW5500_SOCKET_PROTOCOL_MACRAW) && socketCount > 1) return HPW5500_SOCKET_OPEN_FAILED_MACRAW_UDP_MULTIPLE_SOCKETS;

    // Select free sockets (restricted to socket_mask if provided)
    uint16_t free_sockets = selectFreeSockets(socketCount, protocol == HPW5500_SOCKET_PROTOCOL_MACRAW, socket_mask);
    uint8_t opened_socket_count = 0;

    // Won't do. All sockets are busy at the moment
    if(free_sockets == HPW5500_ALL_SOCKETS_BUSY) return HPW5500_SOCKET_OPEN_FAILED_ALL_SOCKETS_BUSY;

    // MACRAW can only work with socket 0
    if(free_sockets == HPW5500_MACRAW_ONLY_SOCKET_0) return HPW5500_SOCKET_OPEN_FAILED_MACRAW_ONLY_SOCKET_0;
    
    // MACRAW socket is busy
    if(free_sockets == HPW5500_MACRAW_SOCKET_BUSY) return HPW5500_SOCKET_OPEN_FAILED_MACRAW_SOCKET_BUSY;

    // Reset handle to defaults
    *handle = 0x00;

    // Now iterate through sockets and open them; multiple sockets only for TCP, in all other cases below loop will run only once
    for(uint8_t socket = 0; socket<HPW5500_SOCKET_MAX; socket++) {
        // Check if socket is selected
        if(((free_sockets >> socket) & 0x1) != 0x1) continue;

        // Configure socket (enable No Delayed ACK for TCP — equivalent of TCP_NODELAY)
        bool nd = (protocol == HPW5500_SOCKET_PROTOCOL_TCP);
        if(!configureSocket(socket, protocol, port, false, false, nd)) continue;

        // Update handle
        *handle |= 1 << socket;

        // TCP protocol
        if(protocol == HPW5500_socketProtocol_t::HPW5500_SOCKET_PROTOCOL_TCP) {
            // Should socket be reopened in case it is closed? Will apply in TCP mode only
            sockets[socket].tcp_reopen_onclose = reopen_on_closed;
            sockets[socket].tcp_reopen_onclose_times = reopen_on_closed ? 255 : 0;
            sockets[socket].tcp_reopen_onclose_attempts = 0;

            // Socket is acting as TCP server
            sockets[socket].tcp_server = true;

            // Start listening
            write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::LISTEN);
        }

        // Increment opened socket count, later used to determine result
        opened_socket_count++;
    }

    // Initially unlink all callbacks
    unlinkSocketCallbacks(handle);

    // No sockets could be opened
    if(opened_socket_count == 0) return HPW5500_SOCKET_OPEN_FAILED_NO_SOCKETS_OPENED;

    // Return result
    return opened_socket_count == socketCount ? HPW5500_SOCKET_OPEN_SUCCESS : HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS;
}

HPW5500_socket_handle_t HPW5500::close(HPW5500_socket_handle_t *handle) {
    // Initialize handle for closed socket
    HPW5500_socket_handle_t closed_sockets = 0x00;
    
    for(uint8_t socket = 0; socket<HPW5500_SOCKET_MAX; socket++) {
        // Check if we are talking about the right socket here
        if(((*handle >> socket) & 0x1) != 0b1) continue;

        uint8_t status = socketStatus(socket);

        // Do nothing if socket is already closed
        if(status == HPW5500_SOCKET_STATUS::CLOSED) continue;

        // For TCP; set to not re-open in case this was set
        sockets[socket].tcp_reopen_onclose = false;

        // Send command to close socket
        write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::CLOSE);

        // Check if operation was successful
        if(socketStatus(socket) == HPW5500_SOCKET_STATUS::CLOSED) {
            // Update flag
            closed_sockets |= 1 << socket;
            continue;
        }
    }

    return closed_sockets;
}

HPW5500_socket_init_attempt_t HPW5500::connect(HPW5500_socket_handle_t *handle, uint8_t *ip, uint16_t port, bool reconnect_on_closed, bool reconnect_on_timeout, uint8_t reconnect_on_closed_x, uint8_t reconnect_on_timeout_x, uint16_t local_port) {
    // Reset handle to defaults
    *handle = 0x00;

    // Select first free socket
    uint8_t free_socket = selectFreeSockets(1);

    // Won't do. All sockets are busy at the moment
    if(free_socket == HPW5500_ALL_SOCKETS_BUSY || free_socket > 0xFF) return HPW5500_SOCKET_OPEN_FAILED_ALL_SOCKETS_BUSY;

    // Determine socket number
    uint8_t socket = HPW5500_bitOffset(free_socket);

    // Configure socket
    if(!configureSocket(socket, HPW5500_SOCKET_PROTOCOL_TCP, local_port)) return HPW5500_SOCKET_OPEN_FAILED_CONFIGURATION;

    // Store socket protocol
    sockets[socket].socket_protocol = HPW5500_SOCKET_PROTOCOL_TCP;

    // Should socket reconnect in case connection was dropped (or disconnected by peer?)
    sockets[socket].tcp_reopen_onclose = reconnect_on_closed;
    sockets[socket].tcp_reopen_onclose_times = reconnect_on_closed_x;
    sockets[socket].tcp_reopen_onclose_attempts = 0;

    // Should we attempt to reopen socket in case of a timeout? If so, how many times?
    sockets[socket].tcp_reopen_ontimeout = reconnect_on_timeout;
    sockets[socket].tcp_reopen_ontimeout_times = reconnect_on_timeout_x;
    sockets[socket].tcp_reopen_ontimeout_attempts = 0;

    // Socket is acting as TCP client
    sockets[socket].tcp_server = false;

    // Update handle
    *handle |= 1 << socket;

    // Initially unlink all callbacks
    unlinkSocketCallbacks(handle);

    // Set destination IP and port
    write(HPW5500_COM_SN_DIPR(socket), ip, 4);
    write16(HPW5500_COM_SN_DPORT(socket), port);

    // Issue CONNECT command
    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::CONNECT);

    // Assume success
    return HPW5500_SOCKET_CONNECT_SUCCESS;
}

HPW5500_socket_handle_t HPW5500::disconnect(HPW5500_socket_handle_t *handle) {
    // Initialize handle for disconnected socket
    HPW5500_socket_handle_t disconnected_sockets = 0x00;

    for(uint8_t socket = 0; socket<HPW5500_SOCKET_MAX; socket++) {
        // Check if we are talking about the right socket here
        if(((*handle >> socket) & 0x1) != 0x1) continue;

        // Do nothing if socket is not established
        if(socketStatus(socket) != HPW5500_SOCKET_STATUS::ESTABLISHED) continue;

        // Send command to disconnect socket from peer
        write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::DISCON);

        // Check if operation was successful
        if(socketStatus(socket) == HPW5500_SOCKET_STATUS::INIT) {
            // Update flag
            disconnected_sockets |= 1 << socket;
            continue;
        }
    }

    return disconnected_sockets;
}

uint16_t HPW5500::selectFreeSockets(uint8_t socketCount, bool macRaw, uint8_t allowed_mask) {
    // MACRAW can only operate on socket 0
    if(macRaw && socketCount != 1) return HPW5500_MACRAW_ONLY_SOCKET_0;

    // MACRAW mode; check if socket 0 is free
    if(macRaw) return socketStatus(0) == HPW5500_SOCKET_STATUS::CLOSED ? 0x01 : HPW5500_MACRAW_SOCKET_BUSY;

    // Variables to track selected sockets and how many are available
    uint8_t sockets = 0x00;
    uint8_t available = 0;

    // Iterate through all other sockets [7 → 0] and return the first one that is free
    for(int8_t socket=HPW5500_SOCKET_MAX-1; socket>=0; socket--) {
        // Skip sockets not in the allowed mask
        if(!((allowed_mask >> socket) & 0x1)) continue;
        // Check if socket is free/closed
        if(socketStatus(socket) == HPW5500_SOCKET_STATUS::CLOSED) {
            // Mark socket as selected and increment available counter
            sockets |= 1 << socket;
            available++;

            // Have we selected enough sockets?
            if(available == socketCount) break;
        }
    }

    // Return selected sockets or error code
    return available == socketCount ? (uint16_t)sockets : HPW5500_ALL_SOCKETS_BUSY;
}

uint8_t HPW5500::freeSockets() {
    // Assume none are free at start
    uint8_t free = 0;

    // Iterate through all [0 → 7] and count how many are free
    for(uint8_t socket=0; socket<HPW5500_SOCKET_MAX; socket++) {
        if(socketStatus(socket) == HPW5500_SOCKET_STATUS::CLOSED) free++;
    }

    // Expect here a number between [0 → 8]
    return free;
}

uint16_t HPW5500::portAutoSelect() {
    static uint16_t nextPortInLine = HPW5500_PORT_AUTOSELECT_START;

    nextPortInLine++; // starting with 10540

    // Can not be 0
    if(nextPortInLine == 0) nextPortInLine++;

    return nextPortInLine;
}

uint8_t HPW5500::socketStatus(uint8_t socket) {
    // Query status
    return read8(HPW5500_COM_SN_SR(socket));
}

void HPW5500::waitForCmdDone(uint8_t socket) {
    for(uint8_t i = 0; i < 200; i++) {
        if(read8(HPW5500_COM_SN_CR(socket)) == 0x00) return;
    }
}

void HPW5500::dropSocket(uint8_t socket) {
    if(socket >= HPW5500_SOCKET_MAX) return;
    uint8_t sr_before = read8(HPW5500_COM_SN_SR(socket));
    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::CLOSE);
    waitForCmdDone(socket);
    uint8_t sr_after = read8(HPW5500_COM_SN_SR(socket));
}

void HPW5500::reopenSocket(uint8_t socket) {
    if(socket >= HPW5500_SOCKET_MAX) return;
    sockets[socket].offset_tracer = 0;
    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::OPEN);
    waitForCmdDone(socket);
    uint8_t sr_open = read8(HPW5500_COM_SN_SR(socket));
    if(sockets[socket].tcp_server) {
        write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::LISTEN);
        waitForCmdDone(socket);
    }
    uint8_t sr_final = read8(HPW5500_COM_SN_SR(socket));
}

HPW5500_socket_handle_t HPW5500::killAll() {
    // Initialize handle for killed sockets
    HPW5500_socket_handle_t killed_sockets = 0x00;

    // Temporary variable
    uint8_t socket_status;
    
    // Iterate through all sockets
    for(uint8_t socket = 0; socket<HPW5500_SOCKET_MAX; socket++) {
        // Query socket status
        socket_status = socketStatus(socket);

        // TCP client mode
        if(socket_status == HPW5500_SOCKET_STATUS::ESTABLISHED) {
            // Send DISCON command first
            write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::DISCON);

            // Check if operation was successful
            if(socketStatus(socket) == HPW5500_SOCKET_STATUS::INIT) {
                // Update handle
                killed_sockets |= 1 << socket;
                continue;
            }
        }

        // TCP server mode
        if(socket_status == HPW5500_SOCKET_STATUS::LISTEN) {
            // Send CLOSE command
            write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::CLOSE);

            // Check if operation was successful
            if(socketStatus(socket) == HPW5500_SOCKET_STATUS::CLOSED) {
                // Update handle
                killed_sockets |= 1 << socket;
                continue;
            }
        }
    }

    // Return killed sockets
    return killed_sockets;
}

/**
 * Core functions
 */

void HPW5500::process(bool force) {
    // Not connected?
    if(!force && !deviceConnected) return;

    // Handle socket interrupt events
    handleSocketInterrupts();

    // Handle device interrupt events
    handleDeviceInterrupts();

    // Sample PHY status
    checkPHYStatus();
}

void HPW5500::handleSocketInterrupts() {
    // Initialize variable to read SIR
    uint8_t sir = HPW5500_DEFAULT_SIR;

    // Counter to not allow while executing forever
    uint8_t counter = 0;

    // Process all events; run no more than 16x times sequentially
    while((sir = read8(HPW5500_COM_SIR)) != HPW5500_DEFAULT_SIR && counter < 16) {
        // Increment counter
        counter++;

        // Cycle through sockets
        for(uint8_t socket=0; socket<HPW5500_SOCKET_MAX; socket++) {
            if((sir >> socket) & 0x1) handleSocketEvents(socket);
        }
    }
}

void HPW5500::handleDeviceInterrupts() {
    // Initialize variable to read IR
    uint8_t ir = HPW5500_DEFAULT_IR;

    // Counter to not allow while executing forever
    uint8_t counter = 0;

    // Process all events; run no more than 4x times sequentially
    while((ir = read8(HPW5500_COM_IR)) != HPW5500_DEFAULT_IR && counter < 4) {
        // Clear immediately
        write8(HPW5500_COM_IR, ir);

        // Increment counter
        counter++;

        // IP conflict
        if(device_events.IPConflict != nullptr && HPW5500_REG_READ(ir, HPW5500_MASK_IR_CONFLICT)) device_events.IPConflict();

        // Destination unreachable
        if(device_events.destinationUnreachable != nullptr && HPW5500_REG_READ(ir, HPW5500_MASK_IR_UNREACH)) {
            // Read unreachable IP address
            HPW5500_IP_t ip;
            read(HPW5500_COM_UIPR, ip, 4);

            // Read unreachable port
            uint16_t port = read16(HPW5500_COM_UPORTR);

            // Call user provided callback
            device_events.destinationUnreachable(ip, port);
        }

        // PPPoE close
        if(device_events.PPPoEClose != nullptr && HPW5500_REG_READ(ir, HPW5500_MASK_IR_PPPOE)) device_events.PPPoEClose();

        // WOL/MP
        if(device_events.wakeUpViaWoL != nullptr && HPW5500_REG_READ(ir, HPW5500_MASK_IR_MP)) device_events.wakeUpViaWoL();
    }
}

void HPW5500::handleSocketEvents(uint8_t socket) {
    // Read SNIR
    uint8_t snir = read8(HPW5500_COM_SN_IR(socket));

    // Clear SNIR immediatelly
    write8(HPW5500_COM_SN_IR(socket), HPW5500_VAL_SN_IR_ALL_RESET);

    // Nothing to process?
    if(HPW5500_REG_READ(snir, HPW5500_USABLE_SN_IR) == HPW5500_USABLE_SN_IR) return;

    // Socket index is already 0-7, no bit offset needed
    HPW5500_socket_event_callbacks_t *socket_events = &sockets[socket].events;

    // SEND_OK event
    if(HPW5500_REG_READ(snir, HPW5500_MASK_SN_IR_SEND_OK)) {
        // Increment counter
        statistics.sockets[socket].send_ok_count++;

        // User provided callbacks
        HPW5500_SAFE_TO_EXECUTE(socket_events->socketSendOK, socket);
        HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketSendOK, socket);
    }

    // TIMEOUT event
    if(HPW5500_REG_READ(snir, HPW5500_MASK_SN_IR_TIMEOUT)) {
        // Increment counter
        statistics.sockets[socket].timeout_count++;

        // User provided callbacks
        HPW5500_SAFE_TO_EXECUTE(socket_events->socketTimeout, socket);
        HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketTimeout, socket);

        // TCP client; in case re-connection is required; issue commands and call used defined callback function
        if(sockets[socket].socket_protocol == HPW5500_SOCKET_PROTOCOL_TCP && !sockets[socket].tcp_server && sockets[socket].tcp_reopen_ontimeout) {
            if(sockets[socket].tcp_reopen_ontimeout_attempts < sockets[socket].tcp_reopen_ontimeout_times) {
                socketAttemptToReconnectTCP(socket);

                // Custom event here to call when TCP socket is reopened
                HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketReopen, socket, true);
            } else {
                // Invoke connection give-up event if callback was provided by user returning number of desired extra attempts
                if(sockets[socket].events.socketConnectionTimeoutGiveUp != nullptr) socketAttemptToReconnectTCP(socket, true, sockets[socket].events.socketConnectionTimeoutGiveUp(socket, sockets[socket].tcp_reopen_ontimeout_attempts));
            }
        }

        // TCP server; timeout means peer disappeared — re-open socket to accept new connections
        if(sockets[socket].socket_protocol == HPW5500_SOCKET_PROTOCOL_TCP && sockets[socket].tcp_server) {
            uint8_t status = socketStatus(socket);
            if(status == HPW5500_SOCKET_STATUS::CLOSED) {
                // Fire disconnect callbacks so upper layers (e.g. WebSocket server) clean up
                HPW5500_SAFE_TO_EXECUTE(socket_events->socketDisconnect, socket);
                HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketDisconnect, socket);

                if(sockets[socket].tcp_reopen_onclose) {
                    sockets[socket].tcp_reopen_onclose_attempts = 0;
                    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::OPEN);
                    waitForCmdDone(socket);
                    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::LISTEN);
                    waitForCmdDone(socket);
                    HPW5500_SAFE_TO_EXECUTE(socket_events->socketReopen, socket, true);
                    HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketReopen, socket, true);
                }
            }
        }
    }

    // CON event
    if(HPW5500_REG_READ(snir, HPW5500_MASK_SN_IR_CON)) {
        // Read unreachable IP address
        uint8_t ip[4];
        read(HPW5500_COM_SN_DIPR(socket), ip, 4);

        // Read unreachable port
        uint16_t port = read16(HPW5500_COM_SN_DPORT(socket));
        
        // User provided callbacks
        HPW5500_SAFE_TO_EXECUTE(socket_events->socketConnect, socket, ip, port);
        HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketConnect, socket, ip, port);

        // Reset number re-open attempts the configure socket may had
        sockets[socket].tcp_reopen_ontimeout_attempts = 0;

        // Might be that data was received at the same time, therefore check
        handleSocketReceiveEvent(socket);
    }

    // RECV; unlike other socket events, we should always read RX_BUF on RECV event to prevent clogging/overflow of data.
    // We are pointing here to our internal library method that processes/reads off data
    // and discards it unless user has provided with approapriate callback via onSocketMessageReceive()
    if(HPW5500_REG_READ(snir, HPW5500_MASK_SN_IR_RECV)) handleSocketReceiveEvent(socket);

    // DISCON event
    if(HPW5500_REG_READ(snir, HPW5500_MASK_SN_IR_DISCON)) {
        // Read socket status
        uint8_t status = socketStatus(socket);

        // Event was triggered after peer requested to disconnect
        if(status == HPW5500_SOCKET_STATUS::CLOSE_WAIT) {
            // Command socket to disconnect from peer
            write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::DISCON);
            waitForCmdDone(socket);

            // Re-read status — DISCON often completes instantly when peer already closed
            status = socketStatus(socket);
        }

        // Event was triggered after connection was closed completely
        if(status == HPW5500_SOCKET_STATUS::CLOSED) {
            // User provided callbacks
            HPW5500_SAFE_TO_EXECUTE(socket_events->socketDisconnect, socket);
            HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketDisconnect, socket);

            // TCP server; in case re-open is required; issue commands and call used defined callback function
            if(sockets[socket].socket_protocol == HPW5500_SOCKET_PROTOCOL_TCP && sockets[socket].tcp_server && sockets[socket].tcp_reopen_onclose) {
                // Reset attempt counter for servers (they should accept connections indefinitely)
                sockets[socket].tcp_reopen_onclose_attempts = 0;

                // Re-issue open and listen commands
                write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::OPEN);
                waitForCmdDone(socket);
                write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::LISTEN);
                waitForCmdDone(socket);

                // User provided callbacks; notify that socket was reopened
                HPW5500_SAFE_TO_EXECUTE(socket_events->socketReopen, socket, false);
                HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketReopen, socket, false);
            }

            // TCP client; in case re-connection is required; issue commands and call used defined callback function
            if(sockets[socket].socket_protocol == HPW5500_SOCKET_PROTOCOL_TCP && !sockets[socket].tcp_server && sockets[socket].tcp_reopen_onclose && sockets[socket].tcp_reopen_onclose_attempts < sockets[socket].tcp_reopen_onclose_times) {
                // Increment counter
                sockets[socket].tcp_reopen_onclose_attempts++;

                // Re-issue open and connect commands
                write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::OPEN);
                waitForCmdDone(socket);
                write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::CONNECT);

                // User provided callbacks; notify that socket was reopened
                HPW5500_SAFE_TO_EXECUTE(socket_events->socketReopen, socket, false);
                HPW5500_SAFE_TO_EXECUTE(global_socket_events.socketReopen, socket, false);
            }
        }
    }
}

void HPW5500::handleSocketReceiveEventUDPorMACRAW(uint8_t socket, uint16_t *pointer, uint8_t *header, HPW5500_packet_t *packet, bool copyData) {
    // Read packet header (8 bytes)
    read(*pointer, HPW5500_REGISTER_BLOCK_SOCK_RX(socket), header, HPW5500_UDP_HEADER_LENGTH);

    // Determine packet payload size
    packet->length = header[6] << 8 | header[7];

    // Proceed if there is callback function to be invoked later
    if(copyData) {
        // Peer IP
        memcpy(&packet->ip[0], &header[0], 4);
        
        // Peer port
        packet->port = header[4] << 8 | header[5];

        // Copy data
        read(*pointer + HPW5500_UDP_HEADER_LENGTH, HPW5500_REGISTER_BLOCK_SOCK_RX(socket), packet->payload, packet->length);
    }

    // Update pointer
    write16(HPW5500_COM_SN_RX_RD(socket), *pointer + HPW5500_UDP_HEADER_LENGTH + packet->length);
}

void HPW5500::handleSocketReceiveEventTCP(uint8_t socket, uint16_t *pointer, HPW5500_packet_t *packet, uint16_t size, bool copyData) {
    // Cap read to payload buffer size to prevent overflow
    packet->length = size > HPW5500_MTU ? HPW5500_MTU : size;

    // Proceed if there is callback function to be invoked later
    if(copyData) {
        // Peer IP
        read(HPW5500_COM_SN_DIPR(socket), packet->ip, 4);
        
        // Peer port
        packet->port = read16(HPW5500_COM_SN_DPORT(socket));

        // Copy data
        read(*pointer, HPW5500_REGISTER_BLOCK_SOCK_RX(socket), packet->payload, packet->length);
    }

    // Advance pointer by actual amount consumed (capped)
    write16(HPW5500_COM_SN_RX_RD(socket), *pointer + packet->length);
}

void HPW5500::handleSocketReceiveEvent(uint8_t socket) {
    // Initialize parser variables
    HPW5500_packet_t packet;
    uint16_t pointer = 0;
    uint8_t header[8];
    uint16_t iterations = 0;
    uint16_t received_content_size = 0;

    // Get socket protocol
    HPW5500_socketProtocol_t protocol = sockets[socket].socket_protocol;

    // Would be funny, but still..
    if(protocol == HPW5500_SOCKET_PROTOCOL_CLOSED) return;

    // Check if there is callback to invoke; because if not, data will be discarded
    bool executeCallback = sockets[socket].events.socketMessageReceive != nullptr;

    // Iterate through RX buffer and query size of received content; if new UDP has arrived while you were still processing the previous one, the `while(..)` will resolve to true as long as iterations counter has not been hit yet
    while(iterations < HPW5500_MAX_RX_PACKETS_AT_ONCE && (received_content_size = read16(HPW5500_COM_SN_RX_RSR(socket)))) {
        // Incr. counter; this is used to prevent infinite loop
        iterations++;
        
        // Obtain pointer
        pointer = read16(HPW5500_COM_SN_RX_RD(socket));

        // Packets are handled differently for different protocols
        if(protocol == HPW5500_SOCKET_PROTOCOL_TCP) {
            // The packet is TCP
            handleSocketReceiveEventTCP(socket, &pointer, &packet, received_content_size, executeCallback);
        } else {
            // Packet is either UDP or MACRAW
            handleSocketReceiveEventUDPorMACRAW(socket, &pointer, header, &packet, executeCallback);
        }

        // Issue a RECV command
        write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::RECV);

        // Execute callback if provided
        if(executeCallback) sockets[socket].events.socketMessageReceive(socket, packet);
    } 
}

void HPW5500::socketAttemptToReconnectTCP(uint8_t socket, bool restart, uint8_t tryMoreTimes) {
    if(restart) {
        // User has requested to not attempt to re-connect after giving up
        if(tryMoreTimes <= 0) return;

        // How many timeouts we can accept before giving up again?
        sockets[socket].tcp_reopen_ontimeout_times = tryMoreTimes;

        // This is an attempt already so reset to 1 and not 0
        sockets[socket].tcp_reopen_ontimeout_attempts = 1;
    } else {
        // Increment attempt counter
        sockets[socket].tcp_reopen_ontimeout_attempts++;
    }

    // Re-open socket
    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::OPEN);
    waitForCmdDone(socket);
    
    // Attempt to establish a connection
    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::CONNECT);
}

void HPW5500::writePacket(uint8_t socket, uint8_t *data, uint16_t length) {
    // Read new pointer and calculate offset
	uint16_t pointer = read16(HPW5500_COM_SN_TX_WR(socket)) + sockets[socket].offset_tracer;
	uint16_t offset = pointer & sockets[socket].buffer_mask;

    // Check if data to be written overflows the circular buffer
	if (offset + length <= sockets[socket].buffer_size) {
        // Write data in one-go
		write(offset, HPW5500_REGISTER_BLOCK_SOCK_TX(socket), data, length);
	} else {
        // Compute how much data can be written in the first op. at the end of circular buffer
		uint16_t size = sockets[socket].buffer_size - offset;
        
        // Write the first portion (size) of data in the end of circular buffer
		write(offset, HPW5500_REGISTER_BLOCK_SOCK_TX(socket), data, size);

        // Write the remainder (length - size) of data (data offsetted by size) in the beginning of circular buffer
		write(0x0000, HPW5500_REGISTER_BLOCK_SOCK_TX(socket), data + size, length - size);
	}

    // Set new pointer and offset for next operation
    write16(HPW5500_COM_SN_TX_WR(socket), pointer + length);
	sockets[socket].offset_tracer += length;
}

uint16_t HPW5500::currentPacketSize(uint8_t socket) {
    // Offset equals to the bytes currently written in circular buffer
    return sockets[socket].offset_tracer;
}

uint16_t HPW5500::bufferSize(uint8_t socket) {
    return sockets[socket].buffer_size;
}

uint16_t HPW5500::txFreeSize(uint8_t socket) {
    return read16(HPW5500_COM_SN_TX_FSR(socket));
}

bool HPW5500::sendPacket(uint8_t socket, uint8_t *ip, uint16_t port) {
    uint8_t status = socketStatus(socket);

    // Retry once on obviously invalid status (SPI glitch — 0xFF is never a valid SR value)
    if(status == 0xFF) {
        status = socketStatus(socket);
    }

    if(sockets[socket].socket_protocol == HPW5500_SOCKET_PROTOCOL_UDP) {
        // Check if port and IP are supplied
        if(ip == nullptr || port == 0) return false;

        // Check if port is open in UDP
        if(status != HPW5500_SOCKET_STATUS::UDP) return false;

        // Write destination IP
        write(HPW5500_COM_SN_DIPR(socket), ip, 4);

        // Write destination port
        write16(HPW5500_COM_SN_DPORT(socket), port);
    }

    // Check if connection is established within TCP mode
    if(sockets[socket].socket_protocol == HPW5500_SOCKET_PROTOCOL_TCP && status != HPW5500_SOCKET_STATUS::ESTABLISHED) {
        return false;
    }

    // Clear stale SEND_OK from any previous send
    uint8_t snir = read8(HPW5500_COM_SN_IR(socket));
    if(snir & HPW5500_MASK_SN_IR_SEND_OK) {
        write8(HPW5500_COM_SN_IR(socket), HPW5500_MASK_SN_IR_SEND_OK);
    }

    // Order SEND command
    write8(HPW5500_COM_SN_CR(socket), HPW5500_SOCKET_CMD::SEND);
    waitForCmdDone(socket);

    // Reset offset
    sockets[socket].offset_tracer = 0;

    return true;
}

bool HPW5500::waitForSend(uint8_t socket, uint32_t timeout_ms) {
    for(uint32_t i = 0; i < timeout_ms; i++) {
        uint8_t snir = read8(HPW5500_COM_SN_IR(socket));

        if(snir & HPW5500_MASK_SN_IR_SEND_OK) {
            write8(HPW5500_COM_SN_IR(socket), HPW5500_MASK_SN_IR_SEND_OK);
            return true;
        }

        if(snir & HPW5500_MASK_SN_IR_TIMEOUT) {
            // Don't clear TIMEOUT here — let handleSocketEvents process it
            return false;
        }

        if(snir & HPW5500_MASK_SN_IR_DISCON) {
            // Peer disconnected — bail out, let handleSocketEvents handle cleanup
            return false;
        }

    }

    return false;
}

HPW5500_socketProtocol_t HPW5500::socketProtocol(uint8_t socket) {
    uint8_t protocol = read8(HPW5500_COM_SN_MR(socket)) & HPW5500_MASK_SN_MR_PROTOCOL;

    if(protocol == HPW5500_SOCKET_PROTOCOL_TCP) return HPW5500_SOCKET_PROTOCOL_TCP;
    if(protocol == HPW5500_SOCKET_PROTOCOL_UDP) return HPW5500_SOCKET_PROTOCOL_UDP;
    if(protocol == HPW5500_SOCKET_PROTOCOL_MACRAW) return HPW5500_SOCKET_PROTOCOL_MACRAW;
    
    return HPW5500_SOCKET_PROTOCOL_CLOSED;
}

/** 
 * Register access methods
 */

void HPW5500::exchange(uint16_t addr, uint8_t block, uint8_t *dataIn, uint8_t *dataOut, uint16_t size) {
    // Compile command
    uint8_t cmd[HPW5500_CMD_SIZE];

    // Set address and block
    cmd[0] = addr >> 8;
    cmd[1] = addr & 0xFF;
    cmd[2] = block;

    // Use library default SPI transfer implementation
    transfer(cmd, dataIn, dataOut, size, this);
}

void HPW5500::read(uint16_t addr, uint8_t block, uint8_t *data, uint16_t size) {
    block = block << 3;
    
    exchange(addr, block, nullptr, data, size);
}

void HPW5500::write(uint16_t addr, uint8_t block, uint8_t *data, uint16_t size) {
    block = block << 3 | 4;

    exchange(addr, block, data, nullptr, size);
}

uint8_t HPW5500::read8(uint16_t addr, uint8_t block) {
    uint8_t buffer[1];

    read(addr, block, buffer, 1);

    return buffer[0];
}

void HPW5500::write8(uint16_t addr, uint8_t block, uint8_t data) {
    uint8_t buffer[1] = { data };

    write(addr, block, buffer, 1);
}

uint16_t HPW5500::read16(uint16_t addr, uint8_t block) {
    uint8_t buffer[2];

    read(addr, block, buffer, 2);

    return buffer[0] << 8 | buffer[1];
}

void HPW5500::write16(uint16_t addr, uint8_t block, uint16_t data) {
    uint8_t buffer[2];

    buffer[0] = data >> 8;
    buffer[1] = data & 0xFF;

    write(addr, block, buffer, 2);
}

/**
 * Callback registration
 */

void HPW5500::onDeviceEvent(HPW5500_device_event_t event, HPW5500_void_callback_t callback) {
    // Wake on WoL
    if(event == HPW5500_DEVICE_WOL) { device_events.wakeUpViaWoL = callback; return; }

    // IP conflict
    if(event == HPW5500_DEVICE_IP_CONFLICT) { device_events.IPConflict = callback; return; }

    // PPPoE close
    if(event == HPW5500_DEVICE_PPPOE_CLOSE) { device_events.PPPoEClose = callback; return; }
}

void HPW5500::onDestinationUnreachableEvent(HPW5500_unreachable_callback_t callback) {
    // Destination unreachable
    device_events.destinationUnreachable = callback;
}

void HPW5500::unlinkDeviceCallbacks() {
    // Unlink all callbacks
    device_events = HPW5500_event_callbacks_t();
}

void HPW5500::onSendOk(HPW5500_socket_callback_t callback) {
    // Register global callback
    global_socket_events.socketSendOK = callback;
}

void HPW5500::onSendOk(uint8_t socket, HPW5500_socket_callback_t callback) {
    // Register per-socket callback
    sockets[socket].events.socketSendOK = callback;
}

void HPW5500::onSendOk(HPW5500_socket_handle_t *handle, HPW5500_socket_callback_t callback) {
    // Register per-socket callback
    onSendOk(HPW5500_bitOffset(*handle), callback);
}

void HPW5500::onTimeout(HPW5500_socket_callback_t callback) {
    // Register global callback
    global_socket_events.socketTimeout = callback;
}

void HPW5500::onTimeout(uint8_t socket, HPW5500_socket_callback_t callback) {
    // Register per-socket callback
    sockets[socket].events.socketTimeout = callback;
}

void HPW5500::onTimeout(HPW5500_socket_handle_t *handle, HPW5500_socket_callback_t callback) {
    // Register per-socket callback
    onTimeout(HPW5500_bitOffset(*handle), callback);
}

void HPW5500::onMessage(HPW5500_socket_receive_callback_t callback) {
    // Register global callback
    global_socket_events.socketMessageReceive = callback;
}

void HPW5500::onMessage(uint8_t socket, HPW5500_socket_receive_callback_t callback) {
    // Register per-socket callback
    sockets[socket].events.socketMessageReceive = callback;
}

void HPW5500::onMessage(HPW5500_socket_handle_t *handle, HPW5500_socket_receive_callback_t callback) {
    // Register per-socket callback
    onMessage(HPW5500_bitOffset(*handle), callback);
}

HPW5500_socket_receive_callback_t HPW5500::swapMessageCallback(uint8_t socket, HPW5500_socket_receive_callback_t callback) {
    if(socket >= HPW5500_SOCKET_MAX) return nullptr;

    HPW5500_socket_receive_callback_t previous = sockets[socket].events.socketMessageReceive;
    sockets[socket].events.socketMessageReceive = callback;

    return previous;
}

void HPW5500::onConnect(HPW5500_socket_connect_callback_t callback) {
    // Register global callback
    global_socket_events.socketConnect = callback;
}

void HPW5500::onConnect(uint8_t socket, HPW5500_socket_connect_callback_t callback) {
    // Register per-socket callback
    sockets[socket].events.socketConnect = callback;
}

void HPW5500::onConnect(HPW5500_socket_handle_t *handle, HPW5500_socket_connect_callback_t callback) {
    // Register per-socket callback
    onConnect(HPW5500_bitOffset(*handle), callback);
}

void HPW5500::onDisconnect(HPW5500_socket_callback_t callback) {
    // Register global callback
    global_socket_events.socketDisconnect = callback;
}

void HPW5500::onDisconnect(uint8_t socket, HPW5500_socket_callback_t callback) {
    // Register per-socket callback
    sockets[socket].events.socketDisconnect = callback;
}

void HPW5500::onDisconnect(HPW5500_socket_handle_t *handle, HPW5500_socket_callback_t callback) {
    // Register per-socket callback
    onDisconnect(HPW5500_bitOffset(*handle), callback);
}

void HPW5500::onReopen(HPW5500_socket_reopen_callback_t callback) {
    // Register global callback
    global_socket_events.socketReopen = callback;
}

void HPW5500::onReopen(uint8_t socket, HPW5500_socket_reopen_callback_t callback) {
    // Register per-socket callback
    sockets[socket].events.socketReopen = callback;
}

void HPW5500::onReopen(HPW5500_socket_handle_t *handle, HPW5500_socket_reopen_callback_t callback) {
    // Register per-socket callback
    onReopen(HPW5500_bitOffset(*handle), callback);
}

void HPW5500::onConnectionTimeoutGiveUp(HPW5500_socket_giveup_callback_t callback) {
    // Register global callback
    global_socket_events.socketConnectionTimeoutGiveUp = callback;
}

void HPW5500::onConnectionTimeoutGiveUp(uint8_t socket, HPW5500_socket_giveup_callback_t callback) {
    // Register per-socket callback
    sockets[socket].events.socketConnectionTimeoutGiveUp = callback;
}

void HPW5500::onConnectionTimeoutGiveUp(HPW5500_socket_handle_t *handle, HPW5500_socket_giveup_callback_t callback) {
    // Register per-socket callback
    onConnectionTimeoutGiveUp(HPW5500_bitOffset(*handle), callback);
}

void HPW5500::unlinkGlobalSocketCallbacks() {
    // Unlink global callbacks
    global_socket_events = HPW5500_socket_event_callbacks_t();
}

void HPW5500::unlinkSocketCallbacks(uint8_t socket) {
    // Unlink callbacks only for main socket in the array
    sockets[socket].events = HPW5500_socket_event_callbacks_t();
}

void HPW5500::unlinkSocketCallbacks(HPW5500_socket_handle_t *handle) {
    // Iterate through all sockets
    for(uint8_t socket = 0; socket<HPW5500_SOCKET_MAX; socket++) {
        // Check if we are talking about the right socket here
        if(((*handle >> socket) & 0x1) != 0x1) continue;

        // Unlink all callbacks
        unlinkSocketCallbacks(socket);
    }
}
