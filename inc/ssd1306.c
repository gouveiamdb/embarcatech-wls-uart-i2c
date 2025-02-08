// ssd1306.c - Implementação do driver SSD1306

#include "ssd1306.h"
#include <string.h>

void ssd1306_init(ssd1306_t *display, i2c_inst_t *i2c, uint8_t address, uint8_t width, uint8_t height) {
    display->i2c = i2c;
    display->address = address;
    display->width = width;
    display->height = height;

    uint8_t init_sequence[] = {
        0xAE,       // Display OFF
        0x20, 0x00, // Memory Addressing Mode: Horizontal
        0xB0,       // Page Start Address for Page Addressing Mode
        0xC8,       // COM Output Scan Direction: remapped mode
        0x00,       // Set Lower Column Start Address
        0x10,       // Set Higher Column Start Address
        0x40,       // Set Display Start Line
        0x81, 0xFF, // Set Contrast Control
        0xA1,       // Segment Re-map: column address 127 is mapped to SEG0
        0xA6,       // Normal Display (A7 for Inverse Display)
        0xA8, 0x3F, // Multiplex Ratio: 1/64 Duty
        0xA4,       // Entire Display ON (A5 for All ON)
        0xD3, 0x00, // Display Offset
        0xD5, 0x80, // Display Clock Divide Ratio/Oscillator Frequency
        0xD9, 0xF1, // Pre-charge Period
        0xDA, 0x12, // COM Pins Hardware Configuration
        0xDB, 0x20, // VCOMH Deselect Level
        0x8D, 0x14, // Enable Charge Pump
        0xAF        // Display ON
    };

    ssd1306_send_command_sequence(display, init_sequence, sizeof(init_sequence));
    ssd1306_clear(display);
    ssd1306_show(display);
}

void ssd1306_send_command(ssd1306_t *display, uint8_t command) {
    uint8_t buffer[2] = {0x00, command};
    i2c_write_blocking(display->i2c, display->address, buffer, 2, false);
}

void ssd1306_send_command_sequence(ssd1306_t *display, const uint8_t *commands, size_t length) {
    for (size_t i = 0; i < length; i++) {
        ssd1306_send_command(display, commands[i]);
    }
}

void ssd1306_clear(ssd1306_t *display) {
    memset(display->buffer, 0, sizeof(display->buffer));
}

void ssd1306_draw_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool color) {
    if (x >= display->width || y >= display->height) {
        return; // Fora da área visível
    }

    if (color) {
        display->buffer[x + (y / 8) * display->width] |= (1 << (y % 8));
    } else {
        display->buffer[x + (y / 8) * display->width] &= ~(1 << (y % 8));
    }
}

void ssd1306_draw_string(ssd1306_t *display, uint8_t x, uint8_t y, const char *str, uint8_t size, bool color) {
    while (*str) {
        ssd1306_draw_char(display, x, y, *str, size, color);
        x += size * 6; // Avança 6 pixels por caractere
        str++;
    }
}

void ssd1306_draw_char(ssd1306_t *display, uint8_t x, uint8_t y, char chr, uint8_t size, bool color) {
    if (chr < 32 || chr > 127) {
        return; // Caracteres fora do intervalo ASCII
    }

    for (uint8_t i = 0; i < 5; i++) { // Cada caractere tem 5 colunas
        uint8_t line = font[chr - 32][i];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                ssd1306_draw_pixel(display, x + i * size, y + j * size, color);
            }
        }
    }
}

void ssd1306_show(ssd1306_t *display) {
    for (uint8_t page = 0; page < display->height / 8; page++) {
        ssd1306_send_command(display, 0xB0 + page);
        ssd1306_send_command(display, 0x00);
        ssd1306_send_command(display, 0x10);
        i2c_write_blocking(display->i2c, display->address, &display->buffer[page * display->width], display->width, false);
    }
}
