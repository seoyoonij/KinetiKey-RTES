#include "states.h"
#include "gesture.h"

static LockState current_state = STATE_IDLE;
static Gesture_t recorded_key[3];
static uint8_t gesture_index = 0;
static bool key_recorded = false;

static const uint8_t REQUIRED_GESTURES = 3;
static const float GESTURE_ERROR_THRESHOLD = 30.0f; // tune experimentally

void States_Init()
{
    current_state = STATE_IDLE;
    gesture_index = 0;
    key_recorded = false;

    for (int i = 0; i < REQUIRED_GESTURES; i++)
    {
        Gesture_Reset(&recorded_key[i]);
    }
}

void States_StartRecord()
{
    current_state = STATE_RECORDING;
    gesture_index = 0;
    key_recorded = false;
}

void States_StartUnlock()
{
    if (!key_recorded)
    {
        current_state = STATE_FAIL;
        return;
    }

    current_state = STATE_UNLOCKING;
    gesture_index = 0;
}

void States_HandleGestureComplete(Gesture_t gesture)
{
    if (current_state == STATE_RECORDING)
    {
        recorded_key[gesture_index] = gesture;
        gesture_index++;

        if (gesture_index >= REQUIRED_GESTURES)
        {
            key_recorded = true;
            current_state = STATE_IDLE;
        }

        return;
    }

    if (current_state == STATE_UNLOCKING)
    {
        float error = Gesture_Error(gesture, recorded_key[gesture_index]);

        if (error > GESTURE_ERROR_THRESHOLD)
        {
            current_state = STATE_FAIL;
            gesture_index = 0;
            return;
        }

        gesture_index++;

        if (gesture_index >= REQUIRED_GESTURES)
        {
            current_state = STATE_PASS;
        }

        return;
    }
}

LockState States_GetState()
{
    return current_state;
}

uint8_t States_GetGestureIndex()
{
    return gesture_index;
}

bool States_HasRecordedKey()
{
    return key_recorded;
}
