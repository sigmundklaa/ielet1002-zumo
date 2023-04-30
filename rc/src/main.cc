
#include <WiFi.h>

#include <Arduino.h>
#include <PubSubClient.h>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <logging/log.hh>

#define PIN_A_X_ (26)
#define PIN_A_Y_ (27)

static PubSubClient client_;
static io::mqtt_gateway mqtt_gw(&client_, "/device/control/1", nullptr);
static io::serial_gateway<HardwareSerial> serial_gw(Serial, 9600);

struct __attribute__((packed)) cmd_packet {
    uint8_t cmd;
    int16_t arg1;
    int16_t arg2;
};

static cmd_packet packet;

#define LOG_MODULE rc
LOG_REGISTER(serial_gw);

static void
reconnect()
{
    if (client_.connected()) {
        return;
    }

    while (!client_.connected()) {
        if (!client_.connect("rc")) {
            LOG_INFO(
                << "connecting to mqtt (" << MQTT_HOST << ":"
                << String(MQTT_PORT) << ")"
            );
            delay(500);
        }
    }
}

static int16_t
calc_axis_(int16_t in)
{
    in = in - 512;

    return abs(in) > 10 ? in : 0;
}

static int16_t
clamp_speed_(int16_t n)
{
    if (n > 255) {
        return 255;
    } else if (n < -255) {
        return -255;
    }

    return n;
}

static void
send_(uint8_t cmd, int16_t arg1, int16_t arg2)
{
    packet = (cmd_packet){
        .cmd = cmd,
        .arg1 = arg1,
        .arg2 = arg2,
    };

    mqtt_gw.write(&packet, sizeof(packet));
}

static int16_t
map_speed_(int16_t n)
{
    int16_t calc = map(abs(n), 0, 512, 0, 255);

    if (n < 0) {
        calc = -calc;
    }

    return calc;
}

void
setup()
{
    // pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);

    WiFi.begin(SSID_NAME, SSID_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        LOG_INFO(<< "waiting for wifi");
        delay(500);
    }

    reconnect();
    send_(1, 0, 0);
}

void
loop()
{
    reconnect();

    int16_t readx = calc_axis_(analogRead(PIN_A_X_)),
            ready = calc_axis_(analogRead(PIN_A_Y_));

    int16_t offset = map_speed_(ready);
    int16_t base_speed = map_speed_(readx);
    int16_t speed_l = base_speed, speed_r = base_speed;

    speed_l = clamp_speed_(speed_l + offset);
    speed_r = clamp_speed_(speed_r - offset);

    send_(0, speed_l, speed_r);

    LOG_INFO(<< "x: " << String(speed_l) << ", y: " << String(speed_r));

    static uint8_t led;
    // digitalWrite(LED_BUILTIN, led = !led);

    // offset = 0-512. subtract left, added right

    delay(300);
}