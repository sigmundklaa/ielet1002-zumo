/* The reporting service is reponsible for sending periodic status updates to
 * the remote, with information regarding the device health and other desirable
 * information*/

#ifndef REPORT_HH__
#define REPORT_HH__

#include <Arduino.h>
#include <io/io.hh>
#include <utils/init.hh>

namespace report
{

template <typename T> class reporter_
{
  protected:
    io::gateway& gateway_;
    uint32_t interval_us_;
    uint64_t last_write_us_;

    T buf_;

    /**
     * @brief Method responsible for generating reports. Should return the size
     * of the generated report
     *
     * @return size_t
     */
    virtual size_t generate_() = 0;

  public:
    reporter_(io::gateway& gateway, uint32_t interval)
        : gateway_(gateway), interval_us_(interval)
    {
    }

    void
    on_tick()
    {
        uint64_t tmp = micros();

        if (tmp - last_write_us_ < interval_us_) {
            return;
        }

        gateway_.write(&buf_, generate_());
        last_write_us_ = tmp;
    }
};
}; // namespace report

#endif // REPORT_HH__