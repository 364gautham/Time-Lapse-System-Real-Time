# Time-Lapse-System [Real Time]

-A real-time Embedded System built on real-time Rate monotonic theory. 

-Scheduling of real-time services are done using the help of Software Sequencer Implementation running at 20Hz [ Nanosleep Delay compensation mechanism]  

-Real-Time Acquisition of images from C-200 camera interfaced to RPI running at a 1Hz frequency and create a video(which would be a Time Lapse) using OpenCV. Capture Service running at 10 Hz and save service at 1Hz.

-The real-time accuracy of the system is tested for long durations of system running.

-OpenCV image difference analysis to capture frames and save image frame with a change in pixel value from a previously captured frame. Linux real-time capabilities – SCHED_FIFO scheduling policy, Linux priority and core affinity services.

-Slightly inefficient real-time system which runs at capture rate of 10Hz. It utilises and real time and best effort service mechanisms with circular buffer array of MAT frames.  
