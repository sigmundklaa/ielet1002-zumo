
#include <Arduino.h>
#include <WiFi.h>
#include <io/espnow.hh>
#include <io/mqtt.hh>
#include <io/redirect.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <string.h>
#include <utils/crc32.hh>
#include <utils/trace.hh>
#include <wifi.hh>

#define RX_PIN_ (26)
#define TX_PIN_ (22)

static io::serial_gateway<HardwareSerial>
    serial_gateway_(Serial2, 115200, RX_PIN_, TX_PIN_);

static io::serial_gateway<HardwareSerial> log_gateway_(Serial, 9600);

#define LOG_MODULE router
LOG_REGISTER(log_gateway_);

#define BUF_SIZE_ (512)

static uint8_t buf_[256];

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
    io::redirect::header_type type, uint8_t* buf, const void* header_data,
    size_t header_size, const void* data, size_t data_size
)
{
    TRACE_ENTER(__func__);

    size_t size = io::redirect::prepare_header(
        type, buf, header_data, header_size, data_size
    );

    ::memcpy(buf + size, data, data_size);

    size += data_size;
    if (serial_gateway_.write(buf, size) != size) {
        LOG_ERR(<< "serial write blocked");
    }

    TRACE_EXIT(__func__);
}

static void
redirect_mqtt_(io::redirect::header* header, uint8_t* buf, size_t size)
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
redirect_espnow_(io::redirect::header* header, uint8_t* buf, size_t size)
{
    TRACE_ENTER(__func__);

    TRACE_EXIT(__func__);
}

static void
redirect_network_(io::redirect::header* header, uint8_t* buf, size_t size)
{
    switch (header->type) {
    case io::redirect::PACKET_MQTT:
        redirect_mqtt_(header, buf, size);
        break;
    case io::redirect::PACKET_ESPNOW:
        redirect_espnow_(header, buf, size);
        break;
    case io::redirect::PACKET_HOST:
        /* Not meant to be redirected anywhere */
        break;
    }
}

static void
esp_callback_(const uint8_t* mac_addr, const uint8_t* data, int recv_sz)
{
    redirect_serial_(
        io::redirect::PACKET_ESPNOW, buf_, mac_addr, ESP_NOW_ETH_ALEN, data,
        recv_sz
    );
}

static void
mqtt_callback_(char* topic, uint8_t* data, unsigned int sz)
{
    TRACE_ENTER(__func__);
    redirect_serial_(
        io::redirect::PACKET_MQTT, buf_, topic, ::strlen(topic), data, sz
    );
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

    size_t header_sz = io::redirect::prepare_header(
        io::redirect::PACKET_MQTT, buf, "/test", 5, sizeof(test_data)
    );
    memcpy(buf + header_sz, &test_data, sizeof(test_data));

    size_t bread =
        sizeof(io::redirect::header) + 5 +
        sizeof(test_data
        ); // serial_gateway_.read(buf, sizeof(io::redirect::header));

    if (bread != 0) {
        io::redirect::header* header =
            reinterpret_cast<io::redirect::header*>(buf);
        // bread +=
        //     serial_gateway_.read(buf + bread, header->dst_size +
        //     header->size);

        redirect_network_(header, buf + sizeof(*header), bread);
    }

    io::mqtt_client.loop();
    delay(3000);
}