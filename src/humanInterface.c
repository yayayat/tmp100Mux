#include "humanInterface.h"

#include "config.h"
#include "eventLogging.h"
#include "sensorPolling.h"
#include "sysInit.h"

#define abs(v) ((v) > 0 ? (v) : -(v))
#define constrain(v, min, max) ((v) > (max) ? (max) : ((v) < (min) ? (min) : (v)))

uint8_t rxBuf[128];
uint8_t rxBufCnt = 0;
volatile uint8_t messageFlag;

//Обработчик прерываний
void USART1_IRQHandler() {
    if (USART1->ISR & USART_ISR_RXNE) {
        uint8_t receivedData = (uint8_t)(USART1->RDR);
        if (!messageFlag) {
            rxBuf[rxBufCnt++] = receivedData;
            if (rxBufCnt == sizeof(rxBuf)) rxBufCnt = 0;
            if (receivedData == 0x0D || receivedData == 0x0A) {
                messageFlag = 1;
                rxBufCnt = 0;
            }
        } else
            rxBufCnt = 0;
    }
    if (USART1->ISR & USART_ISR_FE) USART1->ICR = USART_ICR_FECF;
    if (USART1->ISR & USART_ISR_ORE) USART1->ICR = USART_ICR_ORECF;
    if (USART1->ISR & USART_ISR_NE) USART1->ICR = USART_ICR_NCF;
}

/// @brief Сравнить начало буффера и строку
/// @param s1 буффер
/// @param s2 строка
/// @return результат сравнения
int strc(char* s1, const char* s2) {
    while (1) {
        if (!*s2) return 1;
        if ((*s1 ^ *s2) || !*s1) return 0;
        s1++;
        s2++;
    }
}

/// @brief Парсинг строки с числом
/// @param buf буффер для чтения
/// @param mul сдвиг десятичной запятой
/// @return Значение прочитанного числа
int32_t str2num(char* buf, uint8_t mul) {
    int tmp = 0;
    uint8_t sign = 0;
    int8_t cnt = -128;
    while (*buf != 0 && *buf != ' ' && *buf != '\r' && *buf != '\n' && *buf != '&') {
        if (*buf >= '0' && *buf <= '9') {
            tmp += *buf & 0x0F;
            tmp *= 10;
            if (cnt++ > mul) break;
        } else if (*buf == '.' || *buf == ',')
            cnt = 0;
        else if (*buf == '-')
            sign = 1;
        else
            return 0;
        buf++;
    }
    tmp /= 10;
    if (cnt < 0) cnt = 0;
    for (uint8_t i = cnt; i < mul; i++) tmp *= 10;
    return sign ? -tmp : tmp;
}

/// @brief сдвиг указателя буффера до следующего слова
/// @param b указатель на буффер
void nextWord(char** b) {
    while ((**b != ' ') && (**b != 0)) (*b)++;
    while (**b == ' ') (*b)++;
}

void humanInterface_handler() {
    if (messageFlag) {
        messageFlag = 0;
        char* buf = (char*)rxBuf;
        //Справка //
        if (strc(buf, "помощь") || strc(buf, "help")) {
            xprintf("помощь / help\n        Вывести эту справку\n");
            xprintf("показания / readings\n        Вывести последние показания каждого датчика\n");
            xprintf("пределыТемпературы / tempLimits [номер датчика] [Макс] [Мин]\n        Задать пределы температуры для создания событий\n");
            xprintf("разрешениеПриОжидании / normalRes [номер датчика] [Разрешение(9-12)]\n        Задать разрешение датчика в нормальном режиме\n");
            xprintf("разрешениеПриСобытии / eventRes [номер датчика] [Разрешение(9-12)]\n        Задать разрешение датчика при записи события\n");
            xprintf("количествоИзмерений / measNumber [номер датчика] [количество измерений]\n        Задать количество измерений при записи события\n");
            xprintf("времяИзмерений / measTime [номер датчика] [время измерений]\n        Задать количество времени при записи события\n");
            xprintf("события / events [число событий (0 - все)]\n        Вывести логи событий, начиная с последних записанных\n");
            return;
        }
        //Вывести последние показания каждого датчика //
        if (strc(buf, "показания") || strc(buf, "readings")) {                      //Считывание первого слова - команда
            for (uint8_t i = 0; i < NUM_OF_SENSORS; i++) {                          //Перебор всех датчиков
                int16_t temp = (int32_t)tempSensors[i].temp * 100 / 256;            //Приведение температуры к десятичному виду
                xprintf("Датчик %d; Температура: %c%d.%02d\n",                      //Форматированный вывод
                        (temp < 0 ? '-' : ' '), abs(temp) / 100, abs(temp) % 100);  //с фиксированной запятой
            }
            return;
        }
        //Установить пределы температуры для регистрации события //
        if (strc(buf, "пределыТемпературы") || strc(buf, "tempLimits")) {  //Считывание первого слова - команда
            nextWord(&buf);                                                //Следующее слово
            uint8_t sensorNum = str2num(buf, 0);                           //Номер датчика
            nextWord(&buf);                                                //Следующее слово
            int16_t maxTemp = (str2num(buf, 2) * 256 / 100) & 0xFFF0;      //Максимальная температура (десятичная запятая после 2 разряда)
            nextWord(&buf);                                                //Следующее слово
            int16_t minTemp = (str2num(buf, 2) * 256 / 100) & 0xFFF0;      //Минимальная температура (десятичная запятая после 2 разряда)
            if (minTemp > maxTemp) {                                       //Если температуры перепутаны
                int16_t tempBuf = maxTemp;                                 //
                maxTemp = minTemp;                                         //Поменять местами
                minTemp = tempBuf;                                         //
            }                                                              //
            tempSensors[sensorNum].tempEventMax = maxTemp;                 //Запись параметров в структуру датчика
            tempSensors[sensorNum].tempEventMin = minTemp;                 //
            xprintf("Ок\n");                                               //Вывод сообщения об успехе
            return;
        }
        //Установить разрешение для датчика в режиме ожидания //
        if (strc(buf, "разрешениеПриОжидании") || strc(buf, "normalRes")) {  //Считывание первого слова - команда
            nextWord(&buf);                                                  //Следующее слово
            uint8_t sensorNum = str2num(buf, 0);                             //Номер датчика
            nextWord(&buf);                                                  //Следующее слово
            uint8_t res = constrain(str2num(buf, 0) - 9, 0, 3);              //Приведение значения разрешения
            tempSensors[sensorNum].tempRes = res;                            //Запись параметра в структуру датчика
            if (!tempSensors[sensorNum].eventFlag)                           //Если датчик не записывает событие
                tempSensor_setRes(&tempSensors[sensorNum], res);             //то меняем разрешение
            xprintf("Ок\n");                                                 //Вывод сообщения об успехе
            return;
        }
        //Установить разрешение для датчика во время события //
        if (strc(buf, "разрешениеПриСобытии") || strc(buf, "eventRes")) {  //Считывание первого слова - команда
            nextWord(&buf);                                                //Следующее слово
            uint8_t sensorNum = str2num(buf, 0);                           //Номер датчика
            nextWord(&buf);                                                //Следующее слово
            uint8_t res = constrain(str2num(buf, 0) - 9, 0, 3);            //Приведение значения разрешения
            tempSensors[sensorNum].tempEventRes = res;                     //Запись параметра в структуру датчика
            xprintf("Ок\n");                                               //Вывод сообщения об успехе
            return;
        }
        //Количество измерений при событии //
        if (strc(buf, "количествоИзмерений") || strc(buf, "measNumber")) {  //Считывание первого слова - команда
            nextWord(&buf);                                                 //Следующее слово
            uint8_t sensorNum = str2num(buf, 0);                            //Номер датчика
            nextWord(&buf);                                                 //Следующее слово
            uint8_t measNumber = str2num(buf, 0);                           //Число измерений во время события
            tempSensors[sensorNum].measCounterThreshold = measNumber;       //Запись параметра в структуру датчика
            xprintf("Ок\n");                                                //Вывод сообщения об успехе
            return;
        }
        //Время  проведения точных измерений при событии //
        if (strc(buf, "времяИзмерений") || strc(buf, "measTime")) {  //Считывание первого слова - команда
            nextWord(&buf);                                          //Следующее слово
            uint8_t sensorNum = str2num(buf, 0);                     //Номер датчика
            nextWord(&buf);                                          //Следующее слово
            uint32_t measTime = str2num(buf, 2);                     //Длительность измерений (десятичная запятая после 2 разряда)
            tempSensors[sensorNum].measTotalEventPeriod = measTime;  //Запись параметра в структуру датчика
            xprintf("Ок\n");                                         //Вывод сообщения об успехе
            return;
        }
        //Вывести список произошедших событий //
        if (strc(buf, "события") || strc(buf, "events")) {  //Считывание первого слова - команда
            nextWord(&buf);                                 //Следующее слово
            uint8_t eventsToPrint = str2num(buf, 0);        //Число событий в выводе
            eventLogging_print(eventsToPrint);              //Вывод лога событий
            return;
        }
    }
}
