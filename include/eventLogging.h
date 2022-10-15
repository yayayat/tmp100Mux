#pragma once
#ifndef EVENTLOGGING
#define EVENTLOGGING

#include "config.h"
#include "stm32f0xx.h"

typedef struct
{
    uint8_t sensorNumber;            // Номер датчика
    uint16_t tempEventMin : 12;      // Минимальная температура в событии
    uint16_t tempEventMax : 12;      // Максимальная температура в событии
    uint16_t tempEventAverage : 12;  // Средняя температура в событии
    uint32_t time;                   // Время записи о событии
} event_t;

extern event_t eventLog[EVENT_LOG_SIZE];  //Лог событий

/// @brief Сделать запись о событии
/// @param sensorNumber Номер датчика
/// @param tempEventMin Минимальная температура в событии
/// @param tempEventMax Максимальная температура в событии
/// @param tempEventAverage Средняя температура в событии
/// @param time Время записи о событии
void eventLogging_log(uint8_t sensorNumber, uint16_t tempEventMin, uint16_t tempEventMax, uint16_t tempEventAverage, uint32_t time);

/// @brief Вывести в UART лог событий
/// @param eventsToPrint Число событий для вывода (0 - вывести все)
void eventLogging_print(uint16_t eventsToPrint);
#endif