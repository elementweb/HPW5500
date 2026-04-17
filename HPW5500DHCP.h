// DHCP client for HPW5500

#ifndef CLASS_HPW5500_DHCP_H
#define CLASS_HPW5500_DHCP_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"

class HPW5500DHCP {
	public: explicit HPW5500DHCP(HPW5500 *device = nullptr, HPW5500_delay_callback_t delay_cb = nullptr, HPW5500_millis_callback_t millis_cb = nullptr);

	public: void attach(HPW5500 *device);
	public: void setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb = nullptr);

	public: bool acquire(uint16_t timeout_ms = 10000, uint8_t retries = 3);
	public: void resetLease();
	public: bool acquired() const;

	public: const HPW5500_IP_t &ip() const;
	public: const HPW5500_IP_t &subnet() const;
	public: const HPW5500_IP_t &gateway() const;
	public: const HPW5500_IP_t &server() const;
	public: uint32_t leaseTime() const;

	private: enum class State : uint8_t { Idle, DiscoverSent, RequestSent, Bound };

	private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
	private: void processPacket(const HPW5500_packet_t &packet);

	private: bool openSocket();
	private: void closeSocket();
	private: void resetState();

	private: bool sendDiscover();
	private: bool sendRequest();
	private: bool waitForOffer(uint16_t timeout_ms);
	private: bool waitForAck(uint16_t timeout_ms);

	private: uint16_t buildDiscover(uint8_t *buffer, uint16_t max_len);
	private: uint16_t buildRequest(uint8_t *buffer, uint16_t max_len);

	private: HPW5500 *device = nullptr;
	private: HPW5500_socket_handle_t socket_handle = 0x00;
	private: uint8_t socket_index = HPW5500_UNASSOCIATED;
	private: HPW5500_socket_receive_callback_t previous_callback = nullptr;

	private: HPW5500_delay_callback_t delay_callback = nullptr;
	private: HPW5500_millis_callback_t millis_callback = nullptr;

	private: State state = State::Idle;
	private: bool offer_ready = false;
	private: bool ack_ready = false;
	private: bool nak_received = false;

	private: uint32_t transaction_id = 0;

	private: HPW5500_IP_t offered_ip = { 0, 0, 0, 0 };
	private: HPW5500_IP_t subnet_mask = { 0, 0, 0, 0 };
	private: HPW5500_IP_t gateway_ip = { 0, 0, 0, 0 };
	private: HPW5500_IP_t server_ip = { 0, 0, 0, 0 };
	private: uint32_t lease_time = 0;

	private: static HPW5500DHCP *active_instance;
};

#endif // CLASS_HPW5500_DHCP_H
