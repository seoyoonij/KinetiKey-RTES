## Current status

1. Main flow is wired up:
   - board starts in IDLE
   - long press enters RECORDING
   - short press enters UNLOCKING
   - RECORDING captures 3 gestures and stores the key
   - UNLOCKING captures and compares 3 gestures
   - PASS/FAIL states show LEDs briefly, then return to IDLE

2. Gesture capture stores compact features:
   - sample count
   - start/end time and duration
   - first and last IMU sample
   - integrated gyro axes
   - peak gyro velocity
   - accel energy
   - peak accel deviation
   - FFT features

3. Compare uses multiple gesture metrics:
   - rotation error
   - accel energy-per-second error
   - duration error
   - peak gyro error
   - FFT error
   - total weighted error

4. Unlock behavior:
   - all 3 unlock gestures are checked before deciding PASS/FAIL
   - this gives better feedback and avoids failing immediately on gesture 1

## Future improvements

1. Further tune thresholds and weights:
   - collect more same-gesture and different-gesture trials
   - tune MATCH_THRESHOLD using the gap between highest good error and lowest bad error
   - retune weights if one metric dominates the total error too much

2. Improve slow/fast gesture consistency:
   - consider relative error for peak velocity
   - consider relative error for accel energy-per-second
   - keep duration error to penalize gestures that are much slower or faster

3. Improve gesture separation:
   - tune start thresholds
   - tune stop thresholds
   - tune still_time_ms
   - tune min_motion_ms
   - tune max_motion_ms

4. Improve stored gesture features:
   - experiment with downsampled FFT input across the whole gesture
   - compare FFT features across gyro channels too, not only accel deviation
   - consider storing multiple attempts per recorded gesture and averaging features
