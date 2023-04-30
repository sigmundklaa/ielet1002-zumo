/* The controller serves as a hardware abstraction layer between the Zumo and
 * the source code */

#ifndef CONTROLLER_HH__
#define CONTROLLER_HH__

#include <Zumo32U4.h>
#include <stddef.h>
#include <stdint.h>

/* Hardware abstraction layer namespace */
namespace hal
{
class controller_
{
  public:
    /**
     * @brief The Zumo has two sides, left and right, which can work
     * independently of eachother. For instance, the left wheels may be going in
     * reverse while the right wheels are going forward, and we need to control
     * that.
     *
     */
    class side_
    {
      public:
        enum side_num { LEFT = 1, RIGHT };

        enum direction {
            DIR_BWARD = 0,
            DIR_FWARD,
        };

      protected:
        enum state_ {
            STATE_IDLE_,
            STATE_CHANGE_DIR_,
            STATE_RUNNING_,
            STATE_STOPPED_,
        };

        state_ cur_state_;

        direction direction_;
        side_num side_num_;

        uint64_t stop_time_us_;
        uint64_t start_time_us_;
        uint8_t speed_;

        void set_motor_speed_(uint8_t s);

        void transition_(state_ st);
        void start_();
        void stop_();

      public:
        side_(side_num sn);

        void run();

        void set_dir(direction dir);
        void set_speed(uint8_t speed);
        void set_speed_noabs(int16_t speed);
        void stop();
        void start();

        uint8_t running();
    };

    template <typename T> class button_
    {
      public:
        typedef void (*btn_press_fn)(void);

      protected:
        T zbutton_;
        uint8_t pressed_;
        uint64_t time_hold_us_;

        btn_press_fn press_3s_;
        btn_press_fn press_1s_;
        btn_press_fn press_instant_;

        void
        call_callback_(btn_press_fn fn)
        {
            if (fn == nullptr) {
                return;
            }

            fn();
        }

      public:
        void
        handle(uint64_t delta_us)
        {
            uint8_t current = zbutton_.isPressed();

            if (current) {
                if (!pressed_) {
                    pressed_ = 1;
                }

                time_hold_us_ += delta_us;
            } else if (pressed_) {
                /* Was pressed, but no longer pressed */
                pressed_ = 0;

                if (time_hold_us_ >= 3e6) {
                    call_callback_(press_3s_);
                } else if (time_hold_us_ >= 1e6) {
                    call_callback_(press_1s_);
                } else if (time_hold_us_ >= 10e3) {
                    /* 10ms is considered instant */
                    call_callback_(press_instant_);
                }

                time_hold_us_ = 0;
            }
        }

        void
        set_3s_callback(btn_press_fn fn)
        {
            press_3s_ = fn;
        }

        void
        set_1s_callback(btn_press_fn fn)
        {
            press_1s_ = fn;
        }

        void
        set_0s_callback(btn_press_fn fn)
        {
            press_instant_ = fn;
        }
    };

  protected:
    uint64_t total_run_time_us_;
    uint64_t last_read_us_;
    uint8_t inited_;

    struct sensor_readings_ {
        int16_t accel[3];   /* Acceleration X,Y,Z */
        int16_t encoder[2]; /* Readings from left and right encoders,
                               respectively */
        unsigned int lines[5];
        int position;
    } readings_;

    /**
     * @brief Maintains a struct of the readings of the sensors. This is done to
     * prevent duplicate readings, and instead handle readings from one central
     * place. That way if two components rely on readings
     *
     */
    void read_sensors_();
    void init_();

  public:
    side_ left;
    side_ right;

    button_<Zumo32U4ButtonC> button_c;
    button_<Zumo32U4ButtonB> button_b;

    controller_();

    /**
     * @brief To be run on every loop iteration
     *
     */
    void run();

    void set_speeds(int16_t l, int16_t r);

    void stop();
    void start();

    /**
     * @brief Get the latest measurements from the encoder
     *
     * @return int16_t* int16_t[2] of left, right encoder values
     */
    int16_t*
    encoder_data()
    {
        return readings_.encoder;
    }

    unsigned int*
    lines_data()
    {
        return readings_.lines;
    }

    int
    position()
    {
        return readings_.position;
    }

    void calibrate();
};
extern controller_ controller;

inline void
on_tick()
{
    controller.run();
}

}; // namespace hal

#endif // CONTROLLER_HH__