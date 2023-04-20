
#include <Arduino.h>
#include <io/espnow.hh>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <string.h>
#include <wifi.hh>

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

static void
redirect_mqtt_(serial_header_* header, uint8_t* buf, size_t size)
{
    static char topic_buf[20] = {0};

    /* Topic recieved in packet is not null-terminated */
    ::memcpy(topic_buf, buf, header->dst_size);

    io::mqtt_client.publish(topic_buf, buf + header->dst_size, header->size);
}

static ::esp_now_peer_info_t esp_peers_[] = {
    {
        .peer_addr =
            {
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
            },
    },
};

static void
redirect_espnow_(serial_header_* header, uint8_t* buf, size_t size)
{
}

static void
redirect_network_(serial_header_* header, uint8_t* buf, size_t size)
{
    switch (header->type) {
    case PACKET_MQTT:
        redirect_mqtt_(header, buf, size);
        break;
    case PACKET_ESPNOW:
        redirect_espnow_(header, buf, size);
        break;
    case PACKET_HOST:
        /* Not meant to be redirected anywhere */
        break;
    }
}

static void
esp_callback_(const uint8_t* mac_addr, const uint8_t* data, int recv_sz)
{
    redirect_serial_(
        PACKET_ESPNOW, buf_, mac_addr, ESP_NOW_ETH_ALEN, data, recv_sz
    );
}

static void
mqtt_callback_(char* topic, uint8_t* data, unsigned int sz)
{
    redirect_serial_(PACKET_MQTT, buf_, topic, ::strlen(topic), data, sz);
}

void
setup()
{
    WiFi.begin(SSID_NAME, SSID_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        LOG_INFO(<< "waiting for wifi connection");
        delay(500);
    }

    for (size_t i = 0; i < sizeof(esp_peers_) / sizeof(esp_peers_[0]); i++) {
        ::esp_now_add_peer(&esp_peers_[i]);
    }

    ::esp_now_register_recv_cb(esp_callback_);

    io::mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
    io::mqtt_client.setCallback(mqtt_callback_);
}

void
loop()
{
    static uint8_t buf[256];

    size_t bread = serial_sink_.read(buf, sizeof(serial_header_));

    if (bread != 0) {
        serial_header_* header = reinterpret_cast<serial_header_*>(buf);
        bread +=
            serial_sink_.read(buf + bread, header->dst_size + header->size);

        redirect_network_(header, buf + sizeof(*header), bread);
    }
}