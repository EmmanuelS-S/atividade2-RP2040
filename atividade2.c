#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pio_matriz.pio.h"

const uint8_t button_A = 5;
const uint8_t button_B = 6;

const uint8_t led_green = 11;
const uint8_t led_red = 13;

static volatile uint a = 1;
static volatile uint32_t last_time = 0;

void gpio_irq_handler(uint gpio, uint32_t events);
void init_buttons();
void init_leds();

int main(){
    init_buttons();
    init_leds();
    
    stdio_init_all();//inicializa funções de entrada e saída do C

    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
    
        
    }
}

void init_buttons(){
    gpio_init(button_A);
    gpio_init(button_B);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_set_dir(button_B,GPIO_IN);
    gpio_pull_up(button_A);
    gpio_pull_up(button_B);
}

void init_leds(){
    gpio_init(led_red);
    gpio_init(led_green);
    gpio_set_dir(led_green, GPIO_OUT);
    gpio_set_dir(led_red, GPIO_OUT);
}


void gpio_irq_handler(uint gpio, uint32_t events){
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    printf("A = %d\n", a);
    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 200000) // X ms de debouncing
    {
        last_time = current_time; // Atualiza o tempo do último evento
        printf("Mudanca de Estado do Led. A = %d\n", a);
        gpio_put(led_red, !gpio_get(led_red)); // Alterna o estado
        a++;                                     // incrementa a variavel de verificação
    }
}