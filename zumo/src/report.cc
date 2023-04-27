
#include "report.hh"
#include "comms.hh"
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
    new (&mem) zumo_reporter_(comms::report_gw, REPORT_INTERVAL_US_);
}

size_t
zumo_reporter_::generate_()
{

    return sizeof(buf_);
}

}; // namespace report