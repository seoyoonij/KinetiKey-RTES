#include "mbed.h"

// The built-in LED on the IoT Discovery Kit
DigitalOut led(LED1);

int main()
{
    while (1)
    {
        led = !led;            // Toggle LED
        thread_sleep_for(500); // Wait 500ms
    }
}