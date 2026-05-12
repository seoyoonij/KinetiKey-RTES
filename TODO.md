## Current status

1. Main flow is wired up:
   - board starts in IDLE
   - long press enters RECORDING
   - short press enters UNLOCKING
   - RECORDING captures 3 gestures and stores the key
   - UNLOCKING captures 3 gestures and compares against the recorded key
   - PASS/FAIL states show LEDs briefly, then return to IDLE

2. Gesture capture now stores compact features instead of raw sample buffers:
   - sample count
   - start/end time and duration
   - first and last IMU sample for debugging
   - integrated gyro axes
   - peak gyro velocity
   - accel energy
   - peak accel deviation
   - FFT features

3. FFT is back in the gesture path:
   - gesture.cpp builds a temporary accel-deviation signal during capture
   - FFT features are computed when one gesture completes
   - Gesture_t stores FFTFeaturesResult and fft_valid
   - compare.cpp compares stored FFT features instead of needing raw samples

4. Compare prints tuning data:
   - rotation error
   - accel energy-per-second error
   - duration error
   - peak gyro error
   - FFT error
   - total error
   - matched/not matched

5. Unlock debug behavior is temporary:
   - states.cpp currently checks all 3 unlock gestures before failing
   - this is marked with TEMP DEBUG
   - final behavior may go back to fail-fast after tuning

## What is left

1. Tune compare thresholds and weights:
   - current values are experimental
   - collect same-gesture and different-gesture error ranges
   - choose MATCH_THRESHOLD between highest good error and lowest bad error

2. Decide whether to use relative error:
   - slow and fast gestures may produce different score ranges
   - peak velocity and accel energy may work better as relative errors
   - duration error was added to penalize much slower/faster repeats

3. Tune gesture separation:
   - start thresholds
   - stop thresholds
   - still_time_ms
   - min_motion_ms
   - max_motion_ms

4. Clean up debug prints before final demo:
   - COMPARE output

5. Decide final unlock behavior:
   - keep checking all 3 gestures for debug
   - or fail immediately on first wrong gesture for final behavior

6. Test full expected flow:
   - record 3 gestures
   - unlock with same 3 gestures
   - unlock with 1 wrong gesture
   - unlock with all wrong gestures
   - verify LEDs and serial output match state
