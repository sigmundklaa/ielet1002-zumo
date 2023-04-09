
#include "report.hh"
#include <io/mqtt.hh>
#include <utils/compile.hh>
#include <utils/init.hh>
#include <utils/macros.hh>
#include <utils/new.hh>

namespace report
{

void
init_reporter_(zumo_reporter_& mem)
{
    static io::mqtt_sink sink(
        &io::mqtt_client, IO_MQTT_PATH("/report/", IO_MQTT_NODE_ZUMO), nullptr
    );

    new (&mem) zumo_reporter_(sink, REPORT_INTERVAL_US_);
}

size_t
zumo_reporter_::generate_()
{
}

}; // namespace report