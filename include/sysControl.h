#pragma once
#ifndef SYSCONTROL
#define SYSCONTROL

#include <stdint.h>

extern volatile uint32_t tick;  //счетчик времени со старта МК в сотых секунды
extern volatile uint32_t sec;   //счетчик времени со старта МК в секундах

#endif