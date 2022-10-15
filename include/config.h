#pragma once
#ifndef CONFIG
#define CONFIG

#include "pinMap.h"
#include "stm32f0xx.h"
#include "swI2C.h"

#define NUM_OF_SENSORS 10   //Общее количество сенсоров
#define EVENT_LOG_SIZE 256  //Размер лога событий

#define SENSORS_PINS \
    {PA0, PA1},      \
        {PA2, PA3},  \
        {PA4, PA5},  \
        {PA6, PA7},  \
        {PA8, PA9},  \
        {PB0, PB1},  \
        {PB2, PB3},  \
        {PB4, PB5},  \
        {PB6, PB7},  \
    { PB8, PB9 }

#endif