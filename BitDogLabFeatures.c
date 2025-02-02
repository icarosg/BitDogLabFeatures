#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "ws2818b.pio.h"

// macros
#define LED_COUNT 25
#define LED_PIN 7
#define DEBOUNCE_DELAY_MS 200

#define LED_R 13
#define LED_G 11
#define LED_B 12

#define LED_BRIGHTNESS 50  // define a intensidade dos leds

// protótipos
void init_hardware(void);
void init_leds(void);
void set_led(int index, uint8_t r, uint8_t g, uint8_t b);
void clear_leds(void);
void write_leds(void);
void exibirNumero(int countBotao);
static void gpio_irq_handler(uint gpio, uint32_t events);

// variáveis globais
int countBotao = 0;
static volatile uint32_t last_time = 0; // armazena o tempo do último evento (em microssegundos)

// se for 1, deve acender
int numerosMatriz[10][25] = {
    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0}, // 0

    {0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0}, // 1

    {0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0}, // 2

    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0}, // 3

    {0, 1, 0, 0, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 0, 1, 0}, // 4

    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0}, // 5

    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 0, 0,
     0, 1, 1, 1, 0}, // 6

    {0, 0, 0, 1, 0,
     0, 0, 1, 0, 0,
     0, 1, 0, 0, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 0, 0}, // 7

    {0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0}, // 8

    {0, 1, 1, 1, 0,
     0, 0, 0, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 0, 1, 0,
     0, 1, 1, 1, 0} // 9
};

PIO np_pio;
uint sm;
struct pixel_t
{
  uint8_t G, R, B;
};
typedef struct pixel_t npLED_t;
npLED_t leds[LED_COUNT];

// variável para controlar o debounce
absolute_time_t last_press_time;

void init_hardware(void) {
  // configura botao A na GPIO 5 com pull-up e interrupção na borda de descida
  gpio_init(5);
  gpio_set_dir(5, GPIO_IN);
  gpio_pull_up(5);
  gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);

  // configura botão B na GPIO 6 com pull-up e interrupção na borda de descida
  gpio_init(6);
  gpio_set_dir(6, GPIO_IN);
  gpio_pull_up(6);
  gpio_set_irq_enabled(6, GPIO_IRQ_EDGE_FALL, true);

  gpio_init(LED_R);              // inicializa LED_R como saída
  gpio_set_dir(LED_R, GPIO_OUT); // configura LED_R como saída

  gpio_init(LED_G);              // inicializa LED_G como saída
  gpio_set_dir(LED_G, GPIO_OUT); // configura LED_G como saída

  gpio_init(LED_B);              // inicializa LED_B como saída
  gpio_set_dir(LED_B, GPIO_OUT); // configura LED_B como saída

  init_leds(); // inicializa os LEDs
  clear_leds();
  write_leds();
  last_press_time = get_absolute_time(); // inicializa o tempo do último botão pressionado
}

void init_leds(void) {
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, true);
  ws2818b_program_init(np_pio, sm, offset, LED_PIN, 800000.f);
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i].R = leds[i].G = leds[i].B = 0;
  }
}

void set_led(int index, uint8_t r, uint8_t g, uint8_t b) {
  if (index < LED_COUNT)
  {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
}

void clear_leds(void) {
  for (int i = 0; i < LED_COUNT; i++)
  {
    set_led(i, 0, 0, 0);
  }
}

void write_leds(void) {
  for (int i = 0; i < LED_COUNT; i++)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
}

void exibirNumero(int countBotao) {
  clear_leds();
  for (int incremento = 0; incremento < 25; incremento++)
  {
    if (numerosMatriz[countBotao][incremento] == 1)
    {
      set_led(incremento, LED_BRIGHTNESS, LED_BRIGHTNESS / 2, LED_BRIGHTNESS / 3);
    }
  }
  write_leds();
}

/** 
 * Função de interrupção para tratar os botões A (GPIO 5) e B (GPIO 6)
 * Implementa debouncing software usando a constante DEBOUNCE_DELAY_MS.
 */
static void gpio_irq_handler(uint gpio, uint32_t events) {
  uint32_t current_time = to_us_since_boot(get_absolute_time());
  if (current_time - last_time < (DEBOUNCE_DELAY_MS * 1000))
    return;

  last_time = current_time;

  if (gpio == 5) { // botão A: incremento
    countBotao++;
    if (countBotao > 9)
      countBotao = 0;
    exibirNumero(countBotao);
  }

  if (gpio == 6) { // botão B: decremento
    countBotao--;
    if (countBotao < 0)
      countBotao = 0;
    exibirNumero(countBotao);
  }
}

// manipulador de interrupção para o timer
bool repeating_timer_callback(struct repeating_timer *t) {
  gpio_put(LED_R, !gpio_get(LED_R)); // alterna o estado do LED
  return true;                       // continua executando a cada intervalo definido
}

int main() {
  stdio_init_all();
  init_hardware();
  exibirNumero(countBotao);

  // timer que dispare a cada 100ms
  struct repeating_timer timer;
  add_repeating_timer_ms(-100, repeating_timer_callback, NULL, &timer);

  // loop principal (as ações dos botões são tratadas via IRQ)
  while (true) {
    sleep_ms(10); // pequeno atraso
  }
}
