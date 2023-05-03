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

        /**
         * @brief Call the underlying set speed command for the Zumo
         *
         * @param s Speed in the range 0-255
         */
        void set_motor_speed_(uint8_t s);

        void transition_(state_ st);

        /**
         * @brief Called when the Zumo has entered the running state
         *
         */
        void start_();

        /**
         * @brief Called when the Zumo has entered the stopped state
         *
         */
        void stop_();

      public:
        side_(side_num sn);

        /**
         * @brief To be ran on every iteration. Handles things like sampling
         * data from sensors and outputting speed to the motors.
         *
         */
        void run();

        void set_dir(direction dir);

        /**
         * @brief Set the speed given in @p speed, in the range 0-255.
         * This will not affect the current direction.
         *
         * @param speed
         */
        void set_speed(uint8_t speed);

        /**
         * @brief Set the speed given in @p speed, in the range -255-255. This
         * will update the current direction if the sign of @p speed is
         * different than the expected sign for the current direction.
         *
         * @param speed
         */
        void set_speed_noabs(int16_t speed);

        /**
         * @brief Transition into the stopped state
         *
         */
        void stop();

        /**
         * @brief Transition into the running state
         *
         */
        void start();

        /**
         * @brief Returns 0 if not running, non-zero otherwise.
         *
         * @return uint8_t
         */
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

        /**
         * @brief Helper function to ensure @p fn is properly called.
         *
         * @param fn
         */
        void
        call_callback_(btn_press_fn fn)
        {
            if (fn == nullptr) {
                return;
            }

            fn();
        }

      public:
        /**
         * @brief Monitors the button and calls the set callback if the button
         * is pressed.
         *
         * @param delta_us Delta in microseconds since last call
         */
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

        /**
         * @brief Set the callback function to @p fn for when the button is
         * pressed atleast 3s
         *
         * @param fn
         */
        void
        set_3s_callback(btn_press_fn fn)
        {
            press_3s_ = fn;
        }

        /**
         * @brief Set the callback function to @p fn for when the button is
         * pressed atleast 1s
         *
         * @param fn
         */
        void
        set_1s_callback(btn_press_fn fn)
        {
            press_1s_ = fn;
        }

        /**
         * @brief Set the callback function to @p fn for when the button is
         * pressed atleast 10ms
         *
         * @param fn
         */
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

    /**
     * @brief Initialize the hardware components
     *
     */
    void init_();

  public:
    side_ left;
    side_ right;

    button_<Zumo32U4ButtonC> button_c;
    button_<Zumo32U4ButtonB> button_b;

    controller_();

    /**
     * @brief To be ran on every iteration. Handles things like sampling
     * data from sensors and outputting speed to the motors. Calls the @code run
     * @endcode method on both sides
     *
     */
    void run();

    /**
     * @brief Set the speed of the left side to @p l an the speed of the right
     * side to @p r. The speed is given in the range -255 - 255, where a
     * negative sign specifies the motors should be going in the backwards
     * direction
     *
     * @param l
     * @param r
     */
    void set_speeds(int16_t l, int16_t r);

    /**
     * @brief Transition both sides into the stopped state
     *
     */
    void stop();

    /**
     * @brief Transition both sides into the running state
     *
     */
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

    /**
     * @brief Returns the array to the contionusly updated line readings
     *
     * @return unsigned*
     */
    unsigned int*
    lines_data()
    {
        return readings_.lines;
    }

    /**
     * @brief Return the position from the last reading
     *
     * @return int
     */
    int
    position()
    {
        return readings_.position;
    }

    /**
     * @brief Calibrate the line sensors
     *
     */
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