Motion password system.

=== File structure===
GESTURE: vectors and math
STATES: IDLE, RECORDING, UNLOCKING etc.
IMU: imu driver functions
MAIN: timing
=====================

# Defining motion passwords
### Distinction criteria
* Angle of tilt: 45deg ≠ 60 deg
* Shape: circle ≠ triangle
* Intensity: slow push ≠ jerk; fast circle ≠ slow circle
* Size: small circle ≠ large circle

### End-of-Gesture
* current: time-based pause
* consideration: closed shape/ physical End-of-Gesture

### Example gestures
* tilt, shapes, curves, etc.


# Detection Logic
* motion → compress → scalars → match scalars
* General approach: Δ=∣Performed−Recorded|, threshold for each
* Each point is a vector in 3D.

| Data          | Information                        | Input                                           |
|---------------|------------------------------------|-------------------------------------------------|
| Vector        | Direction/magnitude of rotation    | Gyro                                            |
| Euclidean distance   | Accuracy of motion                 | d = sqrt((x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2)     |
| Intensity     | Max angular velocity               | Gyro                                            |
| Duration      | How long is motion                 | Timer                                           |


# Gesture Logic (State Machine)
* 3 states
* Driven by button interrupts

IDLE -> [button hold] -> RECORDING -> [3 gestures done] -> IDLE \
IDLE -> [button press] -> UNLOCKING -> [3 gestures done] -> PASS/FAIL -> IDLE \

*during RECORDING/UNLOCKING, subloop pseudocde:*
* WAIT 
* if motion is big enough, enter OBSERVE
* OBSERVE (high priority, over Nyquist): \
    start logging motion. \
    if motion is small for long enough, then one motion is done. 
* EVALUATE (low priority- ISR): \ 
    if above threshold error (pass), then blink LED, move onto next motion.\
    else (fail), then reset motion count, return to IDLE state.

# Pipeline Overview (wip)
[ SENSE -> CAPTURE -> COMPARE -> DECIDE ]

IMU Sample (Ticker, 50Hz)
    ↓
Gesture Window Capture (ring buffer, ~100 samples)
    ↓
Feature Extraction (magnitude signal √ax²+ay²+az²)
    ↓
Gesture Boundary Detection (motion start/end via threshold)
    ↓
Store or Compare (DTW against stored template)
    ↓
LED Feedback