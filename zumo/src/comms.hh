
#ifndef COMMS_HH__
#define COMMS_HH__

#include "common.hh"
#include <io/redirect.hh>
#include <utils/macros.hh>
#include <utils/new.hh>

namespace comms
{

extern uint8_t buf_[256];

/* The beauty of C macros at work! While this looks very dirty, it makes it easy
 * to add more gateways later. This ensures that the gateways are properly
 * created, intialized, and resolved. All that is needed to add a new gateway is
 * to add another X(gateway_name, gateway_node) */
#define COMMS_NODES__                                                          \
    X(report_gw, io::redirect::NODE_MQTT_REPORT_1)                             \
    X(store_gw, io::redirect::NODE_MQTT_STORE_1)                               \
    X(charge_gw, io::redirect::NODE_ESP_CHARGE)

#define X(varname, nodet)                                                      \
    struct UTILS_TRIPLES(init_dummy_, varname, __) {                           \
        io::redirect::redirect_gateway gw;                                     \
        /* The constructor will initialize the gw variable, allowing it to be  \
         * accessed later */                                                   \
        UTILS_TRIPLES(init_dummy_, varname, __)                                \
        (io::redirect::node_type n) : gw(common::serial_gateway_, n, buf_)     \
        {                                                                      \
        }                                                                      \
    };                                                                         \
    /* Function called exactly once by the init guard. This will initialize    \
     * the dummy, which then initializes the gateway */                        \
    inline void UTILS_TRIPLES(                                                 \
        init_, varname, __                                                     \
    )(UTILS_TRIPLES(init_dummy_, varname, __) & mem)                           \
    {                                                                          \
        new (&mem) UTILS_TRIPLES(init_dummy_, varname, __)(nodet);             \
    }                                                                          \
    static io::redirect::redirect_gateway& varname =                           \
        (init_guarded(                                                         \
             UTILS_TRIPLES(init_dummy_, varname, __),                          \
             UTILS_TRIPLES(init_, varname, __)                                 \
         ))                                                                    \
            .gw;
COMMS_NODES__
#undef X

void on_tick();

}; // namespace comms

#endif // COMMS_HH__