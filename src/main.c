
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048

// Add here necessary states
enum state
{
    IDLE = 1
};
enum state programState = IDLE;
float ax, ay, az, gx, gy, gz, t;
int väli_laskuri = 0;
uint32_t BUFFER_SIZE = 270;
char morseviesti[500];
int merkki_laskuri = 0;
/**/

static void btn_fxn(uint gpio, uint32_t eventMask) {
    if(gpio == BUTTON1) {
        if (-0.15 < ay && ay < 0.15){
            morseviesti [merkki_laskuri] = '-';
            väli_laskuri = 0;
            merkki_laskuri += 1; 
            
        }
        else if (-0.15 < az && az < 0.15){
            morseviesti [merkki_laskuri] = '.';
            väli_laskuri = 0;
            merkki_laskuri += 1;
        }

    }

    if(gpio == BUTTON2){
        väli_laskuri += 1;
        if(väli_laskuri == 3){
            morseviesti [merkki_laskuri] = '\n';
            merkki_laskuri += 1;
            printf("koko morsekoodiviesti: %s \n", morseviesti);
            // printf("\n");
            // Kysyttiin tekoälyltä apua merkkijonon tyhjentämiseen /*Parametrit:
            //s: Osoitin siihen muistialueeseen, jota halutaan muuttaa.
            //c: Arvo, jolla muisti täytetään (annetaan int-muodossa, mutta se muunnetaan sisäisesti tavuksi eli se voi olla välillä 0–255).
            //n: Kuinka monta tavua muistialueesta täytetään. 
            memset(morseviesti, 0, sizeof(morseviesti));
            merkki_laskuri = 0;
            väli_laskuri = 0;
            return;
        }
        else{
            morseviesti [merkki_laskuri] = ' ';
            merkki_laskuri += 1;
            
        }
    }
    printf("%c", morseviesti[merkki_laskuri]);
        //printf("merkkilaskuri: %d, morseviesti[merkki_laskuri] = %c, väli_laskuri: %d \n", merkki_laskuri, morseviesti[merkki_laskuri], väli_laskuri);



    // Viimeksi kirjoitettu merkki on indeksissä merkki_laskuri - 1
    if(merkki_laskuri > 0) {
        printf("%c", morseviesti[merkki_laskuri - 1]);
                //printf("merkkilaskuri: %d, viimeisin merkki = %c, väli_laskuri: %d \n", merkki_laskuri, morseviesti[merkki_laskuri - 1], väli_laskuri);


    }
    //printf("koko morsekoodiviesti: %s \n", morseviesti);
}
static void imu_task(void *pvParameters)
{
    (void)pvParameters;

    if (init_ICM42670() == 0){
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0)
        {
            printf("ICM-42670P could not initialize accelerometer or gyroscope");
        }
    }
    else{
        printf("Failed to initialize ICM-42670P.\n");
    }

    while (1){
        if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0)
        {

            // printf("Accel: X=%f, Y=%f, Z=%f\n", ax, ay, az);
        }
        else
        {
            printf("Failed to read imu data\n");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void print_task(void *arg){
    (void)arg;

    // Wait till someone is listening
    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }
    stdio_puts("=== Printing values for temperature and sensor ===\n");

    while (1)
    {

        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

int main()
{
    stdio_init_all();
    // Uncomment this lines if you want to wait till the serial monitor is connected
    /*while (!stdio_usb_connected()){
        sleep_ms(10);
    }*/
    init_hat_sdk();
    sleep_ms(300); // Wait some time so initialization of USB and hat is done.
    init_button1();
    init_button2();
    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, btn_fxn);
    gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_FALL, true, btn_fxn);

    TaskHandle_t himu_task, hprint_task = NULL;
    // Create the tasks with xTaskCreate
    BaseType_t result = xTaskCreate(imu_task,           // (en) Task function
                                    "example",          // (en) Name of the task
                                    DEFAULT_STACK_SIZE, // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                                    NULL,               // (en) Arguments of the task
                                    2,                  // (en) Priority of this task
                                    &himu_task);        // (en) A handle to control the execution of this task

    if (result != pdPASS)
    {
        printf("Example Task creation failed\n");
        return 0;
    }

    result = xTaskCreate(print_task,         // (en) Task function
                         "print",            // (en) Name of the task
                         DEFAULT_STACK_SIZE, // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                         NULL,               // (en) Arguments of the task
                         2,                  // (en) Priority of this task
                         &hprint_task);      // (en) A handle to control the execution of this task

    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}
