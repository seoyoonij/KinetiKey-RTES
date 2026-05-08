#include "mbed.h"
#include <chrono> // c++ time library
#include "LSM6DSL.h"
#include "data_struct.h"
#include "gesture.h"
#include "states.h"
#include "compare.h"
using namespace std::chrono;

Timer t;                      // clock
LSM6DSL imu(PB_11, PB_10, t); // IMU: I2C config
// InterruptIn imu_int1(PB_2);   // IMU: interrupt config
InterruptIn user_button(PC_13);
DigitalOut green_led(LED1);  // LD1 green, PA5
DigitalOut green_led2(LED2); // LD2 green, PB14

// button state tracking
volatile bool buttonDown = false;
volatile bool buttonReleased = false;
volatile uint32_t buttonDownMs = 0;
volatile uint32_t buttonHoldMs = 0;
static const uint32_t LONG_PRESS_MS = 1000;  // hold 1s for recording, adjust as needed
static const uint32_t RESET_PRESS_MS = 5000; // hold 5s for factory reset, adjust as needed
static const uint32_t RESULT_STATE_MS = 3000;

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

int main()
{
    t.start();
    if (!imu.init())
    {
        printf("IMU init failed\r\n");
        return -1;
    }

    States_Init();
    GestureSeparation_SetDefault();
    // TEMP: FORCE TEST MODE
    States_StartRecord();
    GestureCapture_Reset();
    printf("Gesture capture test started\r\n");
    uint32_t lastPrint = 0;
    GestureCaptureState prev = GestureCapture_GetState();

    // attach ISRs
    user_button.fall(&button_fell);
    user_button.rise(&button_rose);
    green_led = 0;
    green_led2 = 0;

    printf("System ready. State: IDLE\r\n");

    while (true)
    {
        updateLedsForState();
        IMUReading r; // one sample per loop

        if (buttonReleased)
        {
            buttonReleased = false;

            if (States_GetState() == STATE_IDLE)
            {
                if (buttonHoldMs >= LONG_PRESS_MS)
                {
                    States_StartRecord();
                    GestureCapture_Reset(); // for one gesture
                    printf("RECORDING\r\n");
                }
                else
                {
                    States_StartUnlock();
                    GestureCapture_Reset(); // for one gesture
                    printf("UNLOCKING\r\n");
                }
            }
        }

        switch (States_GetState())
        {
        case STATE_IDLE:
            // TODO: wait for button input
            // below is for debug
            if (imu.readIMU(r))
            {
                // start timestamp before gesture capture
                r.timestamp_ms = (uint32_t)duration_cast<milliseconds>(t.elapsed_time()).count();
                Gesture_t g;
                if (GestureCapture_Update(&r, &g))
                {
                    GestureResult gr = States_HandleGestureComplete(g);

                    float aDev = Gesture_AccelDeviationG(r.ax, r.ay, r.az);
                    float gMag = Gesture_GyroMagnitudeDps(r.gx, r.gy, r.gz);
                    bool moving = Gesture_IsMotionInstant(r.ax, r.ay, r.az, r.gx, r.gy, r.gz);
                    printf("aDev=%.3f gMag=%.1f moving=%d\r\n", aDev, gMag, (int)moving);
                }
                break;
                //
                break;

            case STATE_RECORDING:
                if (imu.readIMU(r))
                {
                    r.timestamp_ms = (uint32_t)duration_cast<milliseconds>(t.elapsed_time()).count();
                    float aDev = Gesture_AccelDeviationG(r.ax, r.ay, r.az);
                    float gMag = Gesture_GyroMagnitudeDps(r.gx, r.gy, r.gz);
                    bool moving = Gesture_IsMotionInstant(r.ax, r.ay, r.az, r.gx, r.gy, r.gz);

                    GestureCaptureState cur = GestureCapture_GetState();
                    if (cur != prev)
                    {
                        printf("CAP_STATE %d -> %d\r\n", (int)prev, (int)cur);
                        prev = cur;
                    }

                    if (r.timestamp_ms - lastPrint >= 100)
                    {
                        printf("aDev=%.3f gMag=%.1f moving=%d state=%d\r\n",
                               aDev, gMag, (int)moving, (int)cur);
                        lastPrint = r.timestamp_ms;
                    }

                    Gesture_t g;
                    if (GestureCapture_Update(&r, &g))
                    {
                        printf("GESTURE_DONE samples=%u duration=%lu ms\r\n",
                               g.sample_count,
                               (unsigned long)(g.end_time_ms - g.start_time_ms));
                        fflush(stdout);
                    }
                }
                ThisThread::sleep_for(5ms);

                break;

            case STATE_UNLOCKING:
                // below is placeholder for evaluation logic
                if (imu.readIMU(r))
                {
                    Gesture_t g;
                    if (GestureCapture_Update(&r, &g))
                    {
                        GestureResult gr = States_HandleGestureComplete(g);
                        printf("GestureResult=%d idx=%u err=%.2f\r\n",
                               (int)gr, States_GetGestureIndex(), States_GetLastError());
                        fflush(stdout);
                    }
                }
                break;

            case STATE_PASS:
                printf("PASS\r\n");
                green_led = 1;
                green_led2 = 0;
                ThisThread::sleep_for(RESULT_STATE_MS);
                States_ResetToIdle();
                break;

            case STATE_FAIL:
                printf("FAIL\r\n");
                green_led = 0;
                green_led2 = 1;
                ThisThread::sleep_for(RESULT_STATE_MS);
                States_ResetToIdle();
                break;
            }

            ThisThread::sleep_for(5ms);
        }
    }
}