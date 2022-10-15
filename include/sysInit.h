#pragma once
#ifndef SYSINIT
#define SYSINIT

#include "stm32f0xx.h"
#include "xprintf.h"

#define UART1_BAUD 500000  //Скорость UART

/// @brief Инициализация переферии
void sysInit();

/// @brief Отправить байт через UART
/// @param d байт данных
void uartWrite(uint8_t d);

#endif