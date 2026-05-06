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

typedef enum
{
    STATE_IDLE,
    STATE_RECORDING,
    STATE_UNLOCKING,
    STATE_PASS,
    STATE_FAIL
} LockState;

void States_Init();
void States_StartRecord();
void States_StartUnlock();
void States_HandleGestureComplete(Gesture_t gesture);

LockState States_GetState();
uint8_t States_GetGestureIndex();
bool States_HasRecordedKey();

#endif
