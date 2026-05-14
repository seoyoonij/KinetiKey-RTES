# KinetiKey — Motion Password Lock

Record a 3-gesture sequence as a key. Replay it to unlock.

## User Manual
| Action | Result |
|---|---|
| Hold button (≥1s) | Enter RECORDING mode |
| Short press | Enter UNLOCKING mode |
| Perform 3 gestures | One at a time, pause between each |
| 3 gestures match key | PASS (green LED 1, 3s) |
| Any gesture fails | FAIL (green LED 2, 3s) |

*Example gestures: tilt, shapes, curves, etc.*

**LED feedback:**
- IDLE: both off
- RECORDING: alternating blink
- UNLOCKING: synchronous blink
- PASS: LED1 solid
- FAIL: LED2 solid

## Gesture Distinction criteria
- Direction of rotation (axes[3] from gyro integration)
- Motion intensity (peak angular velocity)
- Duration
- Acceleration energy profile (FFT of accel deviation signal)

## Detection Logic
Each gesture is captured as a feature vector:

| Feature | Source | How |
|---|---|---|
| Rotation vector | Gyro | integrate gx,gy,gz × dt |
| Peak velocity | Gyro | max angular speed during motion |
| Accel energy | Accel | sum of (|a|-1g)² per sample |
| Duration | Timer | end_ms - start_ms |
| FFT bins | Accel deviation signal | normalized energy per bin |

Comparison: weighted sum of per-feature absolute differences.  
Match threshold: total error < 80.0 (empirical).

## Gesture Separation
One gesture = one motion between two still periods.

- Motion start: `|a| - 1g > 0.18g` OR `|gyro| > 35 dps`
- Motion end: below stop threshold for ≥ 1000ms
- Min motion duration: 1000ms (discard noise)
- Max motion duration: 5000ms (force complete)

## State Machine
```
IDLE
  → [hold ≥1s] → RECORDING → [3 gestures done] → IDLE
  → [short press] → UNLOCKING → [3 gestures done] → PASS or FAIL → IDLE
```

## Pipeline
[ SENSE -> CAPTURE -> COMPARE -> DECIDE ]

```
LSM6DSL (I2C, 104Hz)
    ↓ ax,ay,az (g), gx,gy,gz (dps)
GestureCapture_Update()
    WAITING → CAPTURING → END_CANDIDATE → emit Gesture_t
    ↓ Gesture_t (features + FFT)
States_HandleGestureComplete()
    RECORDING: store to recorded_key[0..2]
    UNLOCKING: Compare_Gestures() per gesture
        ↓ CompareResult (rotation, energy, duration, peak, FFT)
STATE_PASS / STATE_FAIL
    ↓ LED feedback → IDLE
```
