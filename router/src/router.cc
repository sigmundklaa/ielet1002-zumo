
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
    uint8_t dst_size;
    uint8_t size;
};

static uint8_t buf_[256];

static size_t
prepare_header_(
    header_type_ type, uint8_t* buf, const void* dst_data, size_t dst_size,
    size_t data_size
)
{
    size_t size = dst_size + data_size;
    assert(size <= UINT8_MAX);

    serial_header_* header = reinterpret_cast<serial_header_*>(buf);
    header->type = type;
    header->dst_size = static_cast<uint8_t>(dst_size);

    if (size > MAX_PACKET_SIZE_) {
        LOG_ERR(<< "packet too big");

        return 0;
    }

    if (size > 0) {
        ::memcpy(buf + sizeof(*header), dst_data, dst_size);
    }

    return sizeof(*header) + dst_size;
}

static void
redirect_serial_(
    header_type_ type, uint8_t* buf, const void* header_data,
    size_t header_size, const void* data, size_t data_size
)
{
    size_t size =
        prepare_header_(type, buf_, header_data, header_size, data_size);

    ::memcpy(buf_ + size, data, data_size);

    size += data_size;
    if (serial_sink_.write(buf_, size) != size) {
        LOG_ERR(<< "serial write blocked");
    }
}

static io::esp_now_sink esp_sinks_[2];

static void
esp_callback(const uint8_t* mac_addr, const uint8_t* data, size_t recv_sz)
{
    for (size_t i = 0; i < sizeof(esp_sinks_) / sizeof(esp_sinks_[0]); i++) {
        if (memcmp(mac_addr, esp_sinks_[i].addr(), ESP_NOW_ETH_ALEN) == 0) {
            redirect_serial_(
                PACKET_ESPNOW, buf_, mac_addr, ESP_NOW_ETH_ALEN, data, recv_sz
            );

            return;
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