
#include <WiFi.h>

#include <Arduino.h>
#include <PubSubClient.h>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <logging/log.hh>

#define PIN_A_X_ (34)
#define PIN_A_Y_ (35)

#define ANALOG_RANGE_ (4095)

static io::mqtt_gateway mqtt_gw(&io::mqtt_client, "/device/control/1", nullptr);
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
    if (io::mqtt_client.connected()) {
        return;
    }

    LOG_INFO(
        << "connecting to mqtt (" << MQTT_HOST << ":" << String(MQTT_PORT)
        << ")"
    );

    while (!io::mqtt_client.connected()) {
        if (!io::mqtt_client.connect("rc")) {
            LOG_INFO(<< "waiting for mqtt");
            delay(500);
        }
    }
}

static int16_t
calc_axis_(int16_t in)
{
    in = in - (ANALOG_RANGE_ / 2);

    return abs(in) > 200 ? in : 0;
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
    int16_t calc = map(abs(n), 0, ANALOG_RANGE_ / 2, 0, 255);

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

    io::mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
    reconnect();
    send_(1, 0, 0);
}

#define N_SAMPLES_ (10)

void
loop()
{
    static int16_t l, r;
    static uint64_t last;
    reconnect();

    int16_t suml, sumr;

    for (int i = 0; i < N_SAMPLES_; i++) {
        int16_t readx = calc_axis_(analogRead(PIN_A_X_)),
                ready = calc_axis_(analogRead(PIN_A_Y_));

        int16_t offset = map_speed_(ready) / 2;
        int16_t base_speed = map_speed_(readx);

        suml += clamp_speed_(base_speed + offset);
        sumr += clamp_speed_(base_speed - offset);

        delay(10);
    }

    uint64_t tmp = micros();

    int16_t speed_l = suml / N_SAMPLES_, speed_r = sumr / N_SAMPLES_;
    if (tmp - last >= 10e3 &&
        (abs(l - speed_l) > 10 || abs(r - speed_r) > 10)) {
        l = speed_l;
        r = speed_r;
        last = tmp;
        send_(0, speed_l, speed_r);

        LOG_INFO(
            << "x: " << String(speed_l) << ", y: " << String(speed_r) << ", "
            //<< String(readx) << ", " << String(ready)
        );
    }
}