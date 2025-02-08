#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

// Definições de hardware para BitDogLab
#define BUTTON_A_PIN 5  // GPIO do botão A
#define BUTTON_B_PIN 6  // GPIO do botão B
#define LED_GREEN_PIN 11 // GPIO do LED Verde
#define LED_BLUE_PIN 12 // GPIO do LED Azul
#define LED_RED_PIN 13 // GPIO do LED Vermelho
#define DEBOUNCE_US 200000 // 200ms de debounce

// Estado do LED
static volatile bool led_green_state = false;
static volatile bool led_blue_state = false;

// Função de interrupção do botão A
void button_a_irq_handler(uint gpio, uint32_t events) {
    static absolute_time_t last_interrupt_time = {0};
    absolute_time_t current_time = get_absolute_time();
    
    // Implementação do debounce
    if (absolute_time_diff_us(last_interrupt_time, current_time) < DEBOUNCE_US) {
        return;
    }
    last_interrupt_time = current_time;

    // Alternar estado do LED Verde
    led_green_state = !led_green_state;
    gpio_put(LED_GREEN_PIN, led_green_state);

    // Exibir mensagem no Serial Monitor
    printf("[BitDogLab] Botão A pressionado! LED Verde: %s\n", led_green_state ? "ON" : "OFF");
}

// Função de interrupção do botão B
void button_b_irq_handler(uint gpio, uint32_t events) {
    static absolute_time_t last_interrupt_time = {0};
    absolute_time_t current_time = get_absolute_time();
    
    // Implementação do debounce
    if (absolute_time_diff_us(last_interrupt_time, current_time) < DEBOUNCE_US) {
        return;
    }
    last_interrupt_time = current_time;

    // Alternar estado do LED Verde
    led_blue_state = !led_blue_state;
    gpio_put(LED_BLUE_PIN, led_blue_state);

    // Exibir mensagem no Serial Monitor
    printf("[BitDogLab] Botão A pressionado! LED Verde: %s\n", led_blue_state ? "ON" : "OFF");
}

void setup() {
    stdio_init_all(); // Inicializa comunicação UART
    sleep_ms(500); // Aguarda estabilização da BitDogLab
    
    // Configuração do LED Verde
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0); // Garante que o LED inicia desligado

    // Configuração do LED Azul
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, 0); // Garante que o LED inicia desligado

    // Configuração do Botão A
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN); // Confirma que o pull-up está ativo
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_a_irq_handler);

    // Configuração do Botão A
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN); // Confirma que o pull-up está ativo
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &button_b_irq_handler);
}

int main() {
    setup();
    while (1) {
        tight_loop_contents(); // Mantém o loop eficiente
    }
    return 0;
}
