
#ifndef ZUMO_REPORT_HH__
#define ZUMO_REPORT_HH__

#include <io/mqtt.hh>
#include <report/report.hh>

#define REPORT_INTERVAL_US_ (10e6)

namespace report
{

struct __attribute__((packed)) zumo_report_ {
};

class zumo_reporter_ : public reporter_<zumo_report_>
{
  protected:
    size_t generate_() override__;

  public:
    zumo_reporter_(io::sink& sink, uint32_t interval)
        : reporter_(sink, interval)
    {
    }
};

void init_reporter_(zumo_reporter_& mem);

static zumo_reporter_& reporter = init_guarded(zumo_reporter_, init_reporter_);
}; // namespace report

#endif // ZUMO_REPORT_HH__