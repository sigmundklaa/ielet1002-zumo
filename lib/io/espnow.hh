
#ifndef IO_ESPNOW_HH__
#define IO_ESPNOW_HH__

#include <Arduino.h>
#include <esp_now.h>
#include <io/io.hh>
#include <string.h>
#include <utils/compile.hh>
#include <utils/init.hh>
#include <wifi.hh>

namespace io
{
/**
 * @brief Ensure initialization of esp_now before being used in a constructor
 *
 */
static class esp_init__
{
  public:
    esp_init__() { ::esp_now_init(); }
} esp_init_;

/**
 * @brief Gateway for communicating with a ESP-NOW peer.
 *
 */
class esp_now_gateway : public pushable_gateway
{
  protected:
    esp_now_peer_info_t peer_info_;

    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        ::esp_err_t status = ::esp_now_send(peer_info_.peer_addr, data, size);

        if (status != ESP_OK) {
            return 0;
        }

        return size;
    }

  public:
    esp_now_gateway(uint8_t peer_addr[6])
    {
        ::memcpy(peer_info_.peer_addr, peer_addr, ESP_NOW_ETH_ALEN);

        ::esp_err_t status = ::esp_now_add_peer(&peer_info_);

        if (status != ESP_OK) {
            return;
        }
    }

    uint8_t*
    addr()
    {
        return peer_info_.peer_addr;
    }
};

}; // namespace io

#endif // IO_ESPNOW_HH__