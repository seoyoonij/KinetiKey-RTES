/*
currently thinking of 3 states,
driven by button interrupts

IDLE -> [button hold] -> RECORDING -> [3 gestures done] -> IDLE
IDLE -> [button press] -> UNLOCKING -> [3 gestures done] -> PASS/FAIL -> IDLE
*/

#ifndef STATES_H
#define STATES_H

#include <stdint.h>
#include <stdbool.h>
#include "gesture.h"

// define the possible states of the lock
typedef enum
{
    STATE_IDLE,
    STATE_RECORDING,
    STATE_UNLOCKING,
    STATE_PASS,
    STATE_FAIL
} LockState;

// define the possible results of handling a completed gesture
typedef enum
{
    GESTURE_IGNORED,
    GESTURE_RECORDED,
    GESTURE_MATCHED,
    RECORD_COMPLETE,
    UNLOCK_COMPLETE,
    UNLOCK_FAILED
} GestureResult;

void States_Init();
void States_StartRecord();
void States_StartUnlock();
void States_ResetToIdle();
void States_ClearKey();

GestureResult States_HandleGestureComplete(const Gesture_t &gesture);

LockState States_GetState();
uint8_t States_GetGestureIndex();
bool States_HasRecordedKey();
float States_GetLastError();

#endif
