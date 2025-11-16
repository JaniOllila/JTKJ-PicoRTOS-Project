
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include "morse.c"

#include "tkjhat/sdk.h"
  

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048 

#define INPUT_BUFFER_SIZE 256

// Creating a structure to deal with IMU data and naming it imuData
struct imu_data {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float t;
} imuData;

// Creating a buffer for morsecode, which will be displayed in LCD
char lcd_buffer[10] = "          "; 

// Creating a buffer for the last three marks. 
char last_marks[3] = "   ";

// Creating a text buffer for debugging messages
char text_buffer[INPUT_BUFFER_SIZE];

// Stores the whole morse input
char morse_line[128] = "";

int morse_line_index = 0;

// The final message after converting
char decoded_text[10] = "";

// Creating states for the state machine and making it start at IDLE state
enum state {IDLE=0, READ_IMU, READ_TAG, UPDATE_DATA};
enum state programState = IDLE;

// Creating task and function prototypes
static void imu_task(void *arg);
static void lcd_task(void *arg);
static void btn_fxn(uint gpio, uint32_t eventMask);
static void idle_task(void *arg);
static void serial_task(void *pvParameters);
void update_buffer(char *buffer, char new_mark);
void update_last_marks(char *buffer, char new_mark);
void detect_moves(char *buffer, struct imu_data *data);

void update_last_marks(char *buffer, char new_mark) {
    
    for (int i=0; i<2; i++) {
        buffer[i] = buffer[i+1];
    }
    buffer[2] = new_mark;
    printf("Last three marks: %c%c%c\n", buffer[0], buffer[1], buffer[2]);
    if (buffer[0] == ' ' && buffer[1] == ' ' && buffer[2] == '\n') {
        programState = IDLE;
    } 
}

// Creating a function to update the morsecode buffer. It shift the buffer one index to the left.
void update_buffer(char *buffer, char new_mark) {

    for (int i=0; i<9; i++) {
        buffer[i] = buffer[i+1];
    }

    buffer[9] = new_mark;

    update_last_marks(last_marks, new_mark);
}

// Creating a function to detect physical movements that creates morsecode marks. After that appends the marks to morse_line and calls morse_line_to_text() to translate the message.
void detect_moves(char *buffer, struct imu_data *data) {
    char mark = 0;

    if (data->gx > 100) {
        mark = '.';
        puts("Detected .");
    }
    else if (data->gx < -100) {
        mark = '-';
        puts("Detected -");
    }
    else if (data->gy > 100) {
        mark = ' ';
        puts("Detected space");
    }
    else if (data->gy < -100) {
        mark = '\n';
        puts("Detected new line");
    }
    else {
        return;
    }

    // Update the LCD buffer
    update_buffer(buffer, mark);

    // Append mark to the full morse_line
    if (morse_line_index < sizeof(morse_line) - 1) {
        morse_line[morse_line_index++] = mark;
        morse_line[morse_line_index] = '\0';
    }

    // If newline is detected it decodes the whole line
    if (mark == '\n') {
        morse_line_to_text(morse_line, decoded_text, sizeof(decoded_text));
        printf("Decoded: %s\n", decoded_text);

        // Reset for next message
        morse_line_index = 0;
        morse_line[0] = '\0';
    }

    programState = UPDATE_DATA;
    vTaskDelay(pdMS_TO_TICKS(50));
}

// Creating a button interrupt function to change the state machine state when button2 is pressed.
static void btn_fxn(uint gpio, uint32_t eventMask) {
    programState = (programState + 1) % 3;  // With button press, we can change to IDLE, READ_IMU AND READ_TAG states
    sprintf(text_buffer, "Button pressed, changing state to %d", programState);
    puts(text_buffer); 
}

//Creating a task for reading data from IMU sensor. Based on the data, it calls the detect_moves to add morsecode marks to the buffer.
static void imu_task(void *pvParameters) {
    (void)pvParameters;
    // Creating a pointer to imu_data structure
    struct imu_data *data = &imuData;

    // Data collection starts from the infinite loop which only operates when the state machine is at READ_IMU state.
    while (1)
    {
        if (programState == READ_IMU) {
            if (ICM42670_read_sensor_data(&data->ax, &data->ay, &data->az, &data->gx, 
                &data->gy, &data->gz, &data->t) == 0) {
                
                printf("Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f| Temp: %2.2f°C\n", 
                    imuData.ax, imuData.ay, imuData.az, imuData.gx, imuData.gy, imuData.gz, imuData.t);
                //puts(text_buffer);
                detect_moves(lcd_buffer, data);
                
            } else {
                puts("Failed to read imu data\n");
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        
    }
}

//Creating a task for displaying morsecode message in LCD
static void lcd_task(void *arg) {
    (void)arg;
        //First in the infinite loop, which only works when the state machine is at UPDATE_DATA state, 
        // it clears the lcd display. Then it writes the morsecode buffer to the lcd. After that
        // it waits a bit and changes the state to READ_IMU to continue creating the message.
        while(1){
            if (programState == UPDATE_DATA) {
                clear_display();
                write_text(decoded_text);
                vTaskDelay(pdMS_TO_TICKS(300));
                programState = READ_IMU;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            
        }
}

//Creating a task for waiting. It applies when the state machine is at IDLE state.
static void idle_task(void *arg) {
    (void)arg;
    while (1) {
        if (programState == IDLE) {
           vTaskDelay(pdMS_TO_TICKS(500));
        }
        
    }
}

//Creating a task for receiving messages from the serial-client. I only works when the state machine is at READ_TAG state.
static void serial_task(void *arg) {
    //This task is for receiving data from serial-client. It's mainly from the example but modified a bit to make it fit better. It reads what 
    //serial-client sends and then prints it to the serial monitor adding a "CLIENT: " to the start of the message to make it easier to see where
    // the data is coming from.
    (void)arg;
    
    char *line = text_buffer;
    size_t index = 0;
    
    while (1){
        if (programState == READ_TAG){
            int c = getchar_timeout_us(0);
            if (c != PICO_ERROR_TIMEOUT){
                if (c == '\r') continue; // ignore CR, wait for LF if (ch == '\n') { line[len] = '\0';
                if (c == '\n'){
                    // terminate and process the collected line
                    line[index] = '\0'; 
                    printf("__CLIENT:\"%s\"__\n", line); //Print as debug in the output
                    
                    index = 0;
                    vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
                }
                else if(index < INPUT_BUFFER_SIZE - 1){
                    line[index++] = (char)c;
                }
                else { //Overflow: print and restart the buffer with the new character. 
                    line[INPUT_BUFFER_SIZE - 1] = '\0';
                    printf("__CLIENT:\"%s\"__\n", line);
                    index = 0; 
                    line[index++] = (char)c; 
                }
            }
            else {
                vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
            }
        }
    }
}


int main() {
    // First setting up the stdio and waiting for usb connection.
    stdio_init_all();
    while (!stdio_usb_connected()){
        sleep_ms(10);
    }
    puts("USB connected.");
    // Then iniliazing the hat sdk.
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.
    // Setting up the led
    init_led();
    // Setting up the lcd
    init_display();
    // Setting up the IMU sensor. 
    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0){
            printf("ICM-42670P could not initialize accelerometer or gyroscope");
        }
    } else {
        printf("Failed to initialize ICM-42670P.\n");
    }

    // Initializing button2 and making an interruption when pressed to change the state. FYI the button1 seems not to work on our device.
    init_button2();
    gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_RISE, true, btn_fxn);

    TaskHandle_t IMUTask = NULL, LCDTask = NULL, IDLETask = NULL, SerialTask = NULL;
    // Creating the tasks with xTaskCreate
    BaseType_t result = xTaskCreate(imu_task, 
                "IMU",
                DEFAULT_STACK_SIZE,
                NULL,
                2, 
                &IMUTask);
    
    if(result != pdPASS) {
        puts("IMU Task creation failed");
        return 0;
    }
    result = xTaskCreate(lcd_task, 
                "LCD",
                DEFAULT_STACK_SIZE,
                NULL,
                2, 
                &LCDTask);
    
    if(result != pdPASS) {
        puts("LCD Task creation failed");
        return 0;
    }
    result = xTaskCreate(idle_task, 
                "IDLE",
                1024,
                NULL,
                2, 
                &IDLETask);
    
    if(result != pdPASS) {
        puts("IDLETask creation failed");
        return 0;
    }
    result = xTaskCreate(serial_task, 
                "Serial",
                DEFAULT_STACK_SIZE,
                NULL,
                2, 
                &SerialTask);
    
    if(result != pdPASS) {
        puts("IDLETask creation failed");
        return 0;
    }


    
    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}