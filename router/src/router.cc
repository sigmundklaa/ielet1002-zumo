
#include <Arduino.h>
#include <io/espnow.hh>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <string.h>

#define RX_PIN_ (26)
#define TX_PIN_ (22)

static io::serial_sink<HardwareSerial>
    serial_sink_(Serial2, 115200, RX_PIN_, TX_PIN_);

static io::serial_sink<HardwareSerial> log_sink_(Serial, 9600);

#define LOG_MODULE router
LOG_REGISTER(log_sink_);

#define BUF_SIZE_ (512)
#define MAX_PACKET_SIZE_ (256 - sizeof(serial_header_))

enum header_type_ {
    PACKET_MQTT,
    PACKET_ESPNOW,
    PACKET_HOST,
} __attribute__((packed));

static_assert(sizeof(header_type_) == 1, "size of header type is 1");

struct __attribute__((packed)) serial_header_ {
    header_type_ type;
    uint8_t argc;
};

static size_t
prepare_header_(header_type_ type, uint8_t* buf, void* data, size_t size)
{
    assert(size <= UINT8_MAX);

    serial_header_* header = reinterpret_cast<serial_header_*>(buf);
    header->type = type;
    header->argc = static_cast<uint8_t>(size);

    if (size > MAX_PACKET_SIZE_) {
        LOG_ERR(<< "packet too big");

        return 0;
    }

    if (size > 0) {
        ::memcpy(buf + sizeof(*header), data, size);
    }

    return sizeof(*header) + size;
}

static io::esp_now_sink esp_sinks_[2];

static void
esp_callback(const uint8_t* mac_addr, const uint8_t* data, size_t recv_sz)
{
    for (size_t i = 0; i < sizeof(esp_sinks_) / sizeof(esp_sinks_[0]); i++) {
        if (memcmp(mac_addr, esp_sinks_[i].addr(), ESP_NOW_ETH_ALEN) == 0) {
            /* Send through serial */
        }
    }

    LOG_ERR(<< "unknown mac address, discarding");
}

void
setup()
{
    // put your setup code here, to run once:
}

void
loop()
{
    // put your main code here, to run repeatedly:
}