
#include "report.hh"
#include <io/redirect.hh>
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
    static io::redirect::redirect_gateway gateway(
        common::serial_gateway_, io::redirect::NODE_MQTT_REPORT_1,
        common::redirect_buf
    );

    new (&mem) zumo_reporter_(gateway, REPORT_INTERVAL_US_);
}

size_t
zumo_reporter_::generate_()
{

    return sizeof(buf_);
}

}; // namespace report