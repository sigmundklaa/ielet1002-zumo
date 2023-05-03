
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

#define RX_PIN_ (16)
#define TX_PIN_ (17)

static io::serial_gateway<HardwareSerial>
    serial_gateway_(Serial1, 115200, SERIAL_8N1, RX_PIN_, TX_PIN_);

#if defined(REMOTE_LOG) && REMOTE_LOG
static io::mqtt_gateway log_gateway_(&io::mqtt_client, "/log/router", nullptr);
#else
static io::serial_gateway<HardwareSerial> log_gateway_(Serial, 9600);
#endif

#define LOG_MODULE router
LOG_REGISTER(log_gateway_);

#define BUF_SIZE_ (256)
static uint8_t buf_[BUF_SIZE_];

#ifndef ROUTER_USE_CRC
#define ROUTER_USE_CRC (0)
#endif

static struct mqtt_info {
    io::redirect::node_type node;
    const char *pub, *sub;
} mqtt_topics_[] = {
    {io::redirect::NODE_MQTT_STORE_1, "/redmw/sync/1", "/device/sync/1"},
    {io::redirect::NODE_MQTT_REPORT_1, "/redmw/report/1", nullptr},
    {io::redirect::NODE_MQTT_INIT_1, "/redmw/init/1", nullptr},
    {io::redirect::NODE_MQTT_CONTROL_1, nullptr, "/device/control/1"},
    {io::redirect::NODE_MQTT_CHARGE_1, "/redmw/charge/1", "/device/charge/1"},
    {io::redirect::NODE_MQTT_TRASH_1, "/redmw/trash/1", "/device/trash"},
};

static struct esp_info {
    io::redirect::node_type node;
    ::esp_now_peer_info_t peer;
} esp_peers_[] = {};

/**
 * @brief Insert a CRC-32 checksum at the front of the buffer @p buf. If crc is
 * disabled then nothing is done, and 0 is returned.
 *
 * @param buf
 * @param size
 * @return size_t Size of checksum
 */
static size_t
insert_crc_(uint8_t* buf, size_t size)
{
    if (!ROUTER_USE_CRC) {
        return 0;
    }

    uint32_t crc = utils::crc32(buf, size);
    ::memmove(buf + sizeof(crc), buf, size);
    ::memcpy(buf, &crc, sizeof(crc));

    return sizeof(crc);
}

/**
 * @brief Redirect data to the device connected to the serial gateway
 *
 * @param node
 * @param buf
 * @param data
 * @param data_size
 */
static void
redirect_serial_(
    io::redirect::node_type node, uint8_t* buf, const void* data,
    size_t data_size
)
{
    TRACE_ENTER(__func__);

    size_t size = io::redirect::prepare_header(node, buf, data_size);

    ::memcpy(buf + size, data, data_size);

    size += data_size;
    if (serial_gateway_.write(buf, size) != size) {
        LOG_ERR(<< "serial write blocked");
    }

    TRACE_EXIT(__func__);
}

/**
 * @brief Redirect data to the MQTT topic at @p topic
 *
 * @param header
 * @param topic
 * @param buf
 * @param size
 */
static void
redirect_mqtt_(
    io::redirect::header* header, const char* topic, uint8_t* buf, size_t size
)
{
    TRACE_ENTER(__func__);

    size_t crc_size = insert_crc_(buf, header->size);
    size_t tot_size = crc_size + header->size;

    if (!io::mqtt_client.publish(topic, buf, tot_size)) {
        LOG_ERR(<< "unable to publish");
    }

    TRACE_EXIT(__func__);
}

/**
 * @brief Redirect data to the ESP with mac address @p peer_addr
 *
 * @param header
 * @param peer_addr
 * @param buf
 * @param size
 */
static void
redirect_espnow_(
    io::redirect::header* header, uint8_t peer_addr[6], uint8_t* buf,
    size_t size
)
{
    TRACE_ENTER(__func__);

    TRACE_EXIT(__func__);
}

/**
 * @brief Redirect data to the appropriate network protocol according to the
 * arguments specified in @p header
 *
 * @param header
 * @param buf
 * @param size
 */
static void
redirect_network_(io::redirect::header* header, uint8_t* buf, size_t size)
{
    TRACE_ENTER(__func__);
    LOG_DEBUG(<< "node: " << String(header->node));

    switch (header->node) {
    case io::redirect::NODE_MQTT_REPORT_1:
    case io::redirect::NODE_MQTT_CHARGE_1:
    case io::redirect::NODE_MQTT_CONTROL_1:
    case io::redirect::NODE_MQTT_INIT_1:
    case io::redirect::NODE_MQTT_TRASH_1:
    case io::redirect::NODE_MQTT_STORE_1: {
        /* Send to appropriate mqtt node */
        for (size_t i = 0; i < UTILS_ARR_LEN(mqtt_topics_); i++) {
            LOG_DEBUG(<< "checking mqtt index " << String(i));
            mqtt_info& inf = mqtt_topics_[i];

            if (inf.node == header->node) {
                if (inf.pub != nullptr) {
                    redirect_mqtt_(header, inf.pub, buf, size);
                }
                break;
            }
        }
        break;
    }
#if 0
    case io::redirect::NODE_ESP_CHARGE: {
        for (size_t i = 0; i < UTILS_ARR_LEN(mqtt_topics_); i++) {
            if (esp_peers_[i].node == header->node) {
                redirect_espnow_(
                    header, esp_peers_[i].peer.peer_addr, buf, size
                );

                break;
            }
        }
        break;
    }
#endif
    }

    TRACE_EXIT(__func__);
}

static void
esp_callback_(const uint8_t* mac_addr, const uint8_t* data, int recv_sz)
{
    for (size_t i = 0; i < UTILS_ARR_LEN(esp_peers_); i++) {
        if (memcmp(mac_addr, esp_peers_[i].peer.peer_addr, 6) == 0) {
            redirect_serial_(esp_peers_[i].node, buf_, data, recv_sz);
            break;
        }
    }
}

static void
mqtt_callback_(char* topic, uint8_t* data, unsigned int sz)
{
    TRACE_ENTER(__func__);

    for (size_t i = 0; i < UTILS_ARR_LEN(mqtt_topics_); i++) {
        if (mqtt_topics_[i].sub == nullptr) {
            continue;
        }

        if (strcmp(mqtt_topics_[i].sub, topic) == 0) {
            redirect_serial_(mqtt_topics_[i].node, buf_, data, sz);
            break;
        }
    }

    TRACE_EXIT(__func__);
}

static void
reconnect(PubSubClient& client)
{
    if (client.connected()) {
        return;
    }

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

    for (size_t i = 0; i < UTILS_ARR_LEN(mqtt_topics_); i++) {
        mqtt_info& inf = mqtt_topics_[i];

        if (inf.sub != nullptr) {
            LOG_INFO(<< "subscribing to topic " << inf.sub);

            client.subscribe(inf.sub);
        }
    }
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

    for (size_t i = 0; i < UTILS_ARR_LEN(esp_peers_); i++) {
        ::esp_err_t status = ::esp_now_add_peer(&esp_peers_[i].peer);

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
    static uint8_t buf[256] = {0};

    reconnect(io::mqtt_client);

    size_t bread = serial_gateway_.read(buf, sizeof(io::redirect::header));

    if (bread != 0) {

        io::redirect::header* header =
            reinterpret_cast<io::redirect::header*>(buf);

        LOG_DEBUG(
            << "reading from serial (node " << String(header->node) << ", size "
            << String(header->size) << ", bread " << String(bread) << ")"
        );
        bread += serial_gateway_.read(buf + bread, header->size);

        redirect_network_(header, buf + sizeof(*header), bread);
    }

    io::mqtt_client.loop();
    delay(100);
}