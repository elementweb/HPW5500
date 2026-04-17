#include "HPW5500NTPClient.h"
#include <cstdio>
#include <cstring>

namespace {
	constexpr uint16_t HPW5500_NTP_PORT = 123;
	constexpr uint16_t HPW5500_NTP_PACKET_SIZE = 48;

	// NTP epoch starts 1900-01-01, Unix epoch 1970-01-01 — difference in seconds
	constexpr uint32_t HPW5500_NTP_UNIX_OFFSET = 2208988800UL;

	// NTP version 4, client mode
	constexpr uint8_t HPW5500_NTP_LI_VN_MODE = 0x23; // LI=0, VN=4, Mode=3 (client)

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

	void log_msg(void (*callback)(const char *), const char *msg) {
		if(callback != nullptr) callback(msg);
	}

	void log_u32(void (*callback)(const char *), const char *label, uint32_t value) {
		if(callback == nullptr) return;
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%s: %lu", label, static_cast<unsigned long>(value));
		callback(buffer);
	}
}

HPW5500NTPClient *HPW5500NTPClient::active_instance = nullptr;

HPW5500NTPClient::HPW5500NTPClient(HPW5500 *device,
                                   HPW5500_delay_callback_t delay_cb,
                                   HPW5500_millis_callback_t millis_cb,
                                   const uint8_t *default_server) {
	this->device = device;
	delay_callback = delay_cb;
	millis_callback = millis_cb;

	if(default_server != nullptr) memcpy(default_server_ip, default_server, sizeof(default_server_ip));
}

void HPW5500NTPClient::attach(HPW5500 *device) {
	this->device = device;
}

void HPW5500NTPClient::setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb) {
	millis_callback = millis_cb;
	delay_callback = delay_cb;
}

void HPW5500NTPClient::setServer(const HPW5500_IP_t &server) {
	memcpy(server_ip, server, sizeof(server_ip));
}

void HPW5500NTPClient::setDebug(void (*callback)(const char *msg)) {
	debug_callback = callback;
}

bool HPW5500NTPClient::sync(uint16_t timeout_ms, uint8_t retries) {
	if(device == nullptr || !device->connected()) return false;

	bool server_set = server_ip[0] || server_ip[1] || server_ip[2] || server_ip[3];
	if(!server_set) memcpy(server_ip, default_server_ip, sizeof(server_ip));

	server_set = server_ip[0] || server_ip[1] || server_ip[2] || server_ip[3];
	if(!server_set) {
		log_msg(debug_callback, "NTP: server not set");
		return false;
	}

	if(!openSocket()) {
		log_msg(debug_callback, "NTP: socket open failed");
		return false;
	}

	active_instance = this;
	bool success = false;

	log_msg(debug_callback, "NTP: sync start");
	for(uint8_t attempt = 0; attempt < retries && !success; attempt++) {
		response_ready = false;
		response_ok = false;

		log_u32(debug_callback, "NTP: attempt", attempt + 1);
		if(!sendRequest()) {
			log_msg(debug_callback, "NTP: send request failed");
			continue;
		}
		if(!waitForResponse(timeout_ms)) {
			log_msg(debug_callback, "NTP: wait timeout or error");
			continue;
		}

		if(response_ready && response_ok) {
			has_synced = true;
			if(millis_callback != nullptr) synced_at_millis = millis_callback();
			success = true;
			log_u32(debug_callback, "NTP: epoch", epoch());
		}
	}

	closeSocket();
	active_instance = nullptr;

	return success;
}

bool HPW5500NTPClient::synced() const {
	return has_synced;
}

uint32_t HPW5500NTPClient::epoch() const {
	if(!has_synced) return 0;

	uint32_t ntp_seconds = synced_timestamp.seconds;
	if(ntp_seconds < HPW5500_NTP_UNIX_OFFSET) return 0;

	uint32_t unix_epoch = ntp_seconds - HPW5500_NTP_UNIX_OFFSET;

	// Compensate for elapsed time since sync
	if(millis_callback != nullptr) {
		HPW5500_millis_t now = millis_callback();
		HPW5500_millis_t elapsed_ms = now - synced_at_millis;
		unix_epoch += elapsed_ms / 1000;
	}

	return unix_epoch;
}

uint32_t HPW5500NTPClient::epochLocal() const {
	uint32_t e = epoch();
	if(e == 0) return 0;
	return static_cast<uint32_t>(static_cast<int64_t>(e) + timezone_offset_seconds);
}

void HPW5500NTPClient::setTimezoneOffset(int32_t offset_seconds) {
	timezone_offset_seconds = offset_seconds;
}

int32_t HPW5500NTPClient::timezoneOffset() const {
	return timezone_offset_seconds;
}

HPW5500NTPTimestamp HPW5500NTPClient::timestamp() const {
	return synced_timestamp;
}

HPW5500_millis_t HPW5500NTPClient::lastSyncMillis() const {
	return synced_at_millis;
}

void HPW5500NTPClient::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
	if(active_instance == nullptr) return;
	if(socket != active_instance->socket_index) return;
	active_instance->processPacket(packet);
}

void HPW5500NTPClient::processPacket(const HPW5500_packet_t &packet) {
	if(packet.length < HPW5500_NTP_PACKET_SIZE) {
		log_msg(debug_callback, "NTP: packet too short");
		response_ready = true;
		response_ok = false;
		return;
	}

	const uint8_t *buffer = packet.payload;

	// Byte 0: LI (2 bits) | VN (3 bits) | Mode (3 bits)
	uint8_t li_vn_mode = buffer[0];
	uint8_t mode = li_vn_mode & 0x07;
	uint8_t li = (li_vn_mode >> 6) & 0x03;

	// Mode must be 4 (server) or 5 (broadcast)
	if(mode != 4 && mode != 5) {
		log_msg(debug_callback, "NTP: unexpected mode");
		response_ready = true;
		response_ok = false;
		return;
	}

	// LI == 3 means clock not synchronized (kiss-o'-death)
	if(li == 3) {
		log_msg(debug_callback, "NTP: server unsynchronized (LI=3)");
		response_ready = true;
		response_ok = false;
		return;
	}

	// Byte 1: stratum — 0 = KoD, 1 = primary, 2-15 = secondary
	uint8_t stratum = buffer[1];
	if(stratum == 0) {
		log_msg(debug_callback, "NTP: kiss-o-death (stratum 0)");
		response_ready = true;
		response_ok = false;
		return;
	}

	// Transmit timestamp at offset 40-47 (seconds at 40-43, fraction at 44-47)
	uint32_t tx_seconds = read_be32(&buffer[40]);
	uint32_t tx_fraction = read_be32(&buffer[44]);

	if(tx_seconds == 0) {
		log_msg(debug_callback, "NTP: zero transmit timestamp");
		response_ready = true;
		response_ok = false;
		return;
	}

	synced_timestamp.seconds = tx_seconds;
	synced_timestamp.fraction = tx_fraction;

	response_ready = true;
	response_ok = true;
	log_msg(debug_callback, "NTP: response received");
}

bool HPW5500NTPClient::openSocket() {
	if(socket_index != HPW5500_UNASSOCIATED) return true;
	if(device == nullptr) return false;

	HPW5500_socket_handle_t handle = 0x00;
	HPW5500_socket_init_attempt_t result = device->open(&handle, HPW5500_SOCKET_PROTOCOL_UDP, HPW5500_PORT_AUTOSELECT, 1, false);
	if(result != HPW5500_SOCKET_OPEN_SUCCESS && result != HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS) return false;

	socket_handle = handle;
	socket_index = HPW5500_bitOffset(handle);
	previous_callback = device->swapMessageCallback(socket_index, HPW5500NTPClient::handlePacket);

	return true;
}

void HPW5500NTPClient::closeSocket() {
	if(device == nullptr || socket_index == HPW5500_UNASSOCIATED) return;

	device->swapMessageCallback(socket_index, previous_callback);
	device->close(&socket_handle);

	socket_handle = 0x00;
	socket_index = HPW5500_UNASSOCIATED;
	previous_callback = nullptr;
}

bool HPW5500NTPClient::sendRequest() {
	uint8_t buffer[HPW5500_NTP_PACKET_SIZE];
	uint16_t length = buildRequest(buffer, sizeof(buffer));
	if(length == 0) {
		log_msg(debug_callback, "NTP: build request failed");
		return false;
	}

	device->writePacket(socket_index, buffer, length);
	return device->sendPacket(socket_index, server_ip, HPW5500_NTP_PORT);
}

bool HPW5500NTPClient::waitForResponse(uint16_t timeout_ms) {
	HPW5500_millis_t start = millis_callback != nullptr ? millis_callback() : 0;
	uint32_t elapsed = 0;
	const uint16_t step = 10;

	while(true) {
		if(device != nullptr) device->process(true);

		if(response_ready) return response_ok;

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

uint16_t HPW5500NTPClient::buildRequest(uint8_t *buffer, uint16_t max_len) {
	if(max_len < HPW5500_NTP_PACKET_SIZE) return 0;

	memset(buffer, 0, HPW5500_NTP_PACKET_SIZE);

	// Byte 0: LI=0 (no warning), VN=4, Mode=3 (client)
	buffer[0] = HPW5500_NTP_LI_VN_MODE;

	return HPW5500_NTP_PACKET_SIZE;
}
