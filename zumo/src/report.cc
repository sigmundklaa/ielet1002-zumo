
#include "report.hh"
#include <io/mqtt.hh>
#include <utils/compile.hh>
#include <utils/init.hh>
#include <utils/macros.hh>
#include <utils/new.hh>

namespace report
{

#define REPORT_INTERVAL_US_ (10e6)

void
init_reporter_(zumo_reporter_& mem)
{
    static io::mqtt_gateway gateway(
        &io::mqtt_client, IO_MQTT_PATH("/report/", IO_MQTT_NODE_ZUMO), nullptr
    );

    new (&mem) zumo_reporter_(gateway, REPORT_INTERVAL_US_);
}

size_t
zumo_reporter_::generate_()
{

    return sizeof(buf_);
}

}; // namespace report