# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

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

# Gera o cabeçalho PIO na pasta "generated"
pico_generate_pio_header(embarcatech-wls-uart-i2c ${CMAKE_CURRENT_LIST_DIR}/generated/ws2812.pio)

# Configura o nome do programa e a versão
pico_set_program_name(embarcatech-wls-uart-i2c "embarcatech-wls-uart-i2c")
pico_set_program_version(embarcatech-wls-uart-i2c "0.1")

# Habilita saída UART e USB
pico_enable_stdio_uart(embarcatech-wls-uart-i2c 0)
pico_enable_stdio_usb(embarcatech-wls-uart-i2c 1)

# Adiciona bibliotecas necessárias
target_link_libraries(embarcatech-wls-uart-i2c
    pico_stdlib
    hardware_gpio
    hardware_clocks
    hardware_i2c
    hardware_pio
)

# Adiciona os diretórios de inclusão, incluindo a pasta "generated"
target_include_directories(embarcatech-wls-uart-i2c PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    inc
    ${CMAKE_CURRENT_LIST_DIR}/generated 
)

# Adiciona os binários finais
pico_add_extra_outputs(embarcatech-wls-uart-i2c)
