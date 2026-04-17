// NTP client for HPW5500

#ifndef CLASS_HPW5500_NTP_CLIENT_H
#define CLASS_HPW5500_NTP_CLIENT_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"

struct HPW5500NTPTimestamp {
	uint32_t seconds = 0;
	uint32_t fraction = 0;
};

class HPW5500NTPClient {
	public: explicit HPW5500NTPClient(HPW5500 *device = nullptr,
	                                  HPW5500_delay_callback_t delay_cb = nullptr,
	                                  HPW5500_millis_callback_t millis_cb = nullptr,
	                                  const uint8_t *default_server = nullptr);

	public: void attach(HPW5500 *device);
	public: void setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb = nullptr);
	public: void setServer(const HPW5500_IP_t &server_ip);
	public: void setDebug(void (*callback)(const char *msg));

	public: bool sync(uint16_t timeout_ms = 2000, uint8_t retries = 2);

	public: bool synced() const;
	public: uint32_t epoch() const;
	public: uint32_t epochLocal() const;
	public: void setTimezoneOffset(int32_t offset_seconds);
	public: int32_t timezoneOffset() const;

	public: HPW5500NTPTimestamp timestamp() const;
	public: HPW5500_millis_t lastSyncMillis() const;

	private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
	private: void processPacket(const HPW5500_packet_t &packet);

	private: bool openSocket();
	private: void closeSocket();

	private: bool sendRequest();
	private: bool waitForResponse(uint16_t timeout_ms);
	private: uint16_t buildRequest(uint8_t *buffer, uint16_t max_len);

	private: HPW5500 *device = nullptr;
	private: HPW5500_IP_t server_ip = { 0, 0, 0, 0 };
	private: HPW5500_IP_t default_server_ip = { 162, 159, 200, 1 };
	private: HPW5500_socket_handle_t socket_handle = 0x00;
	private: uint8_t socket_index = HPW5500_UNASSOCIATED;
	private: HPW5500_socket_receive_callback_t previous_callback = nullptr;

	private: HPW5500_delay_callback_t delay_callback = nullptr;
	private: HPW5500_millis_callback_t millis_callback = nullptr;
	private: void (*debug_callback)(const char *msg) = nullptr;

	private: bool response_ready = false;
	private: bool response_ok = false;

	private: bool has_synced = false;
	private: HPW5500NTPTimestamp synced_timestamp;
	private: HPW5500_millis_t synced_at_millis = 0;
	private: int32_t timezone_offset_seconds = 0;

	private: static HPW5500NTPClient *active_instance;
};

#endif // CLASS_HPW5500_NTP_CLIENT_H
