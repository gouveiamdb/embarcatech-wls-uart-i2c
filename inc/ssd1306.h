// ssd1306.h - Definições do driver SSD1306

#ifndef SSD1306_H
#define SSD1306_H

#include "hardware/i2c.h"
#include <stdbool.h>
#include <stdint.h>

// Estrutura para representar o display SSD1306
typedef struct {
    i2c_inst_t *i2c;     // Instância do I2C
    uint8_t address;     // Endereço I2C do display
    uint8_t width;       // Largura do display em pixels
    uint8_t height;      // Altura do display em pixels
    uint8_t buffer[1024]; // Buffer para o conteúdo do display (128x64)
} ssd1306_t;

// Inicializa o display SSD1306
void ssd1306_init(ssd1306_t *display, i2c_inst_t *i2c, uint8_t address, uint8_t width, uint8_t height);

// Envia um comando único ao display
void ssd1306_send_command(ssd1306_t *display, uint8_t command);

// Envia uma sequência de comandos ao display
void ssd1306_send_command_sequence(ssd1306_t *display, const uint8_t *commands, size_t length);

// Limpa o conteúdo do buffer do display
void ssd1306_clear(ssd1306_t *display);

// Mostra o conteúdo do buffer no display físico
void ssd1306_show(ssd1306_t *display);

// Desenha um pixel no buffer do display
void ssd1306_draw_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool color);

// Desenha uma string no buffer do display
void ssd1306_draw_string(ssd1306_t *display, uint8_t x, uint8_t y, const char *str, uint8_t size, bool color);

// Desenha um caractere no buffer do display
void ssd1306_draw_char(ssd1306_t *display, uint8_t x, uint8_t y, char chr, uint8_t size, bool color);

#endif // SSD1306_H
