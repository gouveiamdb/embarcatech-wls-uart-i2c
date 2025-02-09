//Criado por Matheus Gouveia para a formação do EMbarcaTech
// Data: 09/02/2025
// Descrição: Programa para controle de LEDs RGB, botões, display OLED e matriz de LEDs WS2812
//            utilizando comunicação UART e I2C no Raspberry Pi Pico
//            O programa recebe um caractere via UART e exibe o número correspondente na matriz de LEDs
//            O programa também controla o estado dos LEDs RGB e do display OLED através dos botões A e B

// Inclusão das bibliotecas necessárias
#include <stdio.h>              // Biblioteca padrão de I/O
#include "pico/stdlib.h"        // Biblioteca principal do Raspberry Pi Pico
#include "hardware/gpio.h"      // Controle de GPIO
#include "hardware/uart.h"      // Comunicação UART
#include "hardware/i2c.h"       // Comunicação I2C
#include "hardware/irq.h"       // Tratamento de interrupções
#include "inc/ssd1306.h"        // Controle do display OLED
#include "inc/font.h"           // Fonte para o display OLED
#include "ws2812.pio.h"         // Controle dos LEDs WS2812

// Definições dos pinos e parâmetros de hardware
#define BUTTON_A_PIN 5          // GPIO do Botão A
#define BUTTON_B_PIN 6          // GPIO do Botão B
#define LED_GREEN_PIN 11        // GPIO do LED Verde do RGB
#define LED_BLUE_PIN 12         // GPIO do LED Azul do RGB
#define LED_RED_PIN 13          // GPIO do LED Vermelho do RGB
#define I2C_PORT i2c1           // Porta I2C utilizada
#define I2C_SDA 14              // GPIO do SDA (I2C)
#define I2C_SCL 15              // GPIO do SCL (I2C)
#define UART_ID uart0           // ID da UART utilizada
#define BAUD_RATE 115200        // Taxa de transmissão UART
#define UART_TX_PIN 16          // GPIO do TX (UART)
#define UART_RX_PIN 17          // GPIO do RX (UART)
#define WS2812_PIN 7            // GPIO para controle da matriz WS2812
#define NUM_PIXELS 25           // Total de LEDs na matriz (5x5)
#define MATRIX_SIZE 5           // Tamanho da matriz
#define DEBOUNCE_DELAY 200      // Tempo de debounce em ms
#define MATRIX_WIDTH 5          // Largura da matriz
#define MATRIX_HEIGHT 5         // Altura da matriz
#define ENDERECO 0x3C           // Endereço I2C do display OLED

// Variáveis globais e estados
static PIO ws2812_pio = pio0;    // Controlador PIO para WS2812
static uint ws2812_sm = 0;       // Máquina de estado do PIO
ssd1306_t display;               // Estrutura para controle do display
volatile bool led_green_state = false;   // Estado do LED verde
volatile bool led_blue_state = false;    // Estado do LED azul
// Timestamps para debounce dos botões
volatile uint32_t last_button_a_time = 0;
volatile uint32_t last_button_b_time = 0;

// Padrões dos números na matriz 5x5
// Cada número é representado por uma matriz 5x5 onde:
// 1 = LED aceso, 0 = LED apagado
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

// Callback de interrupção para os botões
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (gpio == BUTTON_A_PIN) {
        // Verifica se passou o tempo de debounce
        if (current_time - last_button_a_time >= DEBOUNCE_DELAY) {
            led_green_state = !led_green_state; // Inverte estado do LED
            gpio_put(LED_GREEN_PIN, led_green_state);
            
            // Atualiza display e envia mensagem via UART
            char msg[50];
            sprintf(msg, "Botao A        LED Verde: %s", led_green_state ? "ON" : "OFF");
            update_display(&display, msg);
            printf("%s\n", msg);
            
            last_button_a_time = current_time;
        }
    } else if (gpio == BUTTON_B_PIN) {
        // Verifica se passou o tempo de debounce
        if (current_time - last_button_b_time >= DEBOUNCE_DELAY) {
            led_blue_state = !led_blue_state; // Inverte estado do LED
            gpio_put(LED_BLUE_PIN, led_blue_state);
            
            // Atualiza display e envia mensagem via UART
            char msg[50];
            sprintf(msg, "Botao B        LED Azul: %s", led_blue_state ? "ON" : "OFF");
            update_display(&display, msg);
            printf("%s\n", msg);
            
            last_button_b_time = current_time;
        }
    }
}

// Funções de controle da matriz WS2812
void ws2812_init() {
    // Inicializa o controlador PIO para os LEDs WS2812
    uint offset = pio_add_program(ws2812_pio, &ws2812_program);
    ws2812_program_init(ws2812_pio, ws2812_sm, offset, WS2812_PIN, 800000, false);
}

void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(ws2812_pio, ws2812_sm, pixel_grb << 8u);
}

uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    return (g << 16) | (r << 8) | b;
}

void clear_leds() {
    for(int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(0);  // Apaga o LED
    }
}

void display_number(uint8_t number) {
    if (number > 9) return;

    // Cores para os LEDs acesos e apagados
    uint32_t on_color = rgb_to_grb(3, 10, 32);  // Azul suave
    uint32_t off_color = rgb_to_grb(0, 0, 0);    // Apagado
    
    // Buffer para armazenar o estado de todos os LEDs
    uint32_t led_buffer[NUM_PIXELS];
    
    // Preenche o buffer com os estados corretos
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            // Em linhas pares, a ordem é da esquerda para a direita
            // Em linhas ímpares, a ordem é da direita para a esquerda
            int x_pos = (y % 2 == 0) ? x : (MATRIX_WIDTH - 1 - x);
            int led_index = y * MATRIX_WIDTH + x_pos;
            
            // Armazena o estado no buffer
            led_buffer[led_index] = number_patterns[number][MATRIX_HEIGHT - 1 - y][MATRIX_WIDTH - 1 - x] ? on_color : off_color;
            
            // Debug para ver a ordem dos pixels
            printf("LED[%d,%d] = %d (index=%d)\n", x, y, number_patterns[number][y][x], led_index);
        }
    }
    
    // Agora envia todos os estados para a matriz
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(led_buffer[i]);
    }
    
    // Imprime o padrão completo para debug
    printf("\nO número %d foi digitado com sussesso!\n", number);
}

void process_uart_input() {
    if (stdio_usb_connected()) {
        char c;
        if (scanf("%c", &c) == 1) {
            printf("Recebido - Char: '%c' | Dec: %d | Hex: 0x%02X\n", c, (uint8_t)c, (uint8_t)c);
            
            if (c >= '0' && c <= '9') {
                uint8_t numero = c - '0';
                printf("Exibindo número: %d\n", numero);
                
                // Limpa todos os LEDs antes de mostrar um novo número
                clear_leds();
                
                // Exibe o número na matriz
                display_number(numero);
                
                printf("Número exibido!\n");
            }

            // Adiciona caso especial para o caractere '*', apaga a matriz de LEDs
            else if (c == '*') {
                printf("Limpando matriz de LEDs...\n");
                clear_leds();
                printf("Matriz limpa!\n");
            }
            
            char mensagem[2] = { (char)c, '\0' };
            update_display(&display, mensagem);
        }
    }
    sleep_ms(100);
}


void update_display(ssd1306_t *display, const char *text) {
    ssd1306_fill(display, false);
    ssd1306_draw_string(display, text, 10, 25);
    ssd1306_send_data(display);
}

void uart_init_custom() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void setup_buttons() {
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
    
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);
}

void setup_leds() {
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
}

void setup_display() {
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_init(&display, 128, 64, false, ENDERECO, I2C_PORT);
    ssd1306_fill(&display, false);
    update_display(&display, "Sistema Pronto!");
}

int main() {
    stdio_init_all();
    
    setup_leds();           
    setup_buttons();        
    uart_init_custom();

    ws2812_init();

    setup_display();
    clear_leds();
    
    printf("Sistema Iniciado\n");
    
    // Loop principal
    while(true) {
        process_uart_input();  // Processa a entrada UART
        sleep_ms(10);
    }
}
