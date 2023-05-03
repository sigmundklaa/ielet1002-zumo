
#ifndef ZUMO_REPORT_HH__
#define ZUMO_REPORT_HH__

#include "common.hh"
#include "comms.hh"
#include <report/report.hh>

namespace report
{

struct __attribute__((packed)) zumo_report_ {
    struct side_ {
        int16_t vel;
        int16_t last;
        int16_t max;
        int32_t dist;
        int32_t vel_sum;
        int32_t vel_n;
        uint64_t us_above_70p;
    } left, right;

    uint8_t batt_status;
    uint8_t batt_health;
};

class zumo_reporter_ : public reporter_<zumo_report_>
{
  protected:
    size_t generate_() override__;

  public:
    zumo_reporter_(io::gateway& gateway, uint32_t interval)
        : reporter_<zumo_report_>(gateway, interval)
    {
    }
};

void init_reporter_(zumo_reporter_& mem);

static zumo_reporter_& reporter = init_guarded(zumo_reporter_, init_reporter_);

inline void
on_tick()
{
    reporter.on_tick();
}

}; // namespace report

#endif // ZUMO_REPORT_HH__