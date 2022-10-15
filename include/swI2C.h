#pragma once
#ifndef SWI2C
#define SWI2C

#include "stm32f0xx.h"

#define SW_I2C_PINS_NUMBER 20

typedef struct {
    GPIO_TypeDef* SDA_port;  // GPIO порт к которому подключен контакт SDA
    uint8_t SDA_pos;         // Номер пина к которому подключен контакт SDA
    GPIO_TypeDef* SCL_port;  // GPIO порт к которому подключен контакт SCL
    uint8_t SCL_pos;         // Номер пина к которому подключен контакт SCL
    uint8_t frameError : 1;  // Флаг ошибки кадра
} swI2C_t;                   // Программный I2C интерфейс

/// @brief Инициализация программного интерфейса I2C
/// @param i2cInstance Экземляр структуры I2C интерфейса
void swI2C_init(swI2C_t* i2cInstance);

/// @brief Запись одного байта в регистр по адресу
/// @param i2cInstance Экземляр структуры I2C интерфейса
/// @param devAddr Адрес I2C устройства
/// @param regAddr Адрес регистра
/// @param data Байт данные для передачи
void swI2C_writeReg8(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint8_t data);

/// @brief Запись двух байт в регистр по адресу
/// @param i2cInstance Экземляр структуры I2C интерфейса
/// @param devAddr Адрес I2C устройства
/// @param regAddr Адрес регистра
/// @param data Два байта данных для передачи
void swI2C_writeReg16(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint16_t data);

/// @brief Запись двух байт в регистр по адресу
/// @param i2cInstance Экземляр структуры I2C интерфейса
/// @param devAddr Адрес I2C устройства
/// @param regAddr Адрес регистра
/// @param data Адрес для приема одного байта данных
void swI2C_readReg8(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint8_t* data);

/// @brief
/// @param i2cInstance Экземляр структуры I2C интерфейса
/// @param devAddr Адрес I2C устройства
/// @param regAddr Адрес регистра
/// @param data Адрес для приема двух байт данных
void swI2C_readReg16(swI2C_t* i2cInstance, uint8_t devAddr, uint8_t regAddr, uint16_t* data);

#endif
