#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pio_matriz.pio.h"

#define NUM_PIXELS 25 //tamanho do vetor para a matriz de led
#define NUM_FRAMES_A 25//quantidade de imagens que a animação vai possuir
#define NUM_FRAMES_B 10

const uint8_t button_A = 5;
const uint8_t button_B = 6;

const uint8_t led_green = 11;
const uint8_t led_red = 13;

const uint8_t matriz_led_pin = 7;

static volatile uint a = 1;
static volatile bool green_state = true;
static volatile uint32_t last_time_A = 0;
static volatile uint32_t last_time_B = 0;
static volatile int frame_atual = 0;  // Frame exibido atualmente (0-9)
static volatile bool estado_botao_a = false;
static volatile bool estado_botao_b = false;
static volatile int direcao_botao_a = 1;  // 1 para frente, -1 para trás
static volatile int direcao_botao_b = 1;  // 1 para frente, -1 para trás


double imagem_0[NUM_PIXELS]=
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0., 0.0, 0.0, 0.0, 0.0, 
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0};


double botao_B[NUM_FRAMES_B][NUM_PIXELS] = {
    // Frame 0 - Intensidade 0.01
    {0.0, 0.01, 0.01, 0.01, 0.0,
     0.01, 0.01, 0.01, 0.01, 0.01, 
     0.01, 0.0, 0.01, 0.0, 0.01,
     0.01, 0.01, 0.01, 0.01, 0.01,
     0.01, 0.0, 0.01, 0.0, 0.01},
    
    // Frame 1 - Intensidade 0.02
    {0.0, 0.02, 0.02, 0.02, 0.0,
     0.02, 0.02, 0.02, 0.02, 0.02, 
     0.02, 0.0, 0.02, 0.0, 0.02,
     0.02, 0.02, 0.02, 0.02, 0.02,
     0.02, 0.0, 0.02, 0.0, 0.02},
    
    // Frame 2 - Intensidade 0.03
    {0.0, 0.03, 0.03, 0.03, 0.0,
     0.03, 0.03, 0.03, 0.03, 0.03, 
     0.03, 0.0, 0.03, 0.0, 0.03,
     0.03, 0.03, 0.03, 0.03, 0.03,
     0.03, 0.0, 0.03, 0.0, 0.03},
    
    // Frame 3 - Intensidade 0.04
    {0.0, 0.04, 0.04, 0.04, 0.0,
     0.04, 0.04, 0.04, 0.04, 0.04, 
     0.04, 0.0, 0.04, 0.0, 0.04,
     0.04, 0.04, 0.04, 0.04, 0.04,
     0.04, 0.0, 0.04, 0.0, 0.04},
    
    // Frame 4 - Intensidade 0.05
    {0.0, 0.05, 0.05, 0.05, 0.0,
     0.05, 0.05, 0.05, 0.05, 0.05, 
     0.05, 0.0, 0.05, 0.0, 0.05,
     0.05, 0.05, 0.05, 0.05, 0.05,
     0.05, 0.0, 0.05, 0.0, 0.05},
    
    // Frame 5 - Intensidade 0.06
    {0.0, 0.06, 0.06, 0.06, 0.0,
     0.06, 0.06, 0.06, 0.06, 0.06, 
     0.06, 0.0, 0.06, 0.0, 0.06,
     0.06, 0.06, 0.06, 0.06, 0.06,
     0.06, 0.0, 0.06, 0.0, 0.06},
    
    // Frame 6 - Intensidade 0.07
    {0.0, 0.07, 0.07, 0.07, 0.0,
     0.07, 0.07, 0.07, 0.07, 0.07, 
     0.07, 0.0, 0.07, 0.0, 0.07,
     0.07, 0.07, 0.07, 0.07, 0.07,
     0.07, 0.0, 0.07, 0.0, 0.07},
    
    // Frame 7 - Intensidade 0.08
    {0.0, 0.08, 0.08, 0.08, 0.0,
     0.08, 0.08, 0.08, 0.08, 0.08, 
     0.08, 0.0, 0.08, 0.0, 0.08,
     0.08, 0.08, 0.08, 0.08, 0.08,
     0.08, 0.0, 0.08, 0.0, 0.08},
    
    // Frame 8 - Intensidade 0.09
    {0.0, 0.09, 0.09, 0.09, 0.0,
     0.09, 0.09, 0.09, 0.09, 0.09, 
     0.09, 0.0, 0.09, 0.0, 0.09,
     0.09, 0.09, 0.09, 0.09, 0.09,
     0.09, 0.0, 0.09, 0.0, 0.09},
    
    // Frame 9 - Intensidade 0.1 (máximo)
    {0.0, 0.1, 0.1, 0.1, 0.0,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.0, 0.1, 0.0, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.0, 0.1, 0.0, 0.1}
};

double botao_A[NUM_FRAMES_A][NUM_PIXELS]={
    //frame 0
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.1, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 1
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 2
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 3
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.0, 0.1, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

    //frame 4
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.1, 0.1, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.0, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 5
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.1, 0.1, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.1, 0.0,
          0.0, 0.0, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 6
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.1, 0.1, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.1, 0.0,
          0.0, 0.1, 0.0, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 7
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.1, 0.1, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.1, 0.0,
          0.0, 0.1, 0.1, 0.0, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 8
    {
  /*->*/  0.0, 0.0, 0.0, 0.0, 0.0,
          0.0, 0.1, 0.1, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.1, 0.1, 0.1, 0.0,
          0.0, 0.1, 0.1, 0.1, 0.0, /*<-*/
    /*->*/0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 9
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0, 
     0.0, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 10
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.0, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 11
    {0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 12
    {0.1, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 13
    {0.1, 0.1, 0.0, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 14
    {0.1, 0.1, 0.1, 0.0, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 15
    {0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 16
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 17
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.0,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

      //frame 18
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 19
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.0},

     //frame 20
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.0, 0.1},

      //frame 21
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.0, 0.1, 0.1},

      //frame 22
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.0, 0.1, 0.1, 0.1},

     //frame 23
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.0, 0.1, 0.1, 0.1, 0.1},

    //frame 24
    {0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1, 
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1,
     0.1, 0.1, 0.1, 0.1, 0.1},

};



void gpio_irq_handler(uint gpio, uint32_t events);
void init_buttons();
void imprimir_binario(int num);
uint32_t matrix_rgb(double b, double r, double g);
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b);
void gerar_sequencia_aleatoria();



int main(){

    PIO pio = pio0; 
    bool ok;
    uint16_t i;
    uint32_t valor_led;
    double r = 0.0, b = 0.0 , g = 0.0;

    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, matriz_led_pin);

    init_buttons();

    
    
    stdio_init_all();//inicializa funções de entrada e saída do C

    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(button_B, GPIO_IRQ_EDGE_FALL, true);// Só cha ma a função configurada acima

    while (true) {
        // Exibe o frame atual continuamente
        if(estado_botao_a){
            desenho_pio(botao_A[frame_atual], valor_led, pio, sm, r, g, b);
            sleep_ms(5);
            frame_atual += direcao_botao_a;

            // Verifica se chegou ao final (vai para trás)
            if(frame_atual >= NUM_FRAMES_A){
                frame_atual = NUM_FRAMES_A - 1;  // Volta um frame para não sair do vetor
                direcao_botao_a = -1;  // Muda direção para trás
            }
            // Verifica se voltou ao início (para a animação)
            else if(frame_atual < 0){
                frame_atual = 0;  // Garante que fica no frame 0
                estado_botao_a = false;  // Encerra a animação
                direcao_botao_a = 1;  // Reseta direção para próxima vez
            }
        }else if(estado_botao_b){
            desenho_pio(botao_B[frame_atual], valor_led, pio, sm, r, g, b);
            sleep_ms(2);
            frame_atual += direcao_botao_b;
            
            // Verifica se chegou ao final (vai para trás)
            if(frame_atual >= NUM_FRAMES_B){
                frame_atual = NUM_FRAMES_B - 1;  // Volta um frame para não sair do vetor
                direcao_botao_b = -1;  // Muda direção para trás
            }
            // Verifica se voltou ao início (para a animação)
            else if(frame_atual < 0){
                frame_atual = 0;  // Garante que fica no frame 0
                estado_botao_b = false;  // Encerra a animação
                direcao_botao_b = 1;  // Reseta direção para próxima vez
            }
        }
        else{
            desenho_pio(imagem_0, valor_led, pio, sm, r, g, b);
        }
        
        sleep_ms(200);
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



void gpio_irq_handler(uint gpio, uint32_t events){
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    if(gpio == button_A){
        // Verifica debouncing específico do botão A
        if (current_time - last_time_A > 200000) {
            last_time_A = current_time;
            
            // Parar botão B e iniciar animação A
            estado_botao_b = false;
            estado_botao_a = true;
            frame_atual = 0;  // SEMPRE reseta para começar do primeiro frame
            direcao_botao_a = 1;  // Reseta direção para frente
            
            //gpio_put(led_red, !gpio_get(led_red)); // Alterna o estado
        }
    } 
    else if(gpio == button_B){
        // Verifica debouncing específico do botão B
        if (current_time - last_time_B > 200000) {
            last_time_B = current_time;
            
            // Parar botão A e iniciar animação B
            estado_botao_a = false;
            estado_botao_b = true;
            frame_atual = 0;  // SEMPRE reseta para começar do primeiro frame
            direcao_botao_b = 1;  // Reseta direção para frente
            /*
            gpio_put(led_green, green_state); 
            green_state = ! green_state;
            */
        }
    }
}

//imprimir valor binário
void imprimir_binario(int num) {
 int i;
 for (i = 31; i >= 0; i--) {
  (num & (1 << i)) ? printf("1") : printf("0");
}
    printf("\n");
}

//rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double b, double r, double g){
  unsigned char R, G, B;
  //brilho total 255
  R = r * 200; 
  G = g * 200;
  B = b * 200;
  return (G << 24) | (R << 16) | (B << 8);  //isso é a o que define a cor do led?

}

//rotina para acionar a matrix de leds - ws2812b
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b){

    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        if (i%2==0)
        {
            //b=desenho[24-i], r=0.0, g=0.0
            valor_led = matrix_rgb(desenho[24-i], desenho[24-i], desenho[24-i]);
            pio_sm_put_blocking(pio, sm, valor_led);

        }else{
            //b=0.0, desenho[24-i], g=0.0
            //desenho[24-i], desenho[24-i], desenho[24-i]
            valor_led = matrix_rgb(desenho[24-i], desenho[24-i], desenho[24-i]);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
    }
    imprimir_binario(valor_led);
}
