
#ifndef CONTROL_HH__
#define CONTROL_HH__

#include <stdint.h>

namespace control
{
class remote_
{
  protected:
    struct __attribute__((packed)) packet_ {
        enum command_ {
            CMD_SET_SPEED,
            CMD_AUTONOMY,
        } cmd;

        int16_t arg1, arg2;
    };

    packet_ buf_;
    uint8_t auto_enabled_;

  public:
    remote_();

    void on_tick();
};

extern remote_ remote;

inline void
on_tick()
{
    remote.on_tick();
}

}; // namespace control

#endif // CONTROL_HH__