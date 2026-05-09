#include "states.h"
#include "gesture.h"

static LockState current_state = STATE_IDLE;
static Gesture_t recorded_key[3]; // stores the 3-gesture key
static uint8_t gesture_index = 0;
static bool key_recorded = false;
static bool unlocked = false;
static float last_error = 0.0f;

static const uint8_t REQUIRED_GESTURES = 3;
static const float GESTURE_ERROR_THRESHOLD = 30.0f; // needs tuning based on testing

// initialize the state machine, called once at startup
void States_Init()
{
    current_state = STATE_IDLE;
    gesture_index = 0;
    key_recorded = false;
    last_error = 0.0f;
    unlocked = false;

    for (uint8_t i = 0; i < REQUIRED_GESTURES; i++)
    {
        Gesture_Reset(&recorded_key[i]);
    }
}

// called when button is held down to start recording
void States_StartRecord()
{
    // if already have a recorded key and not unlocked yet, fail immediately
    if (key_recorded && !unlocked)
    {
        current_state = STATE_FAIL;
        return;
    }

    current_state = STATE_RECORDING;
    gesture_index = 0;
    key_recorded = false;
    unlocked = false;
    last_error = 0.0f;
}

// called when button is pressed to start unlocking
void States_StartUnlock()
{
    if (!key_recorded)
    {
        current_state = STATE_FAIL;
        return;
    }

    current_state = STATE_UNLOCKING;
    gesture_index = 0;
    last_error = 0.0f;
}

// called to reset back to idle state, can be triggered by button release or after pass/fail
void States_ResetToIdle()
{
    current_state = STATE_IDLE;
    gesture_index = 0;
}

// called to clear the recorded key, can be triggered by a long press while idle
void States_ClearKey()
{
    current_state = STATE_IDLE;
    gesture_index = 0;
    key_recorded = false;
    unlocked = false;
    last_error = 0.0f;

    for (uint8_t i = 0; i < REQUIRED_GESTURES; i++)
    {
        Gesture_Reset(&recorded_key[i]);
    }
}

// called when a gesture is completed, returns the result of the gesture handling
GestureResult States_HandleGestureComplete(const Gesture_t &gesture)
{
    // check is all 3 gestures were already checked
    if (gesture_index >= REQUIRED_GESTURES)
    {
        return GESTURE_IGNORED;
    }

    if (current_state == STATE_RECORDING)
    {
        recorded_key[gesture_index] = gesture; // store the gesture
        gesture_index++;

        // if all 3 gestures recorded, save the key and go back to idle
        if (gesture_index >= REQUIRED_GESTURES)
        {
            key_recorded = true; // all recorded
            current_state = STATE_IDLE;
            gesture_index = 0;
            return RECORD_COMPLETE;
        }

        return GESTURE_RECORDED;
    }

    // compares performed gesture to recorded key
    if (current_state == STATE_UNLOCKING)
    {
        last_error = Gesture_Error(gesture, recorded_key[gesture_index]);

        // if error exceeds threshold, fail immediately
        if (last_error > GESTURE_ERROR_THRESHOLD)
        {
            current_state = STATE_FAIL;
            gesture_index = 0;
            return UNLOCK_FAILED;
        }

        gesture_index++;

        // if all 3 gestures matched, unlock successful
        if (gesture_index >= REQUIRED_GESTURES)
        {
            unlocked = true;
            current_state = STATE_PASS;
            gesture_index = 0;
            return UNLOCK_COMPLETE;
        }

        return GESTURE_MATCHED;
    }

    return GESTURE_IGNORED;
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

float States_GetLastError()
{
    return last_error;
}
