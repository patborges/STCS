
## Thermal Control Function (TCF) 
Maintains optimal temperature using a PID control mechanism and manages the heaters.

heaters: HTR-01, HTR-02, HTR-03, HTR-04.
thermistors: THERM-01, THERM-02, THERM-03, THERM-04.

# Requirements
    1. Obtain thermal data from the interface provided by TSL -> Pipes
    2. Feed the thermal data to a Proportional-Integral-Derivate (PID) controller for each thermistor -> PID or Bang Bang
    3. Power-on/off one or more heaters to adjust the temperature of each thermistor if needed -> ON/OFF

// Default setpoint: 0.0ºC for all thermistors.

// The TCF shall allow a user to change the setpoint temperature for a specific thermistor or all thermistors at runtime between the values of -20.0ºC and 20.0ºC .
// Temperature -> float


## PID Controller

# PID parameters:
    • Proportional gain (Kp)
    • Integral gain (Ki)
    • Derivative gain (Kd)

# General Tips for PID Controller:
    • Obtain an open-loop response and determine what needs to be improved
    • Add a proportional control to improve the rise time
    • Add a derivative control to reduce the overshoot
    • Add an integral control to reduce the steady-state error
    • Adjust each of the gains Kp, Ki, and Kd until you obtain a desired overall response. 

## Formula (Simpler version)
output = Kp × error + Ki × integral + Kd × derivative


// ------------------------------------

Note:
1 sleep = 1s = 1000ms 
1 Hz is the frequency of 1 event per second


PIPE Parameters :
fd[0] - read 
fd[1] - write 

```c
// read() reads from the pipe/file and returns the number of bytes read
// write() writes to the pipe/file and returns the number of bytes written
// close() closes the pipe/file and returns 0


// ------------ Start Code (to ignore) -------------
float temperature; 

printf("Default setpoint: 0.0ºC for all thermistors.");

// Prompt the user for input
printf("\nChoose the temperature: "); // range: -20.0ºC and 20.0ºC
scanf("%f", &temperature);

if (temperature < -20.0 || temperature > 20.0) {
    printf("\nInvalid temperature");
} else {
    printf("\nValid temperature");
}
```
