
#include "report.hh"
#include "common.hh"
#include "comms.hh"
#include "housekeep.hh"
#include <io/redirect.hh>
#include <logging/log.hh>
#include <utils/compile.hh>
#include <utils/init.hh>
#include <utils/macros.hh>
#include <utils/new.hh>

#define LOG_MODULE reporter
LOG_REGISTER(common::log_gateway);

namespace report
{

#define REPORT_INTERVAL_US_ (10e6)

void
init_reporter_(zumo_reporter_& mem)
{
    new (&mem) zumo_reporter_(comms::report_gw, REPORT_INTERVAL_US_);
}

static void
gen_side_(zumo_report_::side_& side, hk::data_::vel& hk_side)
{
    side = (zumo_report_::side_){
        .vel = hk_side.velocity,
        .last = hk_side.last_count,
        .dist = hk_side.distance,
        .vel_sum = hk_side.vel_sum,
        .vel_n = hk_side.vel_n,
        .us_above_70p = hk_side.us_above_70p,
    };
}

size_t
zumo_reporter_::generate_()
{
    gen_side_(buf_.left, hk::data.vel_l);
    gen_side_(buf_.right, hk::data.vel_r);

    return sizeof(buf_);
}

}; // namespace report