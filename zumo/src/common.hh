
#ifndef COMMON_HH__
#define COMMON_HH__

#if !defined(MCU__) || !MCU__
#include <io/std.hh>
#else
#include <io/serial.hh>
#endif

#include <io/redirect.hh>
#include <string.h>
#include <utils/crc32.hh>
#include <utils/init.hh>

namespace common
{

#if !defined(MCU__) || !MCU__
static io::std_gateway& log_gateway =
    init_guarded(io::std_gateway, utils::init_empty);
#else
static inline void
init_log_gateway_(io::serial_gateway<Serial_>& mem)
{
    new (&mem) io::serial_gateway<Serial_>(Serial, 9600);
}

/**
 * @brief Common gateway for logging
 *
 */
static io::serial_gateway<Serial_>& log_gateway =
    init_guarded(io::serial_gateway<Serial_>, init_log_gateway_);
#endif

static inline void
init_serial_gateway_(io::serial_gateway<HardwareSerial>& mem)
{
    new (&mem) io::serial_gateway<HardwareSerial>(Serial1, 115200);
}

static io::serial_gateway<HardwareSerial>& serial_gateway_ =
    init_guarded(io::serial_gateway<HardwareSerial>, init_serial_gateway_);

/**
 * @brief Manages storage. Saves and retrieves data from the connected
 * gateway when necessary
 *
 * @tparam T Struct for store data
 */
template <typename T> class store
{
  protected:
    io::gateway* gateway_;
    uint32_t crc_last_;
    uint64_t last_save_us_;

  public:
    static T buf;

    T data;

    /**
     * @brief Construct a new store object
     *
     * @param gateway gateway to save/retreive data from
     * @param data_init Initial data
     */
    inline store(io::gateway* gateway, const T& data_init) : gateway_(gateway)
    {
        ::memcpy(&this->data, &data_init, sizeof(T));
    }

    inline size_t
    sync()
    {
        size_t bread = gateway_->read(&store<T>::buf, sizeof(store<T>::buf));

        if (bread != sizeof(store<T>::buf)) {
            return 0;
        }

        ::memcpy(&this->data, &store<T>::buf, bread);

        return bread;
    }

    /**
     * @brief Write the store to a gateway. Returns size written on success, 0
     * otherwise
     *
     * @return int
     */
    inline int
    save()
    {
        uint64_t tmp = micros();
        if (tmp - last_save_us_ < 1e6) {
            return 0;
        }
        last_save_us_ = tmp;

        uint32_t checksum =
            utils::crc32((const uint8_t*)&this->data, sizeof(T));
        if (crc_last_ && checksum == crc_last_) {
            return 0;
        }

        size_t written = gateway_->write(&this->data, sizeof(T));

        if (written != sizeof(T)) {
            return written;
        }

        crc_last_ = checksum;

        /* Read the updated contents into a buffer first, and then if there is
         * no error we can safely update the real data field. */
        return this->sync();
    }
};

/**
 * @brief Data that is saved on connected remote
 *
 */
struct __attribute__((packed)) remote_data {
    uint32_t bank_currency;
};

/**
 * @brief Data that is saved in local non-volatile storage
 *
 */
struct __attribute__((packed)) local_data {
    uint8_t batt_status;
    uint8_t batt_health;
    uint8_t batt_n_charges;
    uint8_t batt_n_drained;
};

extern store<remote_data> remote_store;
extern store<local_data> local_store;

void on_tick();

}; // namespace common

#endif // COMMON_HH__