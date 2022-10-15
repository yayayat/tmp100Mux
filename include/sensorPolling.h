#pragma once
#ifndef SENSORPOLLING
#define SENSORPOLLING
#include "TMP100.h"
#include "config.h"
#include "stm32f0xx.h"

extern tempSensor_t tempSensors[NUM_OF_SENSORS];  //экземпляры структур датчиков

/// @brief обработчик чтения с датчиков
void sensorPolling_handler();

/// @brief иничиализация датчиков
void sensorPolling_init();

#endif