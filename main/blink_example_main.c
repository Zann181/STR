#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_system.h>

#define LED_GPIO2 2
#define LED_GPIO 26
#define BUTTON_GPIO 27

SemaphoreHandle_t xMutex; // Objeto de semaforo
QueueHandle_t buffer;     // Objeto de la cola

void tarea1(void* arg)
{
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    int button_state;
    while (1)
    {
        button_state = gpio_get_level(BUTTON_GPIO);

        // Estado del botón ha cambiado, enviarlo a la cola
        xQueueSend(buffer, &button_state, portMAX_DELAY);

         vTaskDelay(pdMS_TO_TICKS(250)); // Pequeña pausa para evitar rebotes del botón
        
    }
}

void tarea2(void* z)
{
    uint32_t rcv = 0;
    uint32_t variable = 0;

    while (1)
    {
        if (xQueueReceive(buffer, &rcv, pdMS_TO_TICKS(100)) == pdTRUE) // Si recibe el valor dentro de 1 segundo, muestra en pantalla
        {

            printf("Item recibido: %lu\n", rcv); // Muestra el valor en pantalla
            if (variable == rcv)
            {
                while (1)
                {
                    vTaskDelay(pdMS_TO_TICKS(500)); // Pequeña pausa para evitar rebotes del botón
                     xQueueReceive(buffer, &rcv, pdMS_TO_TICKS(100)); // Espera hasta que haya un elemento en la cola
                    if (variable == rcv)
                    {
                        break;
                    }
                    else
                    {
                        printf("Item recibido en rcv: %lu\n", rcv); // Muestra el valor en pantalla
                        printf("Item recibido en variable: %lu\n", variable);
                        gpio_set_level(LED_GPIO2, 1);
                        vTaskDelay(pdMS_TO_TICKS(250)); // Enciende el LED durante 500 ms
                        gpio_set_level(LED_GPIO2, 0);
                        vTaskDelay(pdMS_TO_TICKS(250)); // Apaga el LED durante 500 ms

                    }
                    
                    
                   
                    
                }
                


            }
            else printf("Fuera del if");
            vTaskDelay(pdMS_TO_TICKS(500)); // Pequeña pausa para evitar rebotes del botón
        }
        
    }
}

void app_main()
{
    buffer = xQueueCreate(10, sizeof(uint32_t)); // crea la cola *buffer* con 10 slots de 4 Bytes
    esp_rom_gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(LED_GPIO2);
    gpio_set_direction(LED_GPIO2, GPIO_MODE_OUTPUT);


    xTaskCreatePinnedToCore(tarea1, "tarea1", 4096, NULL, 4, NULL, 0); // Crea tarea que escribe valores en la cola
    xTaskCreatePinnedToCore(tarea2, "tarea2", 4096, NULL, 1, NULL, 0); // Crea tarea que lee valores de la cola.
}