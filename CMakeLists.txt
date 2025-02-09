cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Inicializa o SDK do Raspberry Pi Pico
include(pico_sdk_import.cmake)

project(embarcatech-wls-uart-i2c C CXX ASM)

pico_sdk_init()

# Adiciona o executável
add_executable(embarcatech-wls-uart-i2c
    embarcatech-wls-uart-i2c.c
    inc/ssd1306.c
)

# Gera o cabeçalho PIO
pico_generate_pio_header(embarcatech-wls-uart-i2c ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Configura o nome do programa e a versão
pico_set_program_name(embarcatech-wls-uart-i2c "embarcatech-wls-uart-i2c")
pico_set_program_version(embarcatech-wls-uart-i2c "0.1")

# Habilita saída UART e USB
pico_enable_stdio_uart(embarcatech-wls-uart-i2c 1)
pico_enable_stdio_usb(embarcatech-wls-uart-i2c 1)

# Adiciona bibliotecas necessárias
target_link_libraries(embarcatech-wls-uart-i2c
    pico_stdlib
    hardware_gpio
    hardware_clocks
    hardware_i2c
)

# Adiciona os diretórios de inclusão
target_include_directories(embarcatech-wls-uart-i2c PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    inc
)

# Adiciona os binários finais
pico_add_extra_outputs(embarcatech-wls-uart-i2c)
