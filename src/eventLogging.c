#include "eventLogging.h"

#include "stm32f0xx.h"
#include "sysControl.h"
#include "xprintf.h"

#define abs(v) ((v) > 0 ? (v) : -(v))

event_t eventLog[EVENT_LOG_SIZE];  //Лог событий
uint16_t eventNumber = 0;          //номер ячейки для записи следующего события

//Записать событие
void eventLogging_log(uint8_t sensorNumber, uint16_t tempEventMin, uint16_t tempEventMax, uint16_t tempEventAverage, uint32_t time) {
    eventLog[eventNumber].sensorNumber = sensorNumber;               //Номер датчика
    eventLog[eventNumber].tempEventMin = tempEventMin >> 4;          //Минимальная температура
    eventLog[eventNumber].tempEventMax = tempEventMax >> 4;          //Максимальная температура
    eventLog[eventNumber].tempEventAverage = tempEventAverage >> 4;  //Средняя температура
    eventLog[eventNumber].time = time;                               //Время события
    eventNumber++;
    if (eventNumber > EVENT_LOG_SIZE - 1) eventNumber = 0;  //Замкнутый счетчик для кольцевого буффера
}

//Вывести лог
void eventLogging_print(uint16_t eventsToPrint) {
    uint16_t i = eventNumber;
    xprintf("Лог событий:\n");
    do {
        eventsToPrint--;
        i--;
        if (i > EVENT_LOG_SIZE - 1) i = EVENT_LOG_SIZE - 1;
        uint32_t timePassed = tick - eventLog[i].time;
        int16_t max = (int32_t)eventLog[i].tempEventMax * 100 / 256;
        int16_t min = (int32_t)eventLog[i].tempEventMin * 100 / 256;
        int16_t average = (int32_t)eventLog[i].tempEventAverage * 100 / 256;
        xprintf("Событие на %d датчике, %d.%02d cек назад\n", eventLog[i].sensorNumber, timePassed / 100, timePassed % 100);
        xprintf("Температура: Макс = %c%d.%02d, ", (max < 0 ? '-' : ' '), abs(max) / 100, abs(max) % 100);
        xprintf("Мин = %c%d.%02d, ", (min < 0 ? '-' : ' '), abs(min) / 100, abs(min) % 100);
        xprintf("Сред = %c%d.%02d\n\n", (average < 0 ? '-' : ' '), abs(average) / 100, abs(average) % 100);
    } while (eventsToPrint && eventLog[i].time && i != (eventNumber));
}