
#include <Arduino.h>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <stdio.h>

#include <io/mqtt.impl.hh>
static io::serial_sink sink;

#include <logging/log.hh>

#define LOG_MODULE main
LOG_REGISTER(&sink);

void
setup()
{
    // put your setup code here, to run once:
}

void
loop()
{
    // put your main code here, to run repeatedly:
    // logging::save_all();
    static char buf[512];
    snprintf(buf, 512, "hello world!\n");

    sink.write(buf, 15);
    // sink.read(buf, 512);

    // sink.write(buf, 512);

    LOG_ERR(<< "Error !");
    LOG_SAVE();

    for (;;)
        ;
}
