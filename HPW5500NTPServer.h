// NTP server for HPW5500

#ifndef CLASS_HPW5500_NTP_SERVER_H
#define CLASS_HPW5500_NTP_SERVER_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"
#include "HPW5500NTPClient.h"

class HPW5500NTPServer {
	public: typedef uint32_t (*HPW5500_ntp_epoch_callback_t)();

	public: explicit HPW5500NTPServer(HPW5500 *device = nullptr);

	public: void attach(HPW5500 *device);

	public: bool begin(uint16_t port = 123);
	public: void end();

	public: void setTimeSource(HPW5500_millis_callback_t millis_cb);
	public: void setEpochSource(HPW5500_ntp_epoch_callback_t epoch_cb);
	public: void setReferenceId(const char ref_id[4]);
	public: void setStratum(uint8_t stratum);
	public: void setDebug(void (*callback)(const char *msg));

	private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
	private: void processPacket(uint8_t socket, const HPW5500_packet_t &packet);

	private: uint16_t buildResponse(const uint8_t *request, uint16_t request_length, uint8_t *buffer, uint16_t max_len);

	private: HPW5500 *device = nullptr;
	private: HPW5500_socket_handle_t socket_handle = 0x00;
	private: uint8_t socket_index = HPW5500_UNASSOCIATED;
	private: HPW5500_socket_receive_callback_t previous_callback = nullptr;

	private: HPW5500_millis_callback_t millis_callback = nullptr;
	private: HPW5500_ntp_epoch_callback_t epoch_callback = nullptr;
	private: void (*debug_callback)(const char *msg) = nullptr;

	private: uint8_t stratum = 1;
	private: char reference_id[4] = { 'G', 'P', 'S', '\0' };

	private: static HPW5500NTPServer *instance_by_socket[HPW5500_SOCKET_MAX];
};

#endif // CLASS_HPW5500_NTP_SERVER_H
