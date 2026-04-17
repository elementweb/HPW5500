#include "HPW5500NTPServer.h"
#include <cstdio>
#include <cstring>

namespace {
	constexpr uint16_t HPW5500_NTP_PORT = 123;
	constexpr uint16_t HPW5500_NTP_PACKET_SIZE = 48;

	// NTP epoch starts 1900-01-01, Unix epoch 1970-01-01
	constexpr uint32_t HPW5500_NTP_UNIX_OFFSET = 2208988800UL;

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

HPW5500NTPServer *HPW5500NTPServer::instance_by_socket[HPW5500_SOCKET_MAX] = { nullptr };

HPW5500NTPServer::HPW5500NTPServer(HPW5500 *device) {
	this->device = device;
}

void HPW5500NTPServer::attach(HPW5500 *device) {
	this->device = device;
}

void HPW5500NTPServer::setTimeSource(HPW5500_millis_callback_t millis_cb) {
	millis_callback = millis_cb;
}

void HPW5500NTPServer::setEpochSource(HPW5500_ntp_epoch_callback_t epoch_cb) {
	epoch_callback = epoch_cb;
}

void HPW5500NTPServer::setReferenceId(const char ref_id[4]) {
	memcpy(reference_id, ref_id, 4);
}

void HPW5500NTPServer::setStratum(uint8_t stratum) {
	this->stratum = stratum;
}

void HPW5500NTPServer::setDebug(void (*callback)(const char *msg)) {
	debug_callback = callback;
}

bool HPW5500NTPServer::begin(uint16_t port) {
	if(device == nullptr || !device->connected()) return false;
	if(epoch_callback == nullptr) return false;

	HPW5500_socket_handle_t handle = 0x00;
	HPW5500_socket_init_attempt_t result = device->open(&handle, HPW5500_SOCKET_PROTOCOL_UDP, port, 1, false);
	if(result != HPW5500_SOCKET_OPEN_SUCCESS && result != HPW5500_SOCKET_OPEN_PARTIAL_SUCCESS) return false;

	socket_handle = handle;
	socket_index = HPW5500_bitOffset(handle);
	previous_callback = device->swapMessageCallback(socket_index, HPW5500NTPServer::handlePacket);
	instance_by_socket[socket_index] = this;

	log_msg(debug_callback, "NTP server: started");
	return true;
}

void HPW5500NTPServer::end() {
	if(device == nullptr || socket_index == HPW5500_UNASSOCIATED) return;

	instance_by_socket[socket_index] = nullptr;
	device->swapMessageCallback(socket_index, previous_callback);
	device->close(&socket_handle);

	socket_handle = 0x00;
	socket_index = HPW5500_UNASSOCIATED;
	previous_callback = nullptr;

	log_msg(debug_callback, "NTP server: stopped");
}

void HPW5500NTPServer::handlePacket(uint8_t socket, HPW5500_packet_t packet) {
	if(socket >= HPW5500_SOCKET_MAX) return;
	HPW5500NTPServer *instance = instance_by_socket[socket];
	if(instance == nullptr) return;
	instance->processPacket(socket, packet);
}

void HPW5500NTPServer::processPacket(uint8_t socket, const HPW5500_packet_t &packet) {
	if(packet.length < HPW5500_NTP_PACKET_SIZE) {
		log_msg(debug_callback, "NTP server: packet too short");
		return;
	}

	const uint8_t *request = packet.payload;

	// Check mode == 3 (client)
	uint8_t mode = request[0] & 0x07;
	if(mode != 3) {
		log_msg(debug_callback, "NTP server: not a client request");
		return;
	}

	log_msg(debug_callback, "NTP server: request received");

	uint8_t response[HPW5500_NTP_PACKET_SIZE];
	uint16_t length = buildResponse(request, packet.length, response, sizeof(response));
	if(length == 0) return;

	uint8_t client_ip[4];
	memcpy(client_ip, packet.ip, 4);

	device->writePacket(socket, response, length);
	device->sendPacket(socket, client_ip, packet.port);

	log_msg(debug_callback, "NTP server: response sent");
}

uint16_t HPW5500NTPServer::buildResponse(const uint8_t *request, uint16_t request_length, uint8_t *buffer, uint16_t max_len) {
	if(max_len < HPW5500_NTP_PACKET_SIZE) return 0;
	if(epoch_callback == nullptr) return 0;

	uint32_t unix_epoch = epoch_callback();
	uint32_t ntp_seconds = unix_epoch + HPW5500_NTP_UNIX_OFFSET;

	memset(buffer, 0, HPW5500_NTP_PACKET_SIZE);

	// Byte 0: LI=0 (no warning), VN=4, Mode=4 (server)
	buffer[0] = 0x24; // 00 100 100

	// Byte 1: stratum
	buffer[1] = stratum;

	// Byte 2: poll interval — copy from request
	buffer[2] = (request_length >= 3) ? request[2] : 6;

	// Byte 3: precision — approximate (-20 = ~1 microsecond)
	buffer[3] = static_cast<uint8_t>(static_cast<int8_t>(-20));

	// Bytes 4-7: root delay (0)
	// Bytes 8-11: root dispersion (0)

	// Bytes 12-15: reference ID
	memcpy(&buffer[12], reference_id, 4);

	// Bytes 16-23: reference timestamp (last sync time = current time)
	write_be32(&buffer[16], ntp_seconds);
	// fraction left as 0

	// Bytes 24-31: origin timestamp — copy client's transmit timestamp (bytes 40-47)
	if(request_length >= HPW5500_NTP_PACKET_SIZE) {
		memcpy(&buffer[24], &request[40], 8);
	}

	// Bytes 32-39: receive timestamp (now)
	write_be32(&buffer[32], ntp_seconds);

	// Bytes 40-47: transmit timestamp (now)
	write_be32(&buffer[40], ntp_seconds);

	log_u32(debug_callback, "NTP server: epoch", unix_epoch);

	return HPW5500_NTP_PACKET_SIZE;
}
