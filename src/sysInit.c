#include "sysInit.h"

void rccInit() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOFEN;  //подключаем тактирование GPIO A, GPIO B, GPIO C, GPIO F
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_TIM17EN | RCC_APB2ENR_SYSCFGEN;                 //подключаем тактирование USART1, TIM17 и SYSCFG
}

void gpioInit() {
    // Настройка режимов GPIO:
    // биты не выставлены - input
    // 0-ой бит - output
    // 1-ый бит - Alternate function
    // Msk (оба бита) - АЦП

    GPIOA->MODER |= GPIO_MODER_MODER10_1 | GPIO_MODER_MODER9_1;
    GPIOA->AFR[1] |= 0x00000110;

    // PA10 - USART1_RX
    // PA9  - USART1_TX
}

void uartWrite(uint8_t d) {
    while (!(USART1->ISR & USART_ISR_TXE)) __NOP(); 
    USART1->TDR = d;
}

void tim17Init(void) {  // TIM17 8МГц частота инкремента, 1.6МГц частота обновления
    TIM17->RCR = 4;
    TIM17->DIER |= TIM_DIER_UIE;  // Включаем прерывание по обновлению счетчика
    TIM17->DIER |= TIM_CR1_CEN;   // Включаем таймер
}

void uartInit() {
    USART1->BRR = F_CPU / UART1_BAUD;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
}

void nvicInit() {
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(SysTick_IRQn, 3);
    SysTick_Config(F_CPU / 100);
}

void sysInit() {
    rccInit();
    gpioInit();
    uartInit();
    xdev_out(uartWrite);
    nvicInit();
}
