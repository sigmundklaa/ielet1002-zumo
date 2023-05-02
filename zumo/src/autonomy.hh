
#ifndef AUTONOMY_HH__
#define AUTONOMY_HH__

#include <stdint.h>

namespace autonomy
{
void on_init();
void on_tick();
void toggle(uint8_t mode);
}; // namespace autonomy

#endif // AUTONOMY_HH__
