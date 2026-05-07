1. first read from imu
2. store that in a custom structure: 3d vector of the point, timestamp, etc.
3. finish the "recording" logic: involve button press interrupt + time/motion intensity threshold to move from one motino to the next
4. finish unlocking logic: this involves comparing euclidean distance + fft if needed
5. link everything with IDLE
