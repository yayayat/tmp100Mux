#include "sysControl.h"

#include "sysInit.h"

uint32_t volatile tick = 0;
uint32_t sec_d = 0;
uint32_t volatile sec = 0;

void SysTick_Handler(void) {  //Период ~497 дней при частоте 100Гц
    tick++;
    if (++sec_d == 100) {
        sec_d = 0;
        sec++;
    }
}