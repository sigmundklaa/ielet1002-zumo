
#include <Arduino.h>
#include <WiFi.h>
#include <io/espnow.hh>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <string.h>
#include <utils/crc32.hh>
#include <utils/trace.hh>
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
    header->size = data_size;

    if (size > MAX_PACKET_SIZE_) {
        LOG_ERR(<< "packet too big");

        return 0;
    }

    if (dst_size > 0) {
        ::memcpy(buf + sizeof(*header), dst_data, dst_size);
    }

    return sizeof(*header) + dst_size;
}

/**
 * @brief Insert a CRC-32 checksum at the front of the buffer @p buf
 *
 * @param buf
 * @param size
 * @return size_t Size of checksum
 */
static size_t
insert_crc_(uint8_t* buf, size_t size)
{
    uint32_t crc = utils::crc32(buf, size);
    ::memmove(buf + sizeof(crc), buf, size);
    ::memcpy(buf, &crc, sizeof(crc));

    return sizeof(crc);
}

static void
redirect_serial_(
    header_type_ type, uint8_t* buf, const void* header_data,
    size_t header_size, const void* data, size_t data_size
)
{
    TRACE_ENTER(__func__);

    size_t size =
        prepare_header_(type, buf, header_data, header_size, data_size);

    ::memcpy(buf + size, data, data_size);

    size += data_size;
    if (serial_sink_.write(buf, size) != size) {
        LOG_ERR(<< "serial write blocked");
    }

    TRACE_EXIT(__func__);
}

static void
redirect_mqtt_(serial_header_* header, uint8_t* buf, size_t size)
{
    TRACE_ENTER(__func__);

    static char topic_buf[20] = {0};

    /* Topic recieved in packet is not null-terminated */
    ::memcpy(topic_buf, buf, header->dst_size);
    size_t crc_size = insert_crc_(buf + header->dst_size, header->size);
    size_t tot_size = crc_size + header->size;

    if (!io::mqtt_client.publish(topic_buf, buf + header->dst_size, tot_size)) {
        LOG_ERR(<< "unable to publish");
    }

    TRACE_EXIT(__func__);
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
    TRACE_ENTER(__func__);

    TRACE_EXIT(__func__);
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
    TRACE_ENTER(__func__);
    redirect_serial_(PACKET_MQTT, buf_, topic, ::strlen(topic), data, sz);
    TRACE_EXIT(__func__);
}

static void
reconnect(PubSubClient& client)
{
    while (!client.connected()) {
        if (!client.connect("router")) {
            LOG_INFO(
                << "connecting to mqtt (" << MQTT_HOST << ":"
                << String(MQTT_PORT) << ")"
            );
            delay(500);
            continue;
        }
    }

    client.subscribe("/device/test");
}

void
setup()
{
    TRACE_ENTER(__func__);

    WiFi.begin(SSID_NAME, SSID_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        LOG_INFO(<< "waiting for wifi connection");
        delay(500);
    }

    for (size_t i = 0; i < sizeof(esp_peers_) / sizeof(esp_peers_[0]); i++) {
        ::esp_err_t status = ::esp_now_add_peer(&esp_peers_[i]);

        if (status != ESP_OK) {
            LOG_ERR(<< "unable to add peer");
        }
    }

    if (::esp_now_register_recv_cb(esp_callback_) != ESP_OK) {
        LOG_ERR(<< "unable to register esp callback");
    }

    io::mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
    io::mqtt_client.setCallback(mqtt_callback_);
    reconnect(io::mqtt_client);

    TRACE_EXIT(__func__);
}

void
loop()
{
    static uint8_t buf[256];
    memset(buf, 1, 255);

    struct __attribute__((packed)) {
        uint8_t x;
        uint16_t y;
        int16_t z;
        int32_t u;
        float w;
    } test_data = {
        .x = 1,
        .y = 2,
        .z = -3,
        .u = -4,
        .w = 3.14,
    };

    size_t header_sz =
        prepare_header_(PACKET_MQTT, buf, "/test", 5, sizeof(test_data));
    memcpy(buf + header_sz, &test_data, sizeof(test_data));

    size_t bread =
        sizeof(serial_header_) + 5 +
        sizeof(test_data); // serial_sink_.read(buf, sizeof(serial_header_));

    if (bread != 0) {
        serial_header_* header = reinterpret_cast<serial_header_*>(buf);
        // bread +=
        //     serial_sink_.read(buf + bread, header->dst_size + header->size);

        redirect_network_(header, buf + sizeof(*header), bread);
    }

    io::mqtt_client.loop();
    delay(3000);
}