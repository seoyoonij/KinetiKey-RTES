1. add gyro reads to LSM6DSL.cpp
2. build gesture capture:
   - wait for motion
   - start logging samples
   - keep updating Gesture_t
   - end gesture after motion is quiet for long enough
3. connect gesture capture to STATE_RECORDING:
   - long press enters record mode
   - capture 3 gestures
   - call States_HandleGestureComplete() after each gesture
   - blink/print after each recorded gesture
4. connect gesture capture to STATE_UNLOCKING:
   - short press enters unlock mode
   - capture 3 gestures
   - compare each gesture to recorded key
   - pass/fail based on States_HandleGestureComplete()
5. flesh out Gesture_Error():
   - compare gyro rotation vector
   - compare duration
   - compare peak velocity/intensity
   - add FFT features later if needed
6. tune thresholds on the actual board:
   - motion start threshold
   - end-of-gesture quiet time
   - gesture match tolerance

