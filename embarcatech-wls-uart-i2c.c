#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "ssd1306.h"
#include "ws2812.pio.h"

// Definições de Hardware
#define BUTTON_A_PIN 5        // Pino do Botão A
#define BUTTON_B_PIN 6        // Pino do Botão B
#define LED_GREEN_PIN 11      // Pino do LED verde
#define LED_BLUE_PIN 12       // Pino do LED azul
#define LED_RED_PIN 13        // Pino do LED vermelho
#define I2C_PORT i2c1        // Porta I2C para o display
#define I2C_SDA 14           // Pino SDA
#define I2C_SCL 15           // Pino SCL
#define UART_ID uart0        // Definição da UART0
#define BAUD_RATE 115200     // Taxa de transmissão UART
#define UART_TX_PIN 0        // Pino TX da UART
#define UART_RX_PIN 1        // Pino RX da UART
#define WS2812_PIN 7         // Pino para controlar a matriz WS2812
#define NUM_PIXELS 25        // Número de LEDs na matriz
#define MATRIX_SIZE 5        // Tamanho da matriz 5x5
#define DEBOUNCE_DELAY 200   // Delay para o debouncing dos botões (em ms)
#define MATRIX_WIDTH 5       // Largura da matriz
#define MATRIX_HEIGHT 5      // Altura da matriz

// Estados e Configurações Globais
static PIO ws2812_pio = pio0;            // PIO usado para controle da matriz WS2812
static uint ws2812_sm = 0;              // Máquina de estado para a matriz WS2812
ssd1306_t display;                       // Variável para o display SSD1306
volatile bool led_green_state = false;   // Estado do LED verde
volatile bool led_blue_state = false;    // Estado do LED azul
volatile uint32_t last_button_a_time = 0; // Tempo da última interrupção do botão A
volatile uint32_t last_button_b_time = 0; // Tempo da última interrupção do botão B

// Protótipos de funções
void ws2812_init(void);
void put_pixel(uint32_t pixel_grb);
uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b);
void clear_leds(void);
void display_number(int num);
void update_display(const char *text);
void uart_init_custom(void);
void process_uart_input(void);
void setup_buttons(void);
void setup_display(void);
void gpio_callback(uint gpio, uint32_t events);

// Padrões Numéricos para Matriz 5x5 (0-9)
const uint8_t number_patterns[10][MATRIX_SIZE][MATRIX_SIZE] = {
    {{1,1,1,1,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,1}},  // Número 0
    {{0,1,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {0,0,1,0,0}, {1,1,1,1,1}},  // Número 1
    {{1,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,0}, {1,1,1,1,1}},  // Número 2
    {{1,1,1,1,1}, {0,0,0,0,1}, {0,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}},  // Número 3
    {{1,0,0,0,1}, {1,0,0,0,1}, {1,1,1,1,1}, {0,0,0,0,1}, {0,0,0,0,1}},  // Número 4
    {{1,1,1,1,1}, {1,0,0,0,0}, {1,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}},  // Número 5
    {{1,1,1,1,1}, {1,0,0,0,0}, {1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}},  // Número 6
    {{1,1,1,1,1}, {0,0,0,0,1}, {0,0,0,1,0}, {0,0,1,0,0}, {0,1,0,0,0}},  // Número 7
    {{1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}},  // Número 8
    {{1,1,1,1,1}, {1,0,0,0,1}, {1,1,1,1,1}, {0,0,0,0,1}, {1,1,1,1,1}}   // Número 9
};

// Funções de IRQ para os botões
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Verifica se o botão A foi pressionado e faz a alternância do LED verde
    if (gpio == BUTTON_A_PIN) {
        if (current_time - last_button_a_time >= DEBOUNCE_DELAY) {
            led_green_state = !led_green_state;
            gpio_put(LED_GREEN_PIN, led_green_state);
            
            // Atualiza o display e envia mensagem UART
            char msg[50];
            sprintf(msg, "Botao A        LED Verde: %s", led_green_state ? "ON" : "OFF");
            update_display(msg);
            printf("%s\n", msg);
            
            last_button_a_time = current_time;
        }
    }
    // Verifica se o botão B foi pressionado e faz a alternância do LED azul
    else if (gpio == BUTTON_B_PIN) {
        if (current_time - last_button_b_time >= DEBOUNCE_DELAY) {
            led_blue_state = !led_blue_state;
            gpio_put(LED_BLUE_PIN, led_blue_state);
            
            // Atualiza o display e envia mensagem UART
            char msg[50];
            sprintf(msg, "Botao B        LED Azul: %s", led_blue_state ? "ON" : "OFF");
            update_display(msg);
            printf("%s\n", msg);
            
            last_button_b_time = current_time;
        }
    }
}

// Funções WS2812
void ws2812_init() {
    uint offset = pio_add_program(ws2812_pio, &ws2812_program);
    ws2812_program_init(ws2812_pio, ws2812_sm, offset, WS2812_PIN, 800000, false);
}

// Função para enviar um pixel para a matriz WS2812
void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(ws2812_pio, ws2812_sm, pixel_grb << 8u);
}

// Função para converter RGB para GRB, formato necessário para a matriz WS2812
uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    return (g << 16) | (r << 8) | b;
}

// Limpa todos os LEDs da matriz WS2812
void clear_leds() {
    for(int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(0);  // Apaga o LED
    }
}

// Função que exibe um número (0-9) na matriz de LEDs 5x5
void display_number(int number) {
    if (number < 0 || number > 9) return;

    // Itera sobre cada linha e coluna da matriz para desenhar o número
    for (int row = 0; row < MATRIX_HEIGHT; row++) {
        for (int col = 0; col < MATRIX_WIDTH; col++) {
            // Ajusta o índice baseado na linha (par/ímpar)
            int actual_col = (row % 2 == 0) ? col : (MATRIX_WIDTH - 1 - col);
            if (number_patterns[number][row][actual_col]) {
                put_pixel(rgb_to_grb(19, 96, 48)); // Cor para os LEDs acesos
            } else {
                put_pixel(0); // LED apagado
            }
        }
    }
    sleep_us(50);  // Delay para atualizar a matriz
}

// Funções do Display
void update_display(const char *text) {
    ssd1306_fill(&display, false);
    ssd1306_draw_string(&display, text, 0, 20);
    ssd1306_send_data(&display);
}

// Inicialização UART
void uart_init_custom() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

// Processamento de entrada UART
void process_uart_input() {
    if(uart_is_readable(UART_ID)) {
        char received = uart_getc(UART_ID);
        char display_text[50];
        
        // Exibe o caractere recebido no display
        sprintf(display_text, "Recebido: %c", received);
        update_display(display_text);
        
        // Se for um número, exibe na matriz de LEDs
        if(received >= '0' && received <= '9') {
            int num = received - '0';  // Converte o caractere para inteiro
            display_number(num);       // Exibe o número na matriz
        }
        
        printf("Caractere recebido: %c\n", received);
    }
}

// Configuração dos botões
void setup_buttons() {
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
    
    // Configuração das interrupções
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);
}

// Configuração dos LEDs
void setup_leds() {
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
}

// Configuração do Display
void setup_display() {
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_init(&display, 128, 64, false, 0x3C, I2C_PORT);
    ssd1306_fill(&display, false);
    update_display("Sistema Pronto!");
}

int main() {
    stdio_init_all();
    
    // Inicialização dos componentes
    setup_leds();
    setup_buttons();
    uart_init_custom();

    ws2812_init();
    // Teste inicial - acende todos os LEDs em sequência
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(rgb_to_grb(19, 96, 48));
        sleep_ms(50);
    }
    sleep_ms(500);
    // Apaga todos
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(0);
    }
    sleep_ms(500);

    setup_display();
    clear_leds();
    
    printf("Sistema Iniciado\n");
    
    // Loop principal
    while(true) {
        process_uart_input();  // Processa a entrada UART
        sleep_ms(10);
    }
}
