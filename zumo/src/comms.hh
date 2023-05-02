
#ifndef COMMS_HH__
#define COMMS_HH__

#include "common.hh"
#include <io/redirect.hh>
#include <utils/init.hh>
#include <utils/macros.hh>
#include <utils/new.hh>

namespace comms
{

extern uint8_t buf_[256];

#define COMMS_NODES__                                                          \
    X(report_gw, io::redirect::NODE_MQTT_REPORT_1)                             \
    X(store_gw, io::redirect::NODE_MQTT_STORE_1)                               \
    X(charge_gw, io::redirect::NODE_MQTT_CHARGE_1)                             \
    X(init_gw, io::redirect::NODE_MQTT_INIT_1)                                 \
    X(control_gw, io::redirect::NODE_MQTT_CONTROL_1)

template <io::redirect::node_type node> class dummy__
{
  public:
    static io::redirect::redirect_gateway gw;
};

template <io::redirect::node_type nt>
io::redirect::redirect_gateway
    dummy__<nt>::gw(common::serial_gateway_, nt, buf_);

#define X(varname, nodet)                                                      \
    static io::redirect::redirect_gateway& varname = dummy__<nodet>::gw;

COMMS_NODES__
#undef X

void on_tick();

}; // namespace comms

#endif // COMMS_HH__