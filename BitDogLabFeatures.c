#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "pico/bootrom.h"

// Biblioteca gerada pelo arquivo .pio durante a compilação.
#include "ws2818b.pio.h"

// macros
#define LED_COUNT 25
#define LED_PIN 7
#define DEBOUNCE_DELAY_MS 200

#define LED_R 13
#define LED_G 11
#define LED_B 12

// prototypes
void init_hardware(void);
void init_leds(void);
void set_led(int index, uint8_t r, uint8_t g, uint8_t b);
void clear_leds(void);
void write_leds(void);
void exibirNumero(int countBotao);
static void gpio_irq_handler(uint gpio, uint32_t events);

// global variables
int countBotao = 0;
const uint8_t COL_PINS[] = {19, 18, 17, 16};
const uint8_t ROW_PINS[] = {28, 27, 26, 20};
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

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

// Variável para controlar o debounce
absolute_time_t last_press_time;

void init_hardware(void)
{
  gpio_init(5);             // Inicializa GPIO 5 como entrada
  gpio_set_dir(5, GPIO_IN); // Configura GPIO 5 como entrada
  gpio_pull_up(5);          // Ativa pull-up interno no GPIO 5

  gpio_init(6);             // Inicializa GPIO 6 como entrada
  gpio_set_dir(6, GPIO_IN); // Configura GPIO 6 como entrada
  gpio_pull_up(6);          // Ativa pull-up interno no GPIO 6

  gpio_init(LED_R);             // Inicializa LED_R como saída
  gpio_set_dir(LED_R, GPIO_OUT); // Configura LED_R como saída

  gpio_init(LED_G);             // Inicializa LED_G como saída
  gpio_set_dir(LED_G, GPIO_OUT); // Configura LED_G como saída

  gpio_init(LED_B);             // Inicializa LED_B como saída
  gpio_set_dir(LED_B, GPIO_OUT); // Configura LED_B como saída

  init_leds(); // Inicializa os LEDs
  clear_leds();
  write_leds();
  last_press_time = get_absolute_time(); // Inicializa o tempo do último botão pressionado
}

void init_leds(void)
{
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, true);
  ws2818b_program_init(np_pio, sm, offset, LED_PIN, 800000.f);
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds[i].R = leds[i].G = leds[i].B = 0;
  }
}

void set_led(int index, uint8_t r, uint8_t g, uint8_t b)
{
  if (index < LED_COUNT)
  {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
}

void clear_leds(void)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    set_led(i, 0, 0, 0);
  }
}

void write_leds(void)
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
}

void exibirNumero(int countBotao)
{
  clear_leds();
  for (int incremento = 0; incremento < 25; incremento++)
  {
    if (numerosMatriz[countBotao][incremento] == 1)
    {
      set_led(incremento, 255, 255 / 2, 255 / 3);
    }
  }
  write_leds();
}

// manipulador de interrupção para o timer
bool repeating_timer_callback(struct repeating_timer *t) {
    gpio_put(LED_R, !gpio_get(LED_R)); // alterna o estado do LED
    return true; // continua executando a cada intervalo definido
}

int main()
{
  stdio_init_all();
  init_hardware();
  exibirNumero(countBotao);

  // timer que dispare a cada 100ms
  struct repeating_timer timer;
  add_repeating_timer_ms(-100, repeating_timer_callback, NULL, &timer);

  while (true)
  {
    if (!gpio_get(5)) // verifica se o botão A foi pressionado
    {
      // obtém o tempo atual em microssegundos
      uint32_t current_time = to_us_since_boot(get_absolute_time());
      // verifica se passou tempo suficiente desde o último evento
      if (current_time - last_time > 200000) // 200 ms de debouncing
      {
        last_time = current_time; // atualiza o tempo do último evento
        countBotao += 1;          // incrementa o botão
        if (countBotao > 9)
        {
          countBotao = 0; // reinicia para 0 após 9
        }
        exibirNumero(countBotao);
      }
    }

    if (!gpio_get(6)) // verifica se o botão B foi pressionado
    {
      // obtém o tempo atual em microssegundos
      uint32_t current_time = to_us_since_boot(get_absolute_time());
      // verifica se passou tempo suficiente desde o último evento
      if (current_time - last_time > 200000) // 200 ms de debouncing
      {
        last_time = current_time; // atualiza o tempo do último evento
        countBotao -= 1;          // decrementa o botão
        if (countBotao < 0)
        {
          countBotao = 0; // reinicia para 0 se menor que 0
        }
        exibirNumero(countBotao);
      }
    }

    sleep_ms(10); // pequeno atraso
  }
}
