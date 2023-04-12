
#ifndef IO_ESPNOW_HH__
#define IO_ESPNOW_HH__

#include <Arduino.h>
#include <esp_now.h>
#include <string.h>
#include <utils/compile.hh>
#include <utils/init.hh>
#include <wifi.hh>

namespace io
{
class esp_now_sink : public sink
{
  protected:
    /* Implement a buffer to temporarily store data recieved from the callbacks.
     */
    struct {
        uint8_t mem[128];
        size_t index;
    } buf_;
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

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        if (!buf_.index) {
            return 0;
        }

        buf_size = buf_size < buf_.index ? buf_size : buf_.index;

        ::memcpy(buf, buf_.mem, buf_size);
        return buf_size;
    }

  public:
    esp_now_sink(uint8_t peer_addr[6])
    {
        ::memcpy(&peer_info_.peer_addr, peer_addr, sizeof(peer_addr));

        ::esp_err_t status = ::esp_now_init();

        if (status != ESP_OK) {
            /* TODO: err handle */
            return
        }

        status = ::esp_now_add_peer(&peer_info_);

        if (status != ESP_OK) {
            return;
        }
    }

    void
    push(const uint8_t* data, size_t size)
    {
        if (size > sizeof(buf_.mem)) {
            return;
        }

        if (buf_.index + size >= sizeof(buf_.mem)) {
            buf_.index = 0;
        }

        ::memcpy(buf_.mem + buf_.index, data, size);
        buf_.index += size;
    }
};

}; // namespace io

#endif // IO_ESPNOW_HH__