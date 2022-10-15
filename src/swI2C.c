#include "swI2C.h"

#include "config.h"
#include "stm32f0xx.h"

#define SDA_GET(s) ((s->SDA_port->IDR & 1 << s->SDA_pos) != 0)
#define SDA_HIGH(s) s->SDA_port->BSRR = 0x0100 << s->SDA_pos
#define SDA_LOW(s) s->SDA_port->BSRR = 1 << s->SDA_pos
#define SCL_HIGH(s) s->SCL_port->BSRR = 0x0100 << s->SCL_pos
#define SCL_LOW(s) s->SCL_port->BSRR = 1 << s->SCL_pos

void swI2C_init(swI2C_t* i2cInstance) {  //Инициализация GPIO для программного I2C
    //Линия данных
    i2cInstance->SDA_port->BSRR = 0x0100 << i2cInstance->SDA_pos;        // Выставляем высокий уровень
    i2cInstance->SDA_port->PUPDR |= 0b01 << i2cInstance->SDA_pos * 2;    // Подтяжка к питанию
    i2cInstance->SDA_port->OTYPER |= 1 << i2cInstance->SDA_pos;          // Тип выхода OpenDrain
    i2cInstance->SDA_port->OSPEEDR |= 0b11 << i2cInstance->SDA_pos * 2;  // Скорость High Speed
    i2cInstance->SDA_port->MODER |= 0b01 << i2cInstance->SDA_pos * 2;    // Режим порта GP Output
    //Линия тактирования
    i2cInstance->SCL_port->BSRR = 0x0100 << i2cInstance->SCL_pos;        // Аналогично
    i2cInstance->SCL_port->PUPDR |= 0b01 << i2cInstance->SCL_pos * 2;    //
    i2cInstance->SCL_port->OTYPER |= 1 << i2cInstance->SCL_pos;          // линии
    i2cInstance->SCL_port->OSPEEDR |= 0b11 << i2cInstance->SCL_pos * 2;  //
    i2cInstance->SCL_port->MODER |= 0b01 << i2cInstance->SCL_pos * 2;    // данных
}

static volatile struct {
    swI2C_t* i2cInstance;      // Экземляр структуры I2C интерфейса
    uint8_t* dataAddr;         // Адрес для записи принятых данных
    uint8_t frameData[5];      // Принятые / подготовленные для передачи кадры
    uint8_t frameCounter : 4;  // Количество оставшихся кадров для приема/передачи
    uint8_t state : 4;         // Состояние автомата
    uint8_t quarterPeriods;    // Текущая четверть периода
} i2cStateMachine;

void swI2C_writeReg8(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    while (TIM17->CR1 & TIM_CR1_CEN) __WFI();
    i2cStateMachine.i2cInstance = i2cInstance;
    i2cStateMachine.frameData[2] = (devAddr << 1) + 0;
    i2cStateMachine.frameData[1] = regAddr;
    i2cStateMachine.frameData[0] = data;
    i2cStateMachine.frameCounter = 2;
    i2cStateMachine.state = 0;
    i2cStateMachine.quarterPeriods = 0;
    TIM17->CR1 |= TIM_CR1_CEN;
}

void swI2C_writeReg16(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint16_t data) {
    while (TIM17->CR1 & TIM_CR1_CEN) __WFI();
    i2cStateMachine.i2cInstance = i2cInstance;
    i2cStateMachine.frameData[3] = (devAddr << 1) + 0;
    i2cStateMachine.frameData[2] = regAddr;
    i2cStateMachine.frameData[1] = (uint8_t)(data >> 8);
    i2cStateMachine.frameData[0] = (uint8_t)(data & 0xFF);
    i2cStateMachine.frameCounter = 3;
    i2cStateMachine.state = 0;
    i2cStateMachine.quarterPeriods = 0;
    TIM17->CR1 |= TIM_CR1_CEN;
}
void swI2C_readReg8(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint8_t* data) {
    while (TIM17->CR1 & TIM_CR1_CEN) __WFI();
    i2cStateMachine.i2cInstance = i2cInstance;
    i2cStateMachine.frameData[3] = (devAddr << 1) + 0;
    i2cStateMachine.frameData[2] = regAddr;
    i2cStateMachine.frameData[1] = (devAddr << 1) + 1;
    i2cStateMachine.frameData[0] = 0;
    i2cStateMachine.frameCounter = 3;
    i2cStateMachine.state = 0;
    i2cStateMachine.quarterPeriods = 0;
    TIM17->CR1 |= TIM_CR1_CEN;
}
void swI2C_readReg16(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint16_t* data) {
    while (TIM17->CR1 & TIM_CR1_CEN) __WFI();
    i2cStateMachine.i2cInstance = i2cInstance;
    i2cStateMachine.frameData[4] = (devAddr << 1) + 0;
    i2cStateMachine.frameData[3] = regAddr;
    i2cStateMachine.frameData[2] = (devAddr << 1) + 1;
    i2cStateMachine.frameData[1] = 0;
    i2cStateMachine.frameData[0] = 0;
    i2cStateMachine.frameCounter = 4;
    i2cStateMachine.state = 0;
    i2cStateMachine.quarterPeriods = 0;
    TIM17->CR1 |= TIM_CR1_CEN;
}

void TIM17_IRQHandler() {
    TIM17->SR = 0;
    switch (i2cStateMachine.state) {
        case 0:   // старт-последовательность перед записью в регистр
        case 8:   // старт-последовательность перед передачей адреса регистра для чтения
        case 11:  // повторная старт-последовательность перед чтением из регистра
            switch (i2cStateMachine.quarterPeriods) {
                case 0:
                    SDA_HIGH(i2cStateMachine.i2cInstance);  //отпускаем линию данных
                    break;
                case 1:
                    SCL_HIGH(i2cStateMachine.i2cInstance);  //отпускаем линию тактирования
                    break;
                case 2:
                    SDA_LOW(i2cStateMachine.i2cInstance);  //подтягиваем линию данных
                    break;
                case 3:
                    SCL_LOW(i2cStateMachine.i2cInstance);  //подтягиваем линию тактирования
                    i2cStateMachine.state++;               //следующее состояние
                    i2cStateMachine.quarterPeriods = 0;
                    break;
            }
            break;
        case 1:   // передача кадра c I2C адресом перед записью в регистр
        case 2:   // передача кадра c адресом регистра для записи
        case 3:   // передача кадра c байтом для записи
        case 4:   // передача кадра c младшим байтом для записи
        case 9:   // передача кадра c I2C адресом перед передачей адреса регистра для чтения
        case 10:  // передача кадра c адресом регистра для чтения
            switch (i2cStateMachine.quarterPeriods & 0x3) {
                case 0:
                    if (i2cStateMachine.frameData[i2cStateMachine.frameCounter] & 0x80 || i2cStateMachine.quarterPeriods > 31)
                        SDA_HIGH(i2cStateMachine.i2cInstance);  //отпускаем линию данных при передаче единицы или если ожидаем SLAVE ACK
                    else                                        //иначе
                        SDA_LOW(i2cStateMachine.i2cInstance);   //подтягиваем линию данных
                    break;
                case 1:
                    SCL_HIGH(i2cStateMachine.i2cInstance);  //отпускаем линию тактирования
                    break;
                case 2:
                    i2cStateMachine.frameData[i2cStateMachine.frameCounter] <<= 1;  //подготавливаем следующий бит к отправке
                    break;
                case 3:
                    SCL_LOW(i2cStateMachine.i2cInstance);       //конец периода, подтягиваем линию данных
                    if (i2cStateMachine.quarterPeriods > 31) {  //Конец кадра
                        i2cStateMachine.quarterPeriods = 0;     //Обнуление счетчика четверть-периодов
                        if (i2cStateMachine.frameCounter > 0) {
                            i2cStateMachine.frameCounter--;
                            i2cStateMachine.i2cInstance->frameError = 0;
                            i2cStateMachine.state++;
                            return;
                        }
                        if (SDA_GET(i2cStateMachine.i2cInstance)) {        //Проверка SLAVE ACK
                            i2cStateMachine.i2cInstance->frameError |= 1;  //Счетчик ошибок кадра
                        }
                        i2cStateMachine.state = 0x5;  //Останавливаем передачу
                        return;
                    }
                    break;
            }
            break;
        case 12:  //прием кадра cо старшим байтом
        case 13:  //прием кадра c младшим байтом
            switch (i2cStateMachine.quarterPeriods & 0x3) {
                case 0:
                    if (i2cStateMachine.quarterPeriods > 31)
                        SDA_LOW(i2cStateMachine.i2cInstance);  // если данные переданы, то подтягиваем линию данных - MASTER ACK
                    else {
                        SDA_HIGH(i2cStateMachine.i2cInstance);                          // отпускаем линию данных во время чтения
                        i2cStateMachine.frameData[i2cStateMachine.frameCounter] <<= 1;  // побитовый сдвиг влево (Старший бит идет первым)
                    }
                    break;
                case 1:
                    SCL_HIGH(i2cStateMachine.i2cInstance);  //отпускаем линию тактирования
                    break;
                case 2:  //считываем бит данных
                         // на 9 такте линия данных подтянута к 0. Побитовое ИЛИ с 0х00 не повлияет на принятые данные
                    i2cStateMachine.frameData[i2cStateMachine.frameCounter] |= SDA_GET(i2cStateMachine.i2cInstance);
                    break;
                case 3:
                    SCL_LOW(i2cStateMachine.i2cInstance);       //конец периода, подтягиваем линию данных
                    if (i2cStateMachine.quarterPeriods > 31) {  //Конец кадра
                        i2cStateMachine.quarterPeriods = 0;     //Обнуление счетчика четверть-периодов
                        if (i2cStateMachine.frameCounter > 0) {
                            i2cStateMachine.frameCounter--;
                            i2cStateMachine.state++;
                        } else
                            i2cStateMachine.state += 2;  //Останавливаем передачу
                        return;
                    }
                    break;
            }
            break;
        default:  // стоп-последовательность
            switch (i2cStateMachine.quarterPeriods) {
                case 0:
                    SDA_LOW(i2cStateMachine.i2cInstance);  //подтягиваем линию данных
                    break;
                case 1:
                    SCL_HIGH(i2cStateMachine.i2cInstance);  //отпускаем линию тактирования
                    break;
                case 2:
                    SDA_HIGH(i2cStateMachine.i2cInstance);  //отпускаем линию данных
                    break;
                case 3:
                    TIM17->CR1 &= ~TIM_CR1_CEN;
                    TIM17->CNT = 0xFFFF;
                    TIM17->SR = 0;
                    if (i2cStateMachine.state > 13)
                        i2cStateMachine.dataAddr[0] = i2cStateMachine.frameData[0];  //Последний байт из frameData - младший байт
                    if (i2cStateMachine.state > 14)
                        i2cStateMachine.dataAddr[1] = i2cStateMachine.frameData[1];  //Предпоследний байт из frameData - старший байт
                    break;
            }
    }
    i2cStateMachine.quarterPeriods++;
}