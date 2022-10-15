#include "TMP100.h"

#include "stm32f0xx.h"

int8_t tempSensor_init(tempSensor_t* sensorInstance, swI2C_t* i2cInstance) {
    sensorInstance->i2cInstance = i2cInstance;
    swI2C_init(sensorInstance->i2cInstance);                               //Инициализация I2C
    sensorInstance->i2cAddr = 0b1001000;                                   //Адрес TMP100 по умолчанию
    sensorInstance->measPeriod = FAST_MEASURMENT_PERIOD;                   //Период опроса по умолчанию
    sensorInstance->tempRes = 0b00;                                        //Разрешение в нормальном режиме по умолчанию
    sensorInstance->tempEventRes = 0b11;                                   //Разрешение во время события по умолчанию
    sensorInstance->measCounterThreshold = 10;                             //Число измерений во время события по умолчанию
    sensorInstance->measTotalEventPeriod = 400;                            //Длительность измерений во время события по умолчанию
    swI2C_writeReg8(sensorInstance->i2cInstance, 0x00, 0b00000110, 0x00);  // General call: Защелкивание адресных пинов,
                                                                           // сброс внутренних регистров
    if (sensorInstance->i2cInstance->frameError)                           // Если произошла ошибка кадра, возвращаем ошибку
        return -1;
    return 0;
}

int8_t tempSensor_setRes(tempSensor_t* sensorInstance, uint8_t res) {
    swI2C_writeReg8(sensorInstance->i2cInstance,
                    sensorInstance->i2cAddr, 0x01, (res << 5));    //Отправка разрешения
    if (sensorInstance->i2cInstance->frameError)                   //Если произошла ошибка кадра, возвращаем ошибку
        return -1;                                                 //
    sensorInstance->measPeriod = (FAST_MEASURMENT_PERIOD << res);  //Перерасчет периода опроса исходя из разрешения
    //Период измерений = Период измерения c 9 битной точностью * 2^[разрешение измерения}(0,1,2,3)
    return 0;
}

int8_t tempSensor_readTemp(tempSensor_t* sensorInstance) {
    swI2C_readReg16(sensorInstance->i2cInstance,
                    sensorInstance->i2cAddr, 0x00, (uint16_t*)&sensorInstance->temp);  //Считывание температуры
    if (sensorInstance->i2cInstance->frameError)                                       // Если произошла ошибка кадра, возвращаем ошибку
        return -1;
    return 0;
}
