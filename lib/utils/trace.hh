
#ifndef UTILS_TRACE_HH__
#define UTILS_TRACE_HH__

#include <logging/log.hh>

#define TRACE_ENTER(name) LOG_TRACE(<< "Enter " << name)
#define TRACE_EXIT(name) LOG_TRACE(<< "Exit " << name)

#endif // UTILS_TRACE_HH__