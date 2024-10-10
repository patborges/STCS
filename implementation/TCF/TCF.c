
#include <stdio.h>
#include <unistd.h>  // for the pipe system call
#include <stdbool.h> // Needed for bool -> ON/OFF(1/0)
#include <stdlib.h>
#include <fcntl.h>   // doesn't stdlib do this ?
#include <string.h>
#include <libwebsockets.h>  // for ui (standby)

#define TEMP_INFO_PIPE "/tmp/temp_info_pipe" // Read from /tmp/temp_info_pipe to obtain the temperatures of each thermistor
#define RESPONSE_PIPE "/tmp/response_pipe"   // Write to /tmp/response_pipe to change the heater power status
// TSL write to INFO_PIPE: {THERM-01_TEMP}-{HTR-01_PWR};{THERM-02_TEMP}-{HTR-02_PWR};{THERM-03_TEMP}-{HTR-03_PWR};{THERM-04_TEMP}-{HTR-04_PWR}
// TCF write to from RESPONSE_PIPE: {HTR-01_PWR};{HTR-02_PWR};{HTR-03_PWR};{HTR-04_PWR}
#define BUFFER_SIZE 128

// Each thermistor as a heater
#define NUM_THERMISTORS 4

// Structure to store data for each WebSocket session
// struct session_data {
// };

// Thermistor and Heater
double THERM_01, THERM_02, THERM_03, THERM_04;
int HTR_01, HTR_02, HTR_03, HTR_04;

int time_clock;

float setpoint[4] = {0.0, 0.0, 0.0, 0.0};         // Default setpoint temperature is 0ºC for all 4
bool heater_status[4] = {true, true, true, true}; // Default heater status (All ON according to ui)

void bang_bang()
{
    // TODO: bangbang controller or PID controller

    // Check each thermistor's temperature and adjust heater status
    double temperatures[NUM_THERMISTORS] = {THERM_01, THERM_02, THERM_03, THERM_04};

    for (int i = 0; i < NUM_THERMISTORS; i++)
    {
        // Turn the heater ON if the temperature is below the setpoint
        if (temperatures[i] < setpoint[i])
        {
            heater_status[i] = true;
        }
        else
        {
            // Turn the heater OFF if the temperature is above the setpoint
            heater_status[i] = false;
        }
    }
}

// ----On Standby----
// void pid()
// {
//     // TODO: PID controller -> Which values do I use for Kp, Ki and Kd ?
//     // Example values 
//     double KP = 1.0;
//     double KI = 0.1;
//     double KD = 0.01;
//
//     for (int i = 0; i < NUM_THERMISTORS; i++)
//     {
//         // double error = temperature - setpoint;
//         // integral[i] += error;
//         // double derivative = error - prev_error[i];
//
//         // Formula: output = Kp × error + Ki × integral + Kd × derivative
//         double pid_output = (KP * error) + (KI * integral[i]) + (KD * derivative);
//
//         if (output > 0)
//         {
//         heater_status[i] = true; // Turn ON the heater
//         HTR[i] = 1;              // 1 - ON
//         }
//         else
//         {
//         heater_status[i] = false; // Turn OFF the heater
//         HTR[i] = 0;               // 0 - OFF
//         }
//         // previous_error[i] = error;
//     }
// }
// ------------------

void heating_status()
{
    // TODO: change the heater_status if needed

    // Sync the heater power status with the main heater variables
    HTR_01 = heater_status[0] ? 1 : 0;
    HTR_02 = heater_status[1] ? 1 : 0;
    HTR_03 = heater_status[2] ? 1 : 0;
    HTR_04 = heater_status[3] ? 1 : 0;
}

int main()
{
    char input_buffer[BUFFER_SIZE];
    char output_buffer[BUFFER_SIZE];
    // buffer is a region of memory used to hold data temporarily while it is being moved from one place to another
    // BUFFER_SIZE is the size of the buffer

    int file; // it's a named file (FIFO)

    while (1)
    {
        // First open TEMP_INFO_PIPE in read-only
        file = open(TEMP_INFO_PIPE, O_RDONLY);

        // ---- Handling possible system call fail ----
        if (file == -1)
        {                                            // open() returns -1 on failure
            perror("Failed to open TEMP_INFO_PIPE"); // print error message
            exit(1);                                 // exit with error code 1
        }

        if (read(file, input_buffer, BUFFER_SIZE) == -1)
        {                                                 // read() returns -1 on failure
            perror("Failed to read from TEMP_INFO_PIPE"); // print error message
            close(file);                                  // close the file descriptor
            exit(1);                                      // exit with error code 1
        }
        // ---------------------------------------------

        // read(file, input_buffer, BUFFER_SIZE);

        // Extract data
        // Print the read string and close
        printf("\n\n*DATA: %s*\n", input_buffer);
        // Clock;{THERM-01_TEMP}-{HTR-01_PWR};{THERM-02_TEMP}-{HTR-02_PWR};{THERM-03_TEMP}-{HTR-03_PWR};{THERM-04_TEMP}-{HTR-04_PWR}

        // A timestamp/ An environment status (Eclipse, Normal, Sun Exposure)/ An error status (optional, could be "Sensor Error", "Heater Failure", or empty)

        // -- To separate the data
        sscanf(input_buffer, "%d;%lf-%d;%lf-%d;%lf-%d;%lf-%d", &time_clock, &THERM_01, &HTR_01, &THERM_02, &HTR_02, &THERM_03, &HTR_03, &THERM_04, &HTR_04);
        // sscanf() reads the string and stores the values in the variables
        printf(" __________________________________ ");
        printf("\n\nClock: %d", time_clock);
        printf("\n __________________________________ ");
        printf("\n\nThermistor 1: %lf", THERM_01);
        printf(" | Heater 1: %d", HTR_01);
        printf("\nThermistor 2: %lf", THERM_02);
        printf(" | Heater 2: %d", HTR_02);
        printf("\nThermistor 3: %lf", THERM_03);
        printf(" | Heater 3: %d", HTR_03);
        printf("\nThermistor 4: %lf", THERM_04);
        printf(" | Heater 4: %d", HTR_04);
        printf("\n __________________________________ ");
        printf("\n");
        close(file);

        // Update heater status using bang_bang controller
        bang_bang();
        heating_status();

        // If PID controller works, uncomment next line
        // pid();

        // Now open in write mode and write string taken from DATA
        file = open(RESPONSE_PIPE, O_WRONLY);

        // To verify the output
        if (file == -1)
        {
            perror("Failed to open RESPONSE_PIPE");
            exit(1);
        }

        // NEED THIS -> {HTR-01_PWR};{HTR-02_PWR};{HTR-03_PWR};{HTR-04_PWR}
        sprintf(output_buffer, "%d;%d;%d;%d", HTR_01, HTR_02, HTR_03, HTR_04);
        // sprintf() -> printf to a string

        // fgets(output_buffer, BUFFER_SIZE, stdin);
        // write(file, output_buffer, strlen(output_buffer) + 1);

        if (write(file, output_buffer, BUFFER_SIZE) == -1)
        {
            perror("Failed to write to RESPONSE_PIPE");
            close(file);
            exit(1);
        }

        close(file);

        // escape hatch
    }
    printf("\nEnd");
    return 0;
}
