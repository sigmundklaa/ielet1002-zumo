/* The controller serves as a hardware abstraction layer between the Zumo and
 * the source code */

#ifndef CONTROLLER_HH__
#define CONTROLLER_HH__

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
        void stop();
        void start();

        uint8_t running();
    };

  protected:
    uint64_t total_run_time_us_;
    uint64_t last_read_us_;

    struct sensor_readings_ {
        int16_t accel[3];   /* Acceleration X,Y,Z */
        int16_t encoder[2]; /* Readings from left and right encoders,
                               respectively */
    } readings_;

    /**
     * @brief Maintains a struct of the readings of the sensors. This is done to
     * prevent duplicate readings, and instead handle readings from one central
     * place. That way if two components rely on readings
     *
     */
    void read_sensors_();

  public:
    side_ left, right;

    controller_();

    /**
     * @brief To be run on every loop iteration
     *
     */
    void run();

    /**
     * @brief Get the latest measurements from the accelerometer
     *
     * @return int16_t* int16_t[3] of X,Y,Z accelerations
     */
    int16_t* accel_data();

    /**
     * @brief Get the latest measurements from the encoder
     *
     * @return int16_t* int16_t[2] of left, right encoder values
     */
    int16_t* encoder_data();
};
extern controller_ controller;

}; // namespace hal

#endif // CONTROLLER_HH__