/* For non-Arduino platforms (testing locally e.g.) */

#include <Wire.h>

#ifdef __unix__
extern void setup(void);
extern void loop(void);

int
main(void)
{
    setup();

    for (;;) {
        loop();
    }
}
#endif