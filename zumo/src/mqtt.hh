
#ifndef MQTT_HH__
#define MQTT_HH__

#include <io/mqtt.hh>

#ifndef TRIPLES_
#define TRIPLES__(x, y, z) x##y##z
#define TRIPLES_(x, y, z) TRIPLES__(x, y, z)
#endif

static io::mqtt_init_guard
    TRIPLES_(mqtt_guard_, __LINE__, __)(IPAddress(0, 0, 0, 0), 0, );

#endif // MQTT_HH__