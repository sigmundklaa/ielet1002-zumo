
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
        } __attribute__((packed)) cmd;

        int16_t arg1, arg2;
    };

    packet_ buf_;
    uint8_t auto_enabled_;

  public:
    remote_();

    /**
     * @brief To be run on every iteration. Checks if any commands from the
     * remote controller has been recieved, and acts accordingly if it has.
     *
     */
    void on_tick();
};

extern remote_ remote;

/**
 * @brief To be run on every iteration. Checks if any commands from the
 * remote controller has been recieved, and acts accordingly if it has.
 *
 */
inline void
on_tick()
{
    remote.on_tick();
}

}; // namespace control

#endif // CONTROL_HH__