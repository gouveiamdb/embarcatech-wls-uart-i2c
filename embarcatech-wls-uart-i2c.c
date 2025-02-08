#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"

// Definições de hardware para BitDogLab
#define BUTTON_A_PIN 5  // GPIO do botão A
#define BUTTON_B_PIN 6  // GPIO do botão B
#define LED_GREEN_PIN 11 // GPIO do LED Verde
#define LED_BLUE_PIN 12 // GPIO do LED Azul
#define LED_RED_PIN 13 // GPIO do LED Vermelho
#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define BAUD_RATE 115200
#define DEBOUNCE_US 200000 // 50ms de debounce

// Estado dos LEDs
static volatile bool led_green_state = false;
static volatile bool led_blue_state = false;
static volatile bool led_red_state = false;

// Variáveis de debounce separadas para cada botão
static absolute_time_t last_interrupt_time_a;
static absolute_time_t last_interrupt_time_b;

// Callback global para lidar com interrupções de GPIO
void gpio_irq_handler(uint gpio, uint32_t events) {
    absolute_time_t current_time = get_absolute_time();
    
    if (gpio == BUTTON_A_PIN) {
        if (absolute_time_diff_us(last_interrupt_time_a, current_time) < DEBOUNCE_US) {
            return;
        }
        last_interrupt_time_a = current_time;
        
        // Alternar estado do LED Verde
        led_green_state = !led_green_state;
        gpio_put(LED_GREEN_PIN, led_green_state);
        printf("[BitDogLab] Botão A pressionado! LED Verde: %s\n", led_green_state ? "ON" : "OFF");
    }
    
    if (gpio == BUTTON_B_PIN) {
        if (absolute_time_diff_us(last_interrupt_time_b, current_time) < DEBOUNCE_US) {
            return;
        }
        last_interrupt_time_b = current_time;
        
        // Alternar estado do LED Azul
        led_blue_state = !led_blue_state;
        gpio_put(LED_BLUE_PIN, led_blue_state);
        printf("[BitDogLab] Botão B pressionado! LED Azul: %s\n", led_blue_state ? "ON" : "OFF");
    }
}

void setup() {
    stdio_init_all(); // Inicializa comunicação UART
    sleep_ms(2000); // Garante tempo para inicialização do Serial Monitor
    
    // Inicializa UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Configuração dos LEDs
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0);

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, 0);
    
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, 0);

    // Configuração dos botões
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    
    // Registra interrupção global
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);
}

void process_uart_input() {
    if (uart_is_readable(UART_ID)) {
        char received_char = uart_getc(UART_ID);
        printf("[UART] Recebido: %c\n", received_char);
    }
}

int main() {
    setup();
    printf("[BitDogLab] Sistema inicializado. Pressione os botões ou envie comandos via UART.\n");
    while (1) {
        process_uart_input();
        tight_loop_contents();
    }
    return 0;
}
