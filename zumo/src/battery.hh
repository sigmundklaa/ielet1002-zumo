
#ifndef BATTERY_HH__
#define BATTERY_HH__

#include <logging/log.hh>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace swbat
{

class battery__
{
  protected:
    enum state {
        STATE_INACTIVE_ = 0,
        STATE_DRAINING_,
        STATE_CHARGING_,
    } state_;

    enum health_state {
        HEALTH_NORM_ = 0,
        HEALTH_LOW_,
        HEALTH_CRIT_,
    } health_state_;

    uint64_t time_active_us_;
    uint64_t time_last_us_;
    uint64_t time_last_drain_us_;

    /**
     * @brief Transition the battery into @p new_state
     *
     * @param new_state
     */
    void transition_(enum state new_state);

    /**
     * @brief Calculate the how much is "draining" from the battery health
     * currently. This will be subtracted from battery health
     *
     * @return uint8_t
     */
    uint8_t calc_drain_health_();

    /**
     * @brief Calculate how much the battery is "draining" currently. This will
     * be subtracted from the battery level
     *
     * @return uint8_t
     */
    uint8_t calc_drain_batt_();

  public:
    void on_tick();
    void charge();

    uint8_t
    need_service()
    {
        return health_state_ == HEALTH_LOW_;
    }

    uint8_t
    need_replacement()
    {
        return health_state_ == HEALTH_CRIT_;
    }

    uint8_t need_charge();

    void service();
    void replace();

    uint8_t
    drainable()
    {
        return state_ == STATE_INACTIVE_;
    }

    uint8_t
    charging()
    {
        return state_ == STATE_CHARGING_;
    }

    void toggle_drain(uint8_t draining);

    uint64_t
    time_active_us()
    {
        return time_active_us_;
    }
};
extern battery__ battery;

void on_init();

}; // namespace swbat

#endif // BATTERY_H__