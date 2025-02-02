# Projeto BitDogLab - Controle de LEDs e Botões

## Descrição do Projeto

Este projeto utiliza a placa **BitDogLab** para controlar uma matriz de LEDs **WS2812 (5x5)**, um **LED RGB** e dois **botões**, implementando um sistema interativo de exibição numérica e efeitos luminosos.

## Status do Projeto

O projeto está funcionando perfeitamente. Todas as funcionalidades foram implementadas com sucesso, garantindo um comportamento estável e confiável.

## Componentes Utilizados

- **Matriz 5x5 de LEDs WS2812** (endereçável) – Conectada à **GPIO 7**
- **LED RGB** – Pinos conectados às **GPIOs 11, 12 e 13**
- **Botão A** – Conectado à **GPIO 5**
- **Botão B** – Conectado à **GPIO 6**

## Funcionalidades Implementadas

1. **LED RGB**: O LED vermelho do LED RGB pisca continuamente 5 vezes por segundo.
2. **Botão A**: Incrementa o número exibido na matriz de LEDs quando pressionado.
3. **Botão B**: Decrementa o número exibido na matriz de LEDs quando pressionado.
4. **Matriz WS2812**: Exibe os números 0 a 9 com efeitos visuais:
   - **Formatação fixa**: Os números são exibidos em estilo digital (segmentos iluminados).

## Requisitos Implementados

- **Uso de Interrupções (IRQ)**: As funcionalidades dos botões são gerenciadas através de interrupções, garantindo resposta rápida e eficiente.
- **Debouncing via Software**: Implementado para evitar múltiplas leituras erradas ao pressionar os botões.
- **Controle de LEDs**: O projeto demonstra o controle de LEDs comuns e LEDs WS2812, garantindo o domínio de diferentes métodos de acionamento.

## Como Compilar

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/icarosg/BitDogLabFeatures.git
   cd BitDogLabFeatures
   ```

