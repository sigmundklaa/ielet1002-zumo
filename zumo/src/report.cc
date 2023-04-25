
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
#define REPORT_PATH_ "/report/1"
    static io::redirect::redirect_gateway gateway(
        common::serial_gateway_, io::redirect::PACKET_MQTT,
        (const uint8_t*)REPORT_PATH_, sizeof(REPORT_PATH_)
    );
#undef REPORT_PATH_

    new (&mem) zumo_reporter_(gateway, REPORT_INTERVAL_US_);
}

size_t
zumo_reporter_::generate_()
{

    return sizeof(buf_);
}

}; // namespace report