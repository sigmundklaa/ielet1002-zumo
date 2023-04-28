
#ifndef ZUMO_REPORT_HH__
#define ZUMO_REPORT_HH__

#include "common.hh"
#include "comms.hh"
#include <report/report.hh>

namespace report
{

struct __attribute__((packed)) zumo_report_ {
    uint32_t vel;
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