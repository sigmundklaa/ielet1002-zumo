/*
Implements a memory efficient, low footprint logging library.
*/

#ifndef LOGGING_LOG_HH__
#define LOGGING_LOG_HH__

#include <Arduino.h>
#include <io/io.hh>

namespace logging
{

#ifndef LOG_USE_BUF
#define LOG_USE_BUF 0
#endif

/* While we would ideally not want to send directly to the sink each time,
 * devices with RAM constraints may need to do that as using a buffer would
 * consume too much RAM. */
#if LOG_USE_BUF
#define LOG_BUF_SIZE_ (128)
#else
#define LOG_BUF_SIZE_ (0)
#endif

inline const char* level_to_str_(unsigned int level);

class logger_
{
  protected:
    const char* m_name;
    io::sink& m_sink;

    uint8_t m_buf[LOG_BUF_SIZE_];
    size_t m_written;

    size_t
    send_buf_()
    {
        size_t sent = m_sink.write(m_buf, m_written);
        m_written -= sent;

        return sent;
    }

    /**
     * @brief
     *
     * @param data Data to write
     * @param size Length of data (not including null-terminator)
     * @return size_t
     */
    size_t
    write_(const char* data, size_t size)
    {
        if (!LOG_USE_BUF) {
            return m_sink.write(data, size);
        }

        if (size >= sizeof(m_buf)) {
            // TODO: error handle
            return 0;
        }

        if (m_written + size >= sizeof(m_buf)) {
            send_buf_();
        }

        memcpy(m_buf + m_written, data, size);
        m_written += size;

        return size;
    }

    logger_&
    stream_write_(const char* data, size_t size)
    {
        if (write_(data, size) != size) {
            // TODO: error handle
        }

        return *this;
    }

  public:
    logger_(const char* name, io::sink& sink)
        : m_name(name), m_sink(sink), m_written(0)
    {
        memset(m_buf, 0, sizeof(m_buf));
    }

    void
    save()
    {
        if (!LOG_USE_BUF) {
            return;
        }

        send_buf_();
    }

    logger_&
    operator<<(const char* str)
    {
        return stream_write_(str, ::strlen(str));
    }

    logger_&
    operator<<(const String& str)
    {
        return stream_write_(str.c_str(), str.length());
    }
};

#define LOG_LEVEL_TRACE (0)
#define LOG_LEVEL_DEBUG (1)
#define LOG_LEVEL_INFO (2)
#define LOG_LEVEL_WARN (3)
#define LOG_LEVEL_ERR (4)
#define LOG_LEVEL_DISABLED (5)

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
#if LOG_LEVEL < LOG_LEVEL_DISABLED
#define LOG_REGISTER(sink)                                                     \
    static logging::logger_ /*arch_section__("log_instances")*/                \
        __attribute__((used)) LOG_INSTANCE_(LOG_STR_(LOG_MODULE), sink)
#else
#define LOG_REGISTER(sink)
#endif

#ifndef LOG_SAVE_ON_WRITE
#define LOG_SAVE_ON_WRITE (0)
#endif

#define LOG_HEAD__(level, x)                                                   \
    do {                                                                       \
        (LOG_INSTANCE_ << "[" << logging::level_to_str_(level))                \
            << "]: " x << "\n";                                                \
                                                                               \
        if (LOG_SAVE_ON_WRITE) {                                               \
            LOG_SAVE();                                                        \
        }                                                                      \
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

#if LOG_LEVEL < LOG_LEVEL_DISABLED
#define LOG_SAVE()                                                             \
    do {                                                                       \
        (LOG_INSTANCE_).save();                                                \
    } while (0)
#else
#define LOG_SAVE()
#endif

}; // namespace logging

#endif // LOGGING_LOG_HH__