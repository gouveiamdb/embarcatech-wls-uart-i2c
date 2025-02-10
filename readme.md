# 🚦 Controle de LEDs RGB, Display OLED e Matriz WS2812

## 📚 Descrição do Projeto
Este projeto implementa o controle de **LEDs RGB**, **Display OLED (SSD1306)** e **Matriz de LEDs WS2812** utilizando a placa **RP2040**. A comunicação é realizada via **UART** para controle dos LEDs RGB e via **I2C** para o display OLED. A matriz WS2812 exibe números de 0 a 9, e os LEDs RGB podem ser controlados com os botões A e B.

### Funcionalidades:
- **UART**: Recebe um caractere via comunicação serial e exibe números na matriz WS2812 e no display OLED.
- **Display OLED (SSD1306)**: Exibe caracteres recebidos através da UART.
- **Matriz de LEDs WS2812**: Exibe números de 0 a 9 em uma matriz 5x5 de LEDs.
- **Botões**: Controle dos LEDs RGB (verde e azul) através de botões físicos (Botão A e Botão B).

---

## 🎯 Funcionalidades Implementadas

1. **Leitura via UART**  
   O código lê um caractere recebido via UART (serial monitor) e exibe:  
   - Números de 0 a 9 na matriz WS2812.  
   - O caractere no display OLED SSD1306. 
   - O caractere '*' limpa a matriz WS2812 e o display OLED SSD1306.

2. **Controle dos LEDs RGB**  
   O estado dos LEDs RGB pode ser alterado pressionando os botões A e B.  

3. **Exibição na Matriz WS2812**  
   A matriz de LEDs exibe números de 0 a 9 de acordo com o padrão `number_patterns`.

---

## 🛠️ Componentes Utilizados

### **Hardware**
- **LED Vermelho** - GPIO 13  
- **LED Verde** - GPIO 11  
- **LED Azul** - GPIO 12  
- **Display OLED (SSD1306)**  
  - **SDA** - GPIO 14  
  - **SCL** - GPIO 15  
- **Matriz WS2812**  
  - **Dados** - GPIO 7  

### **Software**
- **RP2040 SDK**  
- **Linguagem C**  
- **Bibliotecas**:  
  - `hardware/uart.h` para comunicação UART  
  - `hardware/i2c.h` para comunicação I2C  
  - `ssd1306.h` para controle do display OLED  
  - `ws2812.pio.h` para controle da matriz WS2812  

---

## ⚙️ Diagrama de Conexões

| Componente        | GPIO  | Função     |
|-------------------|-------|------------|
| **Botão A**       | 5     | Controle LED Verde   |
| **Botão B**       | 6     | Controle LED Azul    |
| **LED Vermelho**  | 13    | Controle LED Vermelho |
| **LED Verde**     | 11    | Controle LED Verde   |
| **LED Azul**      | 12    | Controle LED Azul    |
| **Display OLED (SSD1306)** | SDA: 14, SCL: 15 | I2C Comunicação |
| **Matriz WS2812** | 7     | Dados dos LEDs       |

---

## 📂 Organização do Código

- **`embarcatech-wls-uart-i2c.c`**  
  - Configuração dos GPIOs dos LEDs e display.  
  - Implementação do controle de LEDs RGB via botões e UART.  
  - Controle da matriz WS2812 para exibição dos números.  
  - Loop principal para comunicação serial e atualização dos LEDs e display.

---

## 🛠️ Instruções de Compilação e Execução

1. Clone o repositório para o ambiente local.
2. Certifique-se de que o **RP2040 SDK** está configurado corretamente.
3. Compile o código utilizando o **CMake**.
4. Carregue o firmware na placa **RP2040**.
5. Conecte a placa ao computador e abra o monitor serial para interação via UART.
6. Observe os LEDs e o display OLED alternando conforme os caracteres recebidos.

---
## 📹 Demonstração do Projeto

### Vídeo de Demonstração
- O vídeo de demonstração exibe:
  - Funcionamento do código.
  - Explicação das funcionalidades implementadas.
   [Video Demonstrativo](https://drive.google.com/file/d/10FSCGffz7tmVasIFiXSE_Ti72GdbzqDM/view?usp=sharing)
---



## 💻 Autor
- **Matheus Gouveia de Deus Bastos**

---

## 📜 Licença
Este projeto é de uso acadêmico e segue as diretrizes da Embarcatech.
