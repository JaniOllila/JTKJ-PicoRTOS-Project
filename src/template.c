
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048 

// Creating a structure to deal with IMU data and naming it imuData
struct imu_data {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float t;
} imuData = {.ax=0.0, .ay=0.0, .az=0.0, .gx=0.0, .gy=0.0, .gz=0.0, .t=0.0};


//Add here necessary states
enum state {IDLE=0, READ_IMU, UPDATE_DATA, READ_TAG};
enum state programState = IDLE;

//Creating task prototypes
static void imu_task(void *pvParameters);
static void lcd_task(void *arg);
static void btn_fxn(uint gpio, uint32_t eventMask);
static void idle_task(void *arg);

static void btn_fxn(uint gpio, uint32_t eventMask) {
    programState = (programState + 1) % 3;
    //printf("Button pressed, changing state to %d\n", programState); // used copilot auto-completion for this line
}

//Creating a task for reading data from IMU sensor
static void imu_task(void *pvParameters) {
    (void)pvParameters;
    // Creating a pointer to imu_data structure
    struct imu_data *data = &imuData;

    // Setting up the sensor. 
    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0){
            printf("ICM-42670P could not initialize accelerometer or gyroscope");
        }
    } else {
        printf("Failed to initialize ICM-42670P.\n");
    }
    // Start collection data here. Infinite loop. 
    while (1)
    {
        if (programState == READ_IMU) {
            if (ICM42670_read_sensor_data(&data->ax, &data->ay, &data->az, &data->gx, 
                &data->gy, &data->gz, &data->t) == 0) {
                
                

                //printf("Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f| Temp: %2.2f°C\n", data->ax, data->ay, data->az, 
                    //data->gx, data->gy, data->gz, data->t);
                
            } else {
                printf("Failed to read imu data\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

//Creating a task for displaying morsecode in LCD
static void lcd_task(void *arg) {
    (void)arg;
        init_display();
        for(;;){
            if (programState == UPDATE_DATA) {
                write_text("---...---.\n");
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
}

static void idle_task(void *arg) {
    (void)arg;
    while (1) {
        if (programState == IDLE) {
            printf("Idling\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}


int main() {
    stdio_init_all();
    // Uncomment this lines if you want to wait till the serial monitor is connected
    /*while (!stdio_usb_connected()){
        sleep_ms(10);
    }*/ 
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.

    // Initializing button2 and making an interruption when pressed to change the state. FYI the button1 seems not to work.
    init_button2();
    gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_RISE, true, btn_fxn);

    TaskHandle_t IMUTask, LCDTask = NULL;
    // Creating the tasks with xTaskCreate
    BaseType_t result = xTaskCreate(imu_task, 
                "IMU",
                DEFAULT_STACK_SIZE,
                NULL,
                2, 
                &IMUTask);
    
    if(result != pdPASS) {
        printf("IMU Task creation failed\n");
        return 0;
    }
    result = xTaskCreate(lcd_task, 
                "LCD",
                DEFAULT_STACK_SIZE,
                NULL,
                2, 
                &LCDTask);
    
    if(result != pdPASS) {
        printf("LCD Task creation failed\n");
        return 0;
    }

    
    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}

