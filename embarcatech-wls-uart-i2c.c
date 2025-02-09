#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "ws2812.pio.h"

// Definições de hardware para BitDogLab
#define BUTTON_A_PIN 5  // GPIO do botão A
#define BUTTON_B_PIN 6  // GPIO do botão B
#define LED_GREEN_PIN 11 // GPIO do LED Verde
#define LED_BLUE_PIN 12 // GPIO do LED Azul
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DEBOUNCE_US 200000 // 200ms de debounce
#define UART_ID uart0 // Seleciona a UART0
#define BAUD_RATE 115200 // Define a taxa de transmissão
#define UART_TX_PIN 0 // Pino GPIO usado para TX
#define UART_RX_PIN 1 // Pino GPIO usado para RX
#define WS2812_PIN 7 // Pino GPIO usado para WS2812
#define NUM_PIXELS 25 // Número total de LEDs na matriz
#define MATRIX_WIDTH 5
#define MATRIX_HEIGHT 5

// Estado dos LEDs
static volatile bool led_green_state = false;
static volatile bool led_blue_state = false;

// Estados globais para WS2812
static PIO pio = pio0;
static uint sm = 0;

// Variáveis de debounce separadas para cada botão
static absolute_time_t last_interrupt_time_a;
static absolute_time_t last_interrupt_time_b;

// Inicialização do display SSD1306
ssd1306_t display;

// Matriz de números 1-9 (5x5)
const uint8_t number_patterns[10][MATRIX_HEIGHT][MATRIX_WIDTH] = {
    // Número 1
    {
        {0, 1, 1, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0}
    },
    // Número 2
    {
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1}
    },
    // Número 3
    {
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0}
    },
    // Número 4
    {
        {1, 0, 0, 1, 0},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0}
    },
    // Número 5
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0}
    },
    // Número 6
    {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0}
    },
    // Número 7
    {
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0}
    },
    // Número 8
    {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0}
    },
    // Número 9
    {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {0, 0, 0, 0, 1},
        {1, 1, 1, 1, 0}
    }
};

void update_display_message(const char *message) {
    ssd1306_fill(&display, false); // Limpa o display
    ssd1306_draw_string(&display, "Recebido:", 0, 20);
    ssd1306_draw_string(&display, message, 0, 40);
    ssd1306_send_data(&display);
}

void update_display_status(const char *status) {
    ssd1306_fill(&display, false); // Limpa o display
    ssd1306_draw_string(&display, status, 0, 20);
    ssd1306_send_data(&display);
}

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
        printf("[BitDogLab] Botao A pressionado! LED Verde: %s\n", led_green_state ? "ON" : "OFF");
        update_display_status(led_green_state ? "Botao A        LED Verde    ON" : "Botao A        LED Verde   OFF");
    }

    if (gpio == BUTTON_B_PIN) {
        if (absolute_time_diff_us(last_interrupt_time_b, current_time) < DEBOUNCE_US) {
            return;
        }
        last_interrupt_time_b = current_time;

        // Alternar estado do LED Azul
        led_blue_state = !led_blue_state;
        gpio_put(LED_BLUE_PIN, led_blue_state);
        printf("[BitDogLab] Botao B pressionado! LED Azul: %s\n", led_blue_state ? "ON" : "OFF");
        update_display_status(led_blue_state ? "Botao B        LED Azul     ON" : "Botao B        LED Azul    OFF");
    }
}

void setup() {
    stdio_init_all(); // Inicializa comunicação UART
    sleep_ms(2000); // Garante tempo para inicialização do Serial Monitor

    // Inicializa UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Inicializa WS2812
    ws2812_init();

    // Mensagem inicial
    printf("Digite um número de 1 a 9 para exibir na matriz:\n");

    while (1) {
        // Verifica se há dados disponíveis para leitura
        if (uart_is_readable(UART_ID)) {
            // Lê um caractere da UART
            char c = uart_getc(UART_ID);

            // Verifica se é um número de 1 a 9
            if (c >= '1' && c <= '9') {
                int number = c - '0'; // Converte o caractere para número
                display_number(number); // Exibe o número na matriz
                printf("Número exibido: %d\n", number);
            } else {
                printf("Caractere inválido: %c\n", c);
            }
        }
        sleep_ms(100);
    }

    // Inicializa I2C para SSD1306
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display SSD1306
    ssd1306_init(&display, WIDTH, HEIGHT, false, 0x3C, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    update_display_status("Inicializando...");

    // Configuração dos LEDs
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0);

    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, 0);

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
        char message[2] = {received_char, '\0'}; // Converte para string

        // Exibe caracteres recebidos corretamente
        printf("[UART] Recebido: %c\n", received_char);
        update_display_message(message); // Atualiza o display com o caractere recebido
    }
}

// Função para inicializar o WS2812
void ws2812_init() {
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
}

// Função para enviar um pixel
void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Função para converter RGB em GRB
uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
}

// Função para exibir o número na matriz
void display_number(int number) {
    if (number < 1 || number > 9) return;

    for (int row = 0; row < MATRIX_HEIGHT; row++) {
        for (int col = 0; col < MATRIX_WIDTH; col++) {
            if (number_patterns[number - 1][row][col]) {
                put_pixel(rgb_to_grb(19, 96, 48));  
            } else {
                put_pixel(0);  // Apagado
            }
        }
    }
}

int main() {
    setup();
    printf("[BitDogLab] Sistema inicializado. Pressione os botoes ou envie comandos via UART.\n");
    while (1) {
        process_uart_input();
        tight_loop_contents();
    }
    return 0;
}
