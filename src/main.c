#include "config.h"
#include "humanInterface.h"
#include "sensorPolling.h"
#include "sysControl.h"
#include "sysInit.h"

int main(void) {
    sysInit();
    sensorPolling_init();
    while (1) {
        sensorPolling_handler();
        humanInterface_handler();
        __WFI();
    }
}