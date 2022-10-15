#define SENSORPOLLING_C
#include "sensorPolling.h"

#include "eventLogging.h"
#include "stm32f0xx.h"
#include "sysControl.h"

tempSensor_t tempSensors[NUM_OF_SENSORS];  //Массив подключенных датчиков

void sensorPolling_init() {
    for (uint8_t i = 0; i < NUM_OF_SENSORS; i++)             //Перебираем все датчики
        tempSensor_init(&tempSensors[i], &i2cInstances[i]);  //Инициализация датчиков, передача структур I2C интерфейса для каждого
}

void sensorPolling_handler() {
    static uint8_t i = 0xFF;                                               // Номер текущего датчика
    i = (i + 1) % NUM_OF_SENSORS;                                          // Замкунутый счетчик
    if (tick - tempSensors[i].measLastTime > tempSensors[i].measPeriod) {  // Период нового измерения
        if (tempSensor_readTemp(&tempSensors[i]))                          // Считываем измеренную температуру
            return;                                                        // Ошибка кадра при чтении, пропускаем

        tempSensors[i].measCounter++;                                                 // Счетчик числа измерений
        if (tempSensors[i].temp > tempSensors[i].tempEventUpThreshold ||              // Если температура вышла
            tempSensors[i].temp < tempSensors[i].tempEventBotThreshold) {             // за заданные пределы
            if (tempSensor_setRes(&tempSensors[i], tempSensors[i].tempEventRes))      // Выставляем точность измерения во время события
                return;                                                               // Ошибка кадра при записи, пропускаем
            tempSensors[i].eventFlag = 1;                                             // Устанавливаем флаг события
            tempSensors[i].measCounter = 0;                                           // Сбрасываем счетчик измерений
            tempSensors[i].tempEventMax = 0;                                          // Сбрасываем максимальную зафиксированную температуру
            tempSensors[i].tempEventMin = 0;                                          // Сбрасываем минимальную зафиксированную температуру
            tempSensors[i].measFirstTime = (uint16_t)tick;                            // Записываем время первого измерения в событии
        }                                                                             //
        if (tempSensors[i].eventFlag) {                                               // Если происходит событие, то
            tempSensors[i].tempEventBuffer += tempSensors[i].temp;                    // Заполняем буффер для рассчета средней температуры
            if (tempSensors[i].temp > tempSensors[i].tempEventMax)                    // Если текущая температура выше зафиксированной максимальной
                tempSensors[i].tempEventMax = tempSensors[i].temp;                    // записываем новое значение
            else if (tempSensors[i].temp < tempSensors[i].tempEventMin)               // Если текущая температура ниже зафиксированной минимальной
                tempSensors[i].tempEventMin = tempSensors[i].temp;                    // записываем новое значение
            if ((tempSensors[i].measLastTime - tempSensors[i].measFirstTime) >        // Если вышло время измерений
                    tempSensors[i].measTotalEventPeriod ||                            //
                tempSensors[i].measCounter >                                          // или превышено число измерений
                    tempSensors[i].measCounterThreshold) {                            //
                if (tempSensor_setRes(&tempSensors[i], tempSensors[i].tempEventRes))  // Выставляем точность измерения вне события
                    return;                                                           // Ошибка кадра при записи, пропускаем
                tempSensors[i].eventFlag = 0;                                         // Сбрасываем флаг события
                tempSensors[i].tempEventAverage =                                     // Рассчитываем среднее значение температуры
                    (tempSensors[i].tempEventBuffer / tempSensors[i].measCounter);    //
                eventLogging_log(i,                                                   //
                                 tempSensors[i].tempEventMin,                         //
                                 tempSensors[i].tempEventMax,                         //
                                 tempSensors[i].tempEventAverage,                     //
                                 tick);                                               // Запись события в лог
            }
        }
        tempSensors[i].measLastTime = tick;  //Запоминаем время последнего успешного вызова
    }
}
