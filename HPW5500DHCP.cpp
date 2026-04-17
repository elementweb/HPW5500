#include "HPW5500DHCP.h"

namespace {
	constexpr uint8_t HPW5500_DHCP_OP_REQUEST = 0x01;
	constexpr uint8_t HPW5500_DHCP_OP_REPLY = 0x02;
	constexpr uint8_t HPW5500_DHCP_HTYPE_ETHERNET = 0x01;
	constexpr uint8_t HPW5500_DHCP_HLEN_ETHERNET = 0x06;

	constexpr uint16_t HPW5500_DHCP_CLIENT_PORT = 68;
	constexpr uint16_t HPW5500_DHCP_SERVER_PORT = 67;

	constexpr uint8_t HPW5500_DHCP_MAGIC_COOKIE[4] = { 99, 130, 83, 99 };
	constexpr uint16_t HPW5500_DHCP_FIXED_SIZE = 240;
	constexpr uint16_t HPW5500_DHCP_MAX_MESSAGE = 300;

	constexpr uint8_t HPW5500_DHCP_OPTION_PAD = 0x00;
	constexpr uint8_t HPW5500_DHCP_OPTION_SUBNET = 0x01;
	constexpr uint8_t HPW5500_DHCP_OPTION_ROUTER = 0x03;
	constexpr uint8_t HPW5500_DHCP_OPTION_REQUESTED_IP = 0x32;
	constexpr uint8_t HPW5500_DHCP_OPTION_LEASE_TIME = 0x33;
	constexpr uint8_t HPW5500_DHCP_OPTION_MESSAGE_TYPE = 0x35;
	constexpr uint8_t HPW5500_DHCP_OPTION_SERVER_ID = 0x36;
	constexpr uint8_t HPW5500_DHCP_OPTION_PARAMETER_LIST = 0x37;
	constexpr uint8_t HPW5500_DHCP_OPTION_CLIENT_ID = 0x3D;
	constexpr uint8_t HPW5500_DHCP_OPTION_END = 0xFF;

	constexpr uint8_t HPW5500_DHCP_MESSAGE_DISCOVER = 0x01;
	constexpr uint8_t HPW5500_DHCP_MESSAGE_OFFER = 0x02;
	constexpr uint8_t HPW5500_DHCP_MESSAGE_REQUEST = 0x03;
	constexpr uint8_t HPW5500_DHCP_MESSAGE_ACK = 0x05;
	constexpr uint8_t HPW5500_DHCP_MESSAGE_NAK = 0x06;

	uint32_t read_be32(const uint8_t *buffer) {
		return (static_cast<uint32_t>(buffer[0]) << 24)
			| (static_cast<uint32_t>(buffer[1]) << 16)
			| (static_cast<uint32_t>(buffer[2]) << 8)
			| static_cast<uint32_t>(buffer[3]);
	}

	void write_be32(uint8_t *buffer, uint32_t value) {
		buffer[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
		buffer[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
		buffer[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
		buffer[3] = static_cast<uint8_t>(value & 0xFF);
	}

	void append_option(uint8_t *buffer, uint16_t &offset, uint16_t max_len, uint8_t code, const uint8_t *data, uint8_t len) {
		if(offset + 2 + len > max_len) return;
		buffer[offset++] = code;
		buffer[offset++] = len;
		if(len > 0 && data != nullptr) {
			memcpy(&buffer[offset], data, len);
			offset += len;
		}
	}
}

HPW5500DHCP *HPW5500DHCP::active_instance = nullptr;

HPW5500DHCP::HPW5500DHCP(HPW5500 *device, HPW5500_delay_callback_t delay_cb, HPW5500_millis_callback_t millis_cb) {
	this->device = device;
	delay_callback = delay_cb;
	millis_callback = millis_cb;
}

void HPW5500DHCP::attach(HPW5500 *device) {
	this->device = device;
}

void HPW5500DHCP::setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb) {
	millis_callback = millis_cb;
	delay_callback = delay_cb;
}

void HPW5500DHCP::resetLease() {
	state = State::Idle;
	offer_ready = false;
	ack_ready = false;
	nak_received = false;
	lease_time = 0;
	memset(offered_ip, 0, sizeof(offered_ip));
	memset(subnet_mask, 0, sizeof(subnet_mask));
	memset(gateway_ip, 0, sizeof(gateway_ip));
	memset(server_ip, 0, sizeof(server_ip));
	state = State::Idle;
}

bool HPW5500DHCP::acquired() const {
	return state == State::Bound;
}

const HPW5500_IP_t &HPW5500DHCP::ip() const {
	return offered_ip;
}

const HPW5500_IP_t &HPW5500DHCP::subnet() const {
	return subnet_mask;
}

const HPW5500_IP_t &HPW5500DHCP::gateway() const {
	return gateway_ip;
}

const HPW5500_IP_t &HPW5500DHCP::server() const {
	return server_ip;
}

uint32_t HPW5500DHCP::leaseTime() const {
	return lease_time;
}

bool HPW5500DHCP::acquire(uint16_t timeout_ms, uint8_t retries) {
	if(device == nullptr || !device->connected()) return false;

	if(!openSocket()) return false;

	active_instance = this;
	bool success = false;

	for(uint8_t attempt = 0; attempt < retries && !success; attempt++) {
		resetState();

		transaction_id = 0xA5A50000 | ((attempt + 1) & 0xFF);
		if(millis_callback != nullptr) transaction_id ^= millis_callback();

		if(!sendDiscover()) continue;
		state = State::DiscoverSent;

		if(!waitForOffer(timeout_ms)) continue;

		if(!sendRequest()) continue;
		state = State::RequestSent;

		if(!waitForAck(timeout_ms)) continue;

		if(ack_ready) {
			device->configureIP(offered_ip);
			device->configureSubnet(subnet_mask);
			device->configureGateway(gateway_ip);
			device->applyNetworkConfig(true);
			state = State::Bound;
			success = true;
		}
	}

	closeSocket();
	active_instance = nullptr;

	return success;
}

void HPW5500DHCP::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
	if(active_instance == nullptr) return;
	if(socket != active_instance->socket_index) return;
	active_instance->processPacket(packet);
}

void HPW5500DHCP::processPacket(const HPW5500_packet_t &packet) {
	if(packet.length < HPW5500_DHCP_FIXED_SIZE) return;

	const uint8_t *buffer = packet.payload;

	if(buffer[0] != HPW5500_DHCP_OP_REPLY) return;
	if(buffer[1] != HPW5500_DHCP_HTYPE_ETHERNET) return;
	if(buffer[2] != HPW5500_DHCP_HLEN_ETHERNET) return;

	uint32_t xid = read_be32(&buffer[4]);
	if(xid != transaction_id) return;

	if(device == nullptr) return;
	if(memcmp(&buffer[28], device->configuration.mac, 6) != 0) return;

	if(memcmp(&buffer[236], HPW5500_DHCP_MAGIC_COOKIE, 4) != 0) return;

	uint8_t msg_type = 0;
	bool subnet_set = false;
	bool router_set = false;
	bool server_set = false;
	bool lease_set = false;

	uint16_t offset = HPW5500_DHCP_FIXED_SIZE;
	while(offset < packet.length) {
		uint8_t code = buffer[offset++];
		if(code == HPW5500_DHCP_OPTION_PAD) continue;
		if(code == HPW5500_DHCP_OPTION_END) break;
		if(offset >= packet.length) break;
		uint8_t len = buffer[offset++];
		if(offset + len > packet.length) break;

		if(code == HPW5500_DHCP_OPTION_MESSAGE_TYPE && len >= 1) {
			msg_type = buffer[offset];
		} else if(code == HPW5500_DHCP_OPTION_SUBNET && len == 4) {
			memcpy(subnet_mask, &buffer[offset], 4);
			subnet_set = true;
		} else if(code == HPW5500_DHCP_OPTION_ROUTER && len >= 4) {
			memcpy(gateway_ip, &buffer[offset], 4);
			router_set = true;
		} else if(code == HPW5500_DHCP_OPTION_SERVER_ID && len == 4) {
			memcpy(server_ip, &buffer[offset], 4);
			server_set = true;
		} else if(code == HPW5500_DHCP_OPTION_LEASE_TIME && len == 4) {
			lease_time = read_be32(&buffer[offset]);
			lease_set = true;
		}

		offset += len;
	}

	if(!server_set && packet.port == HPW5500_DHCP_SERVER_PORT) {
		memcpy(server_ip, packet.ip, 4);
	}

	if(msg_type == HPW5500_DHCP_MESSAGE_OFFER) {
		memcpy(offered_ip, &buffer[16], 4);
		offer_ready = true;
		ack_ready = false;
		nak_received = false;
		return;
	}

	if(msg_type == HPW5500_DHCP_MESSAGE_ACK) {
		memcpy(offered_ip, &buffer[16], 4);
		ack_ready = true;
		nak_received = false;
		return;
	}

	if(msg_type == HPW5500_DHCP_MESSAGE_NAK) {
		nak_received = true;
	}
}

bool HPW5500DHCP::openSocket() {
	if(socket_index != HPW5500_UNASSOCIATED) return true;
	if(device == nullptr) return false;

	HPW5500_socket_handle_t handle = 0x00;
	HPW5500_socket_init_attempt_t result = device->open(&handle, HPW5500_SOCKET_PROTOCOL_UDP, HPW5500_DHCP_CLIENT_PORT, 1, false);
	if(result != HPW5500_SOCKET_OPEN_SUCCESS && result != HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS) return false;

	socket_handle = handle;
	socket_index = HPW5500_bitOffset(handle);
	previous_callback = device->swapMessageCallback(socket_index, HPW5500DHCP::handlePacket);

	device->configureIP(0, 0, 0, 0);
	device->configureSubnet(0, 0, 0, 0);
	device->configureGateway(0, 0, 0, 0);
	device->applyNetworkConfig(true);

	return true;
}

void HPW5500DHCP::closeSocket() {
	if(device == nullptr || socket_index == HPW5500_UNASSOCIATED) return;

	device->swapMessageCallback(socket_index, previous_callback);
	device->close(&socket_handle);

	socket_handle = 0x00;
	socket_index = HPW5500_UNASSOCIATED;
	previous_callback = nullptr;
}

void HPW5500DHCP::resetState() {
	offer_ready = false;
	ack_ready = false;
	nak_received = false;
	lease_time = 0;
	memset(offered_ip, 0, sizeof(offered_ip));
	memset(subnet_mask, 0, sizeof(subnet_mask));
	memset(gateway_ip, 0, sizeof(gateway_ip));
	memset(server_ip, 0, sizeof(server_ip));
}

bool HPW5500DHCP::sendDiscover() {
	uint8_t buffer[HPW5500_DHCP_MAX_MESSAGE];
	uint16_t length = buildDiscover(buffer, sizeof(buffer));
	if(length == 0) return false;

	device->writePacket(socket_index, buffer, length);
	uint8_t broadcast[4] = { 255, 255, 255, 255 };
	return device->sendPacket(socket_index, broadcast, HPW5500_DHCP_SERVER_PORT);
}

bool HPW5500DHCP::sendRequest() {
	uint8_t buffer[HPW5500_DHCP_MAX_MESSAGE];
	uint16_t length = buildRequest(buffer, sizeof(buffer));
	if(length == 0) return false;

	device->writePacket(socket_index, buffer, length);
	uint8_t broadcast[4] = { 255, 255, 255, 255 };
	return device->sendPacket(socket_index, broadcast, HPW5500_DHCP_SERVER_PORT);
}

bool HPW5500DHCP::waitForOffer(uint16_t timeout_ms) {
	HPW5500_millis_t start = millis_callback != nullptr ? millis_callback() : 0;
	uint32_t elapsed = 0;
	const uint16_t step = 10;

	while(true) {
		if(device != nullptr) device->process(true);

		if(offer_ready) return true;
		if(nak_received) return false;

		if(millis_callback != nullptr) {
			HPW5500_millis_t now = millis_callback();
			if(static_cast<HPW5500_millis_t>(now - start) >= timeout_ms) return false;
		} else {
			if(elapsed >= timeout_ms) return false;
			elapsed += step;
		}

		if(delay_callback != nullptr) delay_callback(step);
	}
}

bool HPW5500DHCP::waitForAck(uint16_t timeout_ms) {
	HPW5500_millis_t start = millis_callback != nullptr ? millis_callback() : 0;
	uint32_t elapsed = 0;
	const uint16_t step = 10;

	while(true) {
		if(device != nullptr) device->process(true);

		if(ack_ready) return true;
		if(nak_received) return false;

		if(millis_callback != nullptr) {
			HPW5500_millis_t now = millis_callback();
			if(static_cast<HPW5500_millis_t>(now - start) >= timeout_ms) return false;
		} else {
			if(elapsed >= timeout_ms) return false;
			elapsed += step;
		}

		if(delay_callback != nullptr) delay_callback(step);
	}
}

uint16_t HPW5500DHCP::buildDiscover(uint8_t *buffer, uint16_t max_len) {
	if(max_len < HPW5500_DHCP_FIXED_SIZE + 8) return 0;

	memset(buffer, 0, max_len);
	buffer[0] = HPW5500_DHCP_OP_REQUEST;
	buffer[1] = HPW5500_DHCP_HTYPE_ETHERNET;
	buffer[2] = HPW5500_DHCP_HLEN_ETHERNET;
	buffer[3] = 0x00;

	write_be32(&buffer[4], transaction_id);

	buffer[10] = 0x80;
	buffer[11] = 0x00;

	if(device != nullptr) memcpy(&buffer[28], device->configuration.mac, 6);

	memcpy(&buffer[236], HPW5500_DHCP_MAGIC_COOKIE, 4);

	uint16_t offset = HPW5500_DHCP_FIXED_SIZE;
	uint8_t msg_type = HPW5500_DHCP_MESSAGE_DISCOVER;
	append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_MESSAGE_TYPE, &msg_type, 1);

	uint8_t client_id[7] = { 1, 0, 0, 0, 0, 0, 0 };
	if(device != nullptr) memcpy(&client_id[1], device->configuration.mac, 6);
	append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_CLIENT_ID, client_id, sizeof(client_id));

	uint8_t params[3] = { HPW5500_DHCP_OPTION_SUBNET, HPW5500_DHCP_OPTION_ROUTER, HPW5500_DHCP_OPTION_LEASE_TIME };
	append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_PARAMETER_LIST, params, sizeof(params));

	buffer[offset++] = HPW5500_DHCP_OPTION_END;

	return offset;
}

uint16_t HPW5500DHCP::buildRequest(uint8_t *buffer, uint16_t max_len) {
	if(max_len < HPW5500_DHCP_FIXED_SIZE + 16) return 0;

	memset(buffer, 0, max_len);
	buffer[0] = HPW5500_DHCP_OP_REQUEST;
	buffer[1] = HPW5500_DHCP_HTYPE_ETHERNET;
	buffer[2] = HPW5500_DHCP_HLEN_ETHERNET;
	buffer[3] = 0x00;

	write_be32(&buffer[4], transaction_id);

	buffer[10] = 0x80;
	buffer[11] = 0x00;

	if(device != nullptr) memcpy(&buffer[28], device->configuration.mac, 6);

	memcpy(&buffer[236], HPW5500_DHCP_MAGIC_COOKIE, 4);

	uint16_t offset = HPW5500_DHCP_FIXED_SIZE;
	uint8_t msg_type = HPW5500_DHCP_MESSAGE_REQUEST;
	append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_MESSAGE_TYPE, &msg_type, 1);

	append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_REQUESTED_IP, offered_ip, 4);

	if(server_ip[0] != 0 || server_ip[1] != 0 || server_ip[2] != 0 || server_ip[3] != 0) {
		append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_SERVER_ID, server_ip, 4);
	}

	uint8_t client_id[7] = { 1, 0, 0, 0, 0, 0, 0 };
	if(device != nullptr) memcpy(&client_id[1], device->configuration.mac, 6);
	append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_CLIENT_ID, client_id, sizeof(client_id));

	uint8_t params[3] = { HPW5500_DHCP_OPTION_SUBNET, HPW5500_DHCP_OPTION_ROUTER, HPW5500_DHCP_OPTION_LEASE_TIME };
	append_option(buffer, offset, max_len, HPW5500_DHCP_OPTION_PARAMETER_LIST, params, sizeof(params));

	buffer[offset++] = HPW5500_DHCP_OPTION_END;

	return offset;
}
