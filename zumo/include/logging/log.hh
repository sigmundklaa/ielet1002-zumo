
#ifndef LOGGING_LOG_HH__
#define LOGGING_LOG_HH__

#include <Arduino.h>
#include <io/io.hh>

namespace logging
{

#define LOG_BUF_SIZE_ (128)

class logger_
{
  protected:
    const char* m_name;
    io::sink* m_sink;

    uint8_t m_buf[LOG_BUF_SIZE_];
    size_t m_written;

    size_t
    send_buf_()
    {
        size_t sent =
            m_sink->write(static_cast<const uint8_t*>(m_buf), m_written);
        m_written -= sent;

        return sent;
    }

    size_t
    write_(const uint8_t* data, size_t size)
    {
        if (size > sizeof(m_buf)) {
            // TODO: error handle
            return 0;
        }

        if (m_written + size > sizeof(m_buf)) {
            send_buf_();
        }

        memcpy(m_buf + m_written, data, size);

        return size;
    }

    logger_&
    stream_write_(const uint8_t* data, size_t size)
    {
        if (write_(data, size) != size) {
            // TODO: error handle
        }

        return *this;
    }

  public:
    logger_(const char* name, io::sink* sink)
        : m_name(name), m_sink(sink), m_written(0)
    {
        memset(m_buf, 0, sizeof(m_buf));
    }

    void
    save()
    {
        send_buf_();
    }

    logger_&
    operator<<(const char* str)
    {
        return stream_write_(
            reinterpret_cast<const uint8_t*>(str), strlen(str) + 1
        );
    }

    logger_&
    operator<<(const String& str)
    {
        return stream_write_(
            reinterpret_cast<const uint8_t*>(str.c_str()), str.length() + 1
        );
    }
};

#define LOG_LEVEL_TRACE (0)
#define LOG_LEVEL_DEBUG (1)
#define LOG_LEVEL_INFO (2)
#define LOG_LEVEL_WARN (3)
#define LOG_LEVEL_ERR (4)
#define LOG_LEVEL_DISABLED (5)

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_ERR
#endif

#define LOG_STR__(x) #x
#define LOG_STR_(x) LOG_STR__(x)

#define LOG_TRIPLES__(a, b, c) a##b##c
#define LOG_TRIPLES_(a, b, c) LOG_TRIPLES__(a, b, c)

#define LOG_INSTANCE_ LOG_TRIPLES_(log_instance_, LOG_MODULE, __)

/**
 * @brief Register a logging instance
 *
 * @param sink Sink to send logged messages to
 */
#if LOG_LEVEL <= LOG_LEVEL_DISABLED
#define LOG_REGISTER(sink)                                                     \
    static logging::logger_ __attribute__((section("log_instances"), used))    \
    LOG_INSTANCE_(LOG_STR_(LOG_MODULE), sink)
#else
#define LOG_REGISTER(sink)
#endif

/* Autogenerated by ld */
extern logging::logger_ __start_log_instances;
extern logging::logger_ __stop_log_instances;

#define LOG_HEAD__(level, x)                                                   \
    do {                                                                       \
        (LOG_INSTANCE_ << logging::level_to_str_(level)) x;                    \
    } while (0)

/**
 * @brief Log a trace message if log level is >= LOG_LEVEL_TRACE
 *
 * @param any Stream of data (must start with <<)
 */
#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(x) LOG_HEAD__(LOG_LEVEL_TRACE, x)
#else
#define LOG_TRACE(x) (void)0
#endif

/**
 * @brief Log a debug message if log level is >= LOG_LEVEL_DEBUG
 *
 * @param any Stream of data (must start with <<)
 */
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(x) LOG_HEAD__(LOG_LEVEL_DEBUG, x)
#else
#define LOG_DEBUG(x) (void)0
#endif

/**
 * @brief Log a info message if log level is >= LOG_LEVEL_INFO
 *
 * @param any Stream of data (must start with <<)
 */
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(x) LOG_HEAD__(LOG_LEVEL_INFO, x)
#else
#define LOG_INFO(x) (void)0
#endif

/**
 * @brief Log a warn message if log level is >= LOG_LEVEL_WARN
 *
 * @param any Stream of data (must start with <<)
 */
#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(x) LOG_HEAD__(LOG_LEVEL_WARN, x)
#else
#define LOG_WARN(x) (void)0
#endif

/**
 * @brief Log a err message if log level is >= LOG_LEVEL_ERR
 *
 * @param any Stream of data (must start with <<)
 */
#if LOG_LEVEL <= LOG_LEVEL_ERR
#define LOG_ERR(x) LOG_HEAD__(LOG_LEVEL_ERR, x)
#else
#define LOG_ERR(x) (void)0
#endif

inline const char*
level_to_str_(unsigned int level)
{
    static const char* levels[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERR",
    };

    if (level >= LOG_LEVEL_DISABLED) {
        return "UNKNOWN";
    };

    return levels[level];
};

inline void
save_all()
{
    logging::logger_* logger = &__start_log_instances;

    for (; logger < &__stop_log_instances; logger++) {
        logger->save();
    }
}

}; // namespace logging

#endif // LOGGING_LOG_HH__