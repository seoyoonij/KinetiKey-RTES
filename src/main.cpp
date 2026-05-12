#include "mbed.h"
#include <chrono>
#include "LSM6DSL.h"
#include "data_struct.h"
#include "fft.h"
#include "gesture.h"
#include "states.h"

using namespace std::chrono;

Timer t;                      // clock
LSM6DSL imu(PB_11, PB_10, t); // IMU: I2C config
InterruptIn user_button(PC_13);
DigitalOut green_led(LED1);  // LD1 green, PA5
DigitalOut green_led2(LED2); // LD2 green, PB14

// button state tracking
volatile bool buttonDown = false;
volatile bool buttonReleased = false;
volatile uint32_t buttonDownMs = 0;
volatile uint32_t buttonHoldMs = 0;

static const uint32_t LONG_PRESS_MS = 1000; // hold 1s for recording
static const uint32_t RESULT_STATE_MS = 3000;
static Gesture_t completedGesture;

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
    // green 1 for pass
    case STATE_PASS:
        green_led = 1;
        green_led2 = 0;
        break;

    // green 2 for fail
    case STATE_FAIL:
        green_led = 0;
        green_led2 = 1;
        break;

    // blink asynchronously for recording
    case STATE_RECORDING:
        green_led = blinkOn ? 0 : 1;
        green_led2 = blinkOn ? 1 : 0;
        break;

    // blink synchronously for unlocking
    case STATE_UNLOCKING:
        green_led = blinkOn ? 1 : 0;
        green_led2 = blinkOn ? 1 : 0;
        break;

    // off for idle
    case STATE_IDLE:
    default:
        green_led = 0;
        green_led2 = 0;
        break;
    }
}

// reads one IMU sample and passes it into the gesture capture FSM
static bool readGestureSample(Gesture_t *gesture)
{
    IMUReading reading;

    if (!imu.readIMU(reading))
    {
        return false;
    }

    // timestamp before gesture capture so dt can be calculated in gesture.cpp
    reading.timestamp_ms = (uint32_t)duration_cast<milliseconds>(t.elapsed_time()).count();
    return GestureCapture_Update(&reading, gesture);
}

// handles button release events from the ISR flags
static void handleButtonRelease()
{
    if (!buttonReleased)
    {
        return;
    }

    buttonReleased = false;

    if (States_GetState() != STATE_IDLE)
    {
        return;
    }

    GestureCapture_Reset();

    if (buttonHoldMs >= LONG_PRESS_MS)
    {
        // long press starts recording the 3-gesture key
        States_StartRecord();
        printf("RECORDING: perform 3 gestures\r\n");
    }
    else
    {
        // short press starts unlock attempt
        States_StartUnlock();
        printf("UNLOCKING: perform 3 gestures\r\n");
    }
}

// handles one completed gesture while recording or unlocking
static void handleGestureCapture()
{
    if (!readGestureSample(&completedGesture))
    {
        return;
    }

    GestureResult result = States_HandleGestureComplete(completedGesture);

    // print result for compare weights
    printf("GestureResult=%d idx=%u err=%.2f samples=%u duration=%lu ms\r\n",
           (int)result,
           States_GetGestureIndex(),
           States_GetLastError(),
           completedGesture.sample_count,
           (unsigned long)completedGesture.duration_ms);

    if (States_GetState() == STATE_RECORDING || States_GetState() == STATE_UNLOCKING)
    {
        // reset capture FSM for the next gesture in the 3-gesture sequence
        GestureCapture_Reset();
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

    if (!FFT_Init())
    {
        printf("FFT init failed\r\n");
        return -1;
    }

    States_Init();
    GestureSeparation_SetDefault();
    GestureCapture_Reset();

    // attach ISRs
    user_button.fall(&button_fell);
    user_button.rise(&button_rose);
    green_led = 0;
    green_led2 = 0;

    printf("System ready. Long press records, short press unlocks.\r\n");

    while (true)
    {
        updateLedsForState();
        handleButtonRelease();

        switch (States_GetState())
        {
        case STATE_RECORDING:
        case STATE_UNLOCKING:
            // record or check gestures depending on the current state
            handleGestureCapture();
            break;

        case STATE_PASS:
            // show PASS LED briefly, then return to idle
            printf("PASS\r\n");
            updateLedsForState();
            ThisThread::sleep_for(RESULT_STATE_MS);
            States_ResetToIdle();
            GestureCapture_Reset();
            break;

        case STATE_FAIL:
            // show FAIL LED briefly, then return to idle
            printf("FAIL\r\n");
            updateLedsForState();
            ThisThread::sleep_for(RESULT_STATE_MS);
            States_ResetToIdle();
            GestureCapture_Reset();
            break;

        case STATE_IDLE:
        default:
            // wait for button input
            break;
        }

        ThisThread::sleep_for(5ms);
    }
}
