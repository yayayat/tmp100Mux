#include "swI2C.h"

#include "config.h"
#include "stm32f0xx.h"

#define SDA_GET(s) ((s->SDA_port->IDR & 1 << s->SDA_pos) != 0)
#define SDA_HIGH(s) s->SDA_port->BSRR = 0x0100 << s->SDA_pos
#define SDA_LOW(s) s->SDA_port->BSRR = 1 << s->SDA_pos
#define SCL_HIGH(s) s->SCL_port->BSRR = 0x0100 << s->SCL_pos
#define SCL_LOW(s) s->SCL_port->BSRR = 1 << s->SCL_pos

//передача кадра
int8_t swI2C_sendFrame(swI2C_t* i2cInstance, uint8_t data) {
    for (uint8_t i = 8; i; i--) {
        SDA_LOW(i2cInstance);
        if (data & 0x80) SDA_HIGH(i2cInstance);
        SCL_HIGH(i2cInstance);
        data <<= 1;
        SCL_LOW(i2cInstance);
    }
    SDA_HIGH(i2cInstance);
    SCL_HIGH(i2cInstance);
    if (SDA_GET(i2cInstance)) {
        SCL_LOW(i2cInstance);
        i2cInstance->frameError = 1;  // Ошибка кадра: ACK не получено
        return -1;
    } else {
        SCL_LOW(i2cInstance);
        i2cInstance->frameError = 0;  // ACK получено, сброс счетчика ошибок
        return 0;
    }
}

//прием кадра
uint8_t swI2C_receiveFrame(swI2C_t* i2cInstance) {
    uint8_t buf = 0;
    for (uint8_t i = 8; i; i--) {
        SCL_HIGH(i2cInstance);
        asm("nop");
        buf <<= 1;
        buf |= SDA_GET(i2cInstance);
        SCL_LOW(i2cInstance);
    }
    SDA_LOW(i2cInstance);
    SCL_HIGH(i2cInstance);
    asm("nop");
    SCL_LOW(i2cInstance);
    SDA_HIGH(i2cInstance);
    return buf;
}

//старт последовательность
void swI2C_start(swI2C_t* i2cInstance) {
    SDA_LOW(i2cInstance);
    SCL_LOW(i2cInstance);
}

//стоп последовательность
void swI2C_stop(swI2C_t* i2cInstance) {
    SDA_LOW(i2cInstance);
    SCL_HIGH(i2cInstance);
    SDA_HIGH(i2cInstance);
}

//инициализация GPIO для I2C
void swI2C_init(swI2C_t* i2cInstance) {
    i2cInstance->SDA_port->BSRR = 0x0100 << i2cInstance->SDA_pos;
    i2cInstance->SDA_port->PUPDR |= 0b01 << i2cInstance->SDA_pos * 2;
    i2cInstance->SDA_port->OTYPER |= 1 << i2cInstance->SDA_pos;
    i2cInstance->SDA_port->OSPEEDR |= 0b11 << i2cInstance->SDA_pos * 2;
    i2cInstance->SDA_port->MODER |= 0b01 << i2cInstance->SDA_pos * 2;

    i2cInstance->SCL_port->BSRR = 0x0100 << i2cInstance->SCL_pos;
    i2cInstance->SCL_port->PUPDR |= 0b01 << i2cInstance->SCL_pos * 2;
    i2cInstance->SCL_port->OTYPER |= 1 << i2cInstance->SCL_pos;
    i2cInstance->SCL_port->OSPEEDR |= 0b11 << i2cInstance->SCL_pos * 2;
    i2cInstance->SCL_port->MODER |= 0b01 << i2cInstance->SCL_pos * 2;
}

void swI2C_writeReg8(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    swI2C_start(i2cInstance);                              //старт-последовательность
    if (swI2C_sendFrame(i2cInstance, (devAddr << 1) + 0))  //Передача адреса устройства c флагом записи
        return;
    if (swI2C_sendFrame(i2cInstance, regAddr))  //Передача адреса регистра
        return;
    if (swI2C_sendFrame(i2cInstance, data))  //Запись байта по адресу
        return;
    swI2C_stop(i2cInstance);  //стоп-последовательность
}

void swI2C_writeReg16(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint16_t data) {
    swI2C_start(i2cInstance);                              //старт-последовательность
    if (swI2C_sendFrame(i2cInstance, (devAddr << 1) + 0))  //Передача адреса устройства c флагом записи
        return;
    if (swI2C_sendFrame(i2cInstance, regAddr))  //Передача адреса регистра
        return;
    if (swI2C_sendFrame(i2cInstance, (uint8_t)(data << 8)))  //Запись старшего байта по адресу
        return;
    if (swI2C_sendFrame(i2cInstance, (uint8_t)(data & 0xFF)))  //Запись младшего байта по адресу
        return;

    swI2C_stop(i2cInstance);  //стоп-последовательность
}
void swI2C_readReg8(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint8_t* data) {
    swI2C_start(i2cInstance);                              //старт-последовательность
    if (swI2C_sendFrame(i2cInstance, (devAddr << 1) + 0))  //Передача адреса устройства c флагом записи
        return;
    if (swI2C_sendFrame(i2cInstance, regAddr))  //Передача адреса регистра
        return;
    swI2C_start(i2cInstance);                              //старт-последовательность
    if (swI2C_sendFrame(i2cInstance, (devAddr << 1) + 1))  //Передача адреса устройства c флагом чтения
        return;

    uint8_t buf = swI2C_receiveFrame(i2cInstance);  //Прием байта в буффер
    swI2C_stop(i2cInstance);                        //стоп-последовательность
    *data = buf;                                    //Запись данных по адресу
}
void swI2C_readReg16(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint16_t* data) {
    swI2C_start(i2cInstance);                              //старт-последовательность
    if (swI2C_sendFrame(i2cInstance, (devAddr << 1) + 0))  //Передача адреса устройства c флагом записи
        return;
    if (swI2C_sendFrame(i2cInstance, regAddr))  //Передача адреса регистра
        return;
    swI2C_start(i2cInstance);                              //старт-последовательность
    if (swI2C_sendFrame(i2cInstance, (devAddr << 1) + 1))  //Передача адреса устройства c флагом чтения
        return;
    uint8_t buf[2] = {0, 0};                   //Буффер для приема данных
    buf[1] = swI2C_receiveFrame(i2cInstance);  //Прием старшего байта
    buf[0] = swI2C_receiveFrame(i2cInstance);  //Прием младшего байта
    swI2C_stop(i2cInstance);                   //стоп-последовательность
    *data = *(uint16_t*)buf;                   //Запись данных по адресу
}
