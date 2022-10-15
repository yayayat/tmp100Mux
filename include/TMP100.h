#pragma once
#ifndef TMP100
#define TMP100

#include "stm32f0xx.h"
#include "swI2C.h"

#define FAST_MEASURMENT_PERIOD 5

typedef struct
{
    swI2C_t* i2cInstance;           // Экземляр структуры I2C интерфейса
    uint8_t i2cAddr;                // I2C адрес датчика
    int16_t temp;                   // Последнее считанное значение температуры
    int16_t tempEventMin;           // Минимальная температура в последовательности измерений
    int16_t tempEventMax;           // Максимальная температура в последовательности измерений
    int16_t tempEventAverage;       // Средняя температура в событии
    int16_t tempEventUpThreshold;   // Верхнее пороговое значение температуры для начала записи события
    int16_t tempEventBotThreshold;  // Нижнее пороговое значение температуры для начала записи события
    int32_t tempEventBuffer;        // Буффер для расчета средней температуры
    uint8_t tempRes : 2;            // Точность измерения температуры вне события
    uint8_t tempEventRes : 2;       // Точность измерения температуры в событии
    uint16_t measCounter;           // Счетчик измерений в событии
    uint16_t measCounterThreshold;  // Заданное количество измерений в событии
    uint32_t measFirstTime;         // Время первого измерения в событии
    uint32_t measLastTime;          // Время последнего измерения
    uint16_t measPeriod;            // Период одного измерения
    uint32_t measTotalEventPeriod;  // Общее время измерений для одного события
    uint8_t eventFlag : 1;          // Флаг события
} tempSensor_t;

int8_t tempSensor_init(tempSensor_t* sensorInstance, swI2C_t* i2cInstance);
int8_t tempSensor_setRes(tempSensor_t* sensorInstance, uint8_t resolution);
int8_t tempSensor_readTemp(tempSensor_t* sensorInstance);

#endif