#include "mbed.h"
#include <chrono> // c++ time library
#include "LSM6DSL.h"
#include "data_struct.h"
#include "gesture.h"
#include "states.h"
using namespace std::chrono;

Timer t;                      // clock
LSM6DSL imu(PB_11, PB_10, t); // IMU: I2C config
// InterruptIn imu_int1(PB_2);   // IMU: interrupt config
InterruptIn user_button(PC_13);
DigitalOut green_led(LED1); // LD1 green, PA5
DigitalOut red_led(PE_3);   // LD8 red, PE3

// button state tracking
volatile bool buttonDown = false;
volatile bool buttonReleased = false;
volatile uint32_t buttonDownMs = 0;
volatile uint32_t buttonHoldMs = 0;
static const uint32_t LONG_PRESS_MS = 1000; // hold 1s for recording, adjust as needed
static const uint32_t RESET_PRESS_MS = 5000; //hold 5s for factory reset, adjust as needed

// Teleplot config
BufferedSerial serial_port(USBTX, USBRX, 115200);
FileHandle *mbed::mbed_override_console(int)
{
    return &serial_port;
}

// button interrupt handlers
void button_fell()
{
    buttonDown = true;
    buttonDownMs = duration_cast<milliseconds>(t.elapsed_time()).count();
}

void button_rose()
{
    if (buttonDown)
    {
        uint32_t nowMs = duration_cast<milliseconds>(t.elapsed_time()).count();
        buttonHoldMs = nowMs - buttonDownMs;
        buttonReleased = true;
        buttonDown = false;
    }
}

// updates LEDs based on current state, called in main loop
static void updateLedsForState()
{
    uint32_t currentTime = duration_cast<milliseconds>(t.elapsed_time()).count();
    bool blinkOn = ((currentTime / 250) % 2) == 0; // blinks every 250ms

    switch (States_GetState())
    {
    // green for pass
    case STATE_PASS:
        green_led = 1;
        red_led = 0;
        break;

    // red for fail
    case STATE_FAIL:
        green_led = 0;
        red_led = 1;
        break;

    // blink asynchronously for recording
    case STATE_RECORDING:
        green_led = blinkOn ? 0 : 1;
        red_led = blinkOn ? 1 : 0;
        break;

    // blink synchronously for unlocking
    case STATE_UNLOCKING:
        green_led = blinkOn ? 1 : 0;
        red_led = blinkOn ? 1 : 0;
        break;

    // off for idle
    case STATE_IDLE:
    default:
        green_led = 0;
        red_led = 0;
        break;
    }
}

int main()
{
    t.start();
    if (!imu.init())
    {
        printf("IMU init failed\r\n");
        return -1;
    }

    States_Init();
    user_button.fall(&button_fell);
    user_button.rise(&button_rose);
    green_led = 0;
    red_led = 0;

    printf("System ready. State: IDLE\r\n");

    while (true)
    {
        updateLedsForState();
        if (buttonReleased)
        {
            buttonReleased = false;

            if (States_GetState() == STATE_IDLE)
            {
                if (buttonHoldMs >= RESET_PRESS_MS)
                {
                    States_ClearKey();
                    printf("Factory reset: key cleared\r\n");
                }
                else if (buttonHoldMs >= LONG_PRESS_MS)
                {
                    States_StartRecord();
                    printf("RECORDING\r\n");
                }
                else
                {
                    States_StartUnlock();
                    printf("UNLOCKING\r\n");
                }
            }
        }

        switch (States_GetState())
        {
        case STATE_IDLE:
            // wait for button events
            break;

        case STATE_RECORDING:
            // capture gesture
            // States_HandleGestureComplete(gesture);
            break;

        case STATE_UNLOCKING:
            // capture gesture
            // States_HandleGestureComplete(gesture);
            break;

        case STATE_PASS:
            printf("PASS\r\n");
            ThisThread::sleep_for(1000ms);
            States_ResetToIdle();
            break;

        case STATE_FAIL:
            printf("FAIL\r\n");
            ThisThread::sleep_for(1000ms);
            States_ResetToIdle();
            break;
        }

        ThisThread::sleep_for(5ms);
    }
}
