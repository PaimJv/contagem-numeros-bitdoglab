#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// Definições de hardware
#define LED_RGB_G 11
#define LED_RGB_B 12
#define LED_RGB_R 13
#define BUTTON_A 5
#define BUTTON_B 6
#define WS2812_PIN 7
#define NUM_PIXELS 25
#define IS_RGBW false

// Intervalo de tempo
const int intervalo_ms = 400;
const int intervalo_micro = intervalo_ms * 1000;

// Variáveis globais
volatile int numero_atual = 0;
volatile uint32_t ultimo_estado_a = 0;
volatile uint32_t ultimo_estado_b = 0;

// Buffer para números na matriz (0 a 9)
bool numeros[10][NUM_PIXELS] = {
    { // Zero
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Um
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0
    },
    { // Dois
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Três
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Quatro
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0
    },
    { // Cinco
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // Seis
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // Sete
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Oito
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Nove
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    }
};

// Funções auxiliares
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

void display_numeros(int numero) {
    uint32_t color = urgb_u32(100, 0, 0);
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(numeros[numero][i] ? color : 0);
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A && current_time - ultimo_estado_a > intervalo_micro) {
        ultimo_estado_a = current_time;
        numero_atual = (numero_atual + 1) % 10;
        display_numeros(numero_atual);
    }
    if (gpio == BUTTON_B && current_time - ultimo_estado_b > intervalo_micro) {
        ultimo_estado_b = current_time;
        numero_atual = (numero_atual - 1 + 10) % 10;
        display_numeros(numero_atual);
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_RGB_R); gpio_set_dir(LED_RGB_R, GPIO_OUT);
    gpio_init(LED_RGB_G); gpio_set_dir(LED_RGB_G, GPIO_OUT);
    gpio_init(LED_RGB_B); gpio_set_dir(LED_RGB_B, GPIO_OUT);
    gpio_init(BUTTON_A); gpio_set_dir(BUTTON_A, GPIO_IN); gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B); gpio_set_dir(BUTTON_B, GPIO_IN); gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    
    display_numeros(numero_atual);

    while (1) {
        gpio_put(LED_RGB_R, 1);
        sleep_ms(100);
        gpio_put(LED_RGB_R, 0);
        sleep_ms(100);
    }
    return 0;
}
