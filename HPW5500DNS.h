// DNS client for HPW5500

#ifndef CLASS_HPW5500_DNS_H
#define CLASS_HPW5500_DNS_H

#include <stdint.h>
#include <cstring>

#include "HPW5500.h"

class HPW5500DNS {
    public: explicit HPW5500DNS(HPW5500 *device = nullptr,
                                HPW5500_delay_callback_t delay_cb = nullptr,
                                HPW5500_millis_callback_t millis_cb = nullptr,
                                const uint8_t *default_dns = nullptr,
                                uint8_t cache_size = HPW5500_DNS_CACHE_SIZE);

    public: void attach(HPW5500 *device);
    public: void setTimeSource(HPW5500_millis_callback_t millis_cb, HPW5500_delay_callback_t delay_cb = nullptr);
    public: void setServer(const HPW5500_IP_t &dns_server);
    public: void setDebug(void (*callback)(const char *msg));

    public: bool resolve(const char *hostname, HPW5500_IP_t &out_ip, uint16_t timeout_ms = 2000, uint8_t retries = 2);

    private: static void handlePacket(uint8_t socket, HPW5500_packet_t packet);
    private: void processPacket(const HPW5500_packet_t &packet);

    private: bool openSocket();
    private: void closeSocket();

    private: bool sendQuery(const char *hostname);
    private: bool waitForResponse(uint16_t timeout_ms);
    private: uint16_t buildQuery(const char *hostname, uint8_t *buffer, uint16_t max_len);

    private: bool readFromCache(const char *hostname, HPW5500_IP_t &out_ip);
    private: void storeInCache(const char *hostname, const HPW5500_IP_t &ip, uint32_t ttl_seconds);

    private: HPW5500 *device = nullptr;
    private: HPW5500_IP_t dns_server_ip = { 0, 0, 0, 0 };
    private: HPW5500_IP_t default_dns_server = { 8, 8, 8, 8 };
    private: HPW5500_socket_handle_t socket_handle = 0x00;
    private: uint8_t socket_index = HPW5500_UNASSOCIATED;
    private: HPW5500_socket_receive_callback_t previous_callback = nullptr;

    private: HPW5500_delay_callback_t delay_callback = nullptr;
    private: HPW5500_millis_callback_t millis_callback = nullptr;
    private: void (*debug_callback)(const char *msg) = nullptr;

    private: uint16_t transaction_id = 0;
    private: bool response_ready = false;
    private: bool response_ok = false;
    private: HPW5500_IP_t resolved_ip = { 0, 0, 0, 0 };
    private: uint32_t resolved_ttl = 0;

    private: static constexpr uint8_t HPW5500_DNS_CACHE_SIZE = 4;
    private: static constexpr uint16_t HPW5500_DNS_CACHE_HOST_MAX = 128;

    private: struct CacheEntry {
        bool valid = false;
        char host[HPW5500_DNS_CACHE_HOST_MAX] = { 0 };
        HPW5500_IP_t ip = { 0, 0, 0, 0 };
        HPW5500_millis_t expires_at_ms = 0;
    };

    private: CacheEntry cache[HPW5500_DNS_CACHE_SIZE];
    private: uint8_t cache_size = HPW5500_DNS_CACHE_SIZE;
    private: uint8_t cache_next_index = 0;

    private: static HPW5500DNS *active_instance;
};

#endif // CLASS_HPW5500_DNS_H
