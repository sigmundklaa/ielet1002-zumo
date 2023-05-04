
#ifndef AUTONOMY_HH__
#define AUTONOMY_HH__

#include <stdint.h>

namespace autonomy
{
/**
 * @brief Called on initialization. Sets up required functionality for the
 * connected peripherals.
 *
 */
void on_init();

/**
 * @brief To be run on every iteration. Handles autonomous control of the Zumo
 *
 */
void on_tick();

/**
 * @brief Toggles autonomy, disables if @p mode is 0, enables otherwise.
 *
 * @param mode
 */
void toggle(uint8_t mode);
}; // namespace autonomy

#endif // AUTONOMY_HH__
