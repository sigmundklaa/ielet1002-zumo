
#ifndef COMMON_HH__
#define COMMON_HH__

#include <io/serial.hh>
#include <string.h>

namespace common
{

static inline void
init_log_sink_(io::serial_sink& mem)
{
    new (&mem) io::serial_sink(Serial, 9600);
}

/**
 * @brief Common sink for logging
 *
 */
static io::serial_sink& log_sink =
    init_guarded(io::serial_sink, init_log_sink_);

/**
 * @brief Manages storage. Saves and retrieves data from the connected sink when
 * necessary
 *
 * @tparam T Struct for store data
 */
template <typename T> class store
{
  protected:
    io::sink* sink_;

  public:
    T data;

    /**
     * @brief Construct a new store object
     *
     * @param sink Sink to save/retreive data from
     * @param data_init Initial data, will only be used when unable to read from
     * the sink
     */
    inline store(io::sink* sink, const T& data_init) : sink_(sink)
    {
        size_t bread = sink_->read(&this->data, sizeof(T));

        if (bread != sizeof(T)) {
            ::memcpy(&this->data, &data_init, sizeof(T));
        }
    }

    /**
     * @brief Write the store to a sink. Returns 1 on success, 0 otherwise
     *
     * @return int
     */
    inline int
    save()
    {
        static T buf = {0};

        size_t written = sink_->write(&this->data, sizeof(T)) != sizeof(T);

        if (written != sizeof(T)) {
            return written;
        }

        /* Read the updated contents into a buffer first, and then if there is
         * no error we can safely update the real data field. */
        size_t read = sink_->read(&buf, sizeof(buf));

        if (read == sizeof(T)) {
            ::memcpy(&this->data, &buf, sizeof(T));
        }

        return read;
    }
};

/**
 * @brief Data that is saved on connected remote
 *
 */
struct __attribute__((packed)) remote_data {
    uint8_t batt_voltage;
};

/**
 * @brief Data that is saved in local non-volatile storage
 *
 */
struct __attribute__((packed)) local_data {
    uint8_t batt_health;
};

extern store<remote_data> remote_store;
extern store<local_data> local_store;

}; // namespace common

#endif // COMMON_HH__