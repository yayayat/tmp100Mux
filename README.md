# Массив датчиков температуры
## Аппаратная часть
#### Микроконтроллер STM32F030C8

[Datasheet](https://www.st.com/resource/en/datasheet/stm32f030f4.pdf)

[Reference Manual](https://www.st.com/resource/en/reference_manual/dm00091010-stm32f030x4x6x8xc-and-stm32f070x6xb-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

#### Датчик температуры: Texas Instruments TMP100

[Datasheet](https://www.ti.com/lit/ds/symlink/tmp100.pdf?ts=1665523508446&ref_url=https%253A%252F%252Fwww.google.com%252F)

#### Другие датчики
При необходимости, можно использовать другой аналогичный датчик. 

Для нового датчика нужно определить свои функции ```tempSensor_init```, ```tempSensor_setRes``` и ```tempSensor_readTemp``` и включить их в файл ```sensorPolling.h```. За основу можно взять файлы ```TMP100.h``` и ```TMP100.c```

## Сборка
### [Platformio](https://platformio.org)
Проект собирается в среде Platformio. [Плагин для VSCode](https://platformio.org/install/ide?install=vscode)

После установки плагина, на нижней панели появится группа иконок

![platformioBar](https://user-images.githubusercontent.com/19162596/195965329-def5305f-291a-4720-b9ee-bd1b88c483ef.png)

### Компиляция
Можно скомпилировать код для проверки, для этого нужно нажать ![platformioCompile](https://user-images.githubusercontent.com/19162596/195965518-f70c3bd2-a5ef-40b8-bf9a-e608b4bde425.png)

### Загрузка прошивки
Для прошивки подключите SWD программатор и нажмите ![platformioUpload](https://user-images.githubusercontent.com/19162596/195965496-37973f9e-3140-47d8-844f-aaf3b332c32b.png)


## Настройка
Вся настройка осуществляется в файле ```config.h```
### Количество датчиков
Задается макросом
```
#define NUM_OF_SENSORS 10
```
### Распиновка I²C

Задается многострочным макросом в формате {<пин SDA>, <пин SCL>},\

Количество строк должно совпадать с числом указанных датчиков
```
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
```
### Длина лога событий
Задается макросом
```
#define EVENT_LOG_SIZE 256
```
