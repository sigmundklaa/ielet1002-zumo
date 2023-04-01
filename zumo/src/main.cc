
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