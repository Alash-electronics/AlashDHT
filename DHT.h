#ifndef DHT_H
#define DHT_H
#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif
#include <Wire.h>

// 8 МГц (примерно) AVR ---------------------------------------------------------
#if (F_CPU >= 7400000UL) && (F_CPU <= 9500000UL)
    #define COUNT 3
    // 16 МГц (примерно) AVR --------------------------------------------------------
#elif (F_CPU >= 15400000UL) && (F_CPU <= 19000000L)
    #define COUNT 6
    // 48 МГц SAMD21J18A (Sodaq Explorer)
#elif (F_CPU == 48000000UL)
    #define COUNT 18
    // 64 МГц NRF52840 
#elif (F_CPU == 64000000UL)
    #define COUNT 20
    // 168 МГц STM32F405 STM32F407
#elif (F_CPU == 168000000L)
    #define COUNT 40
#elif (F_CPU == 80000000L)
    #define COUNT 22
#elif (F_CPU == 160000000L)
    #define COUNT 32
#else
    #define COUNT 25
    //#error "Скорость ЦП не поддерживается"
#endif

//RP2040
#if defined(ARDUINO_ARCH_RP2040)
    #define COUNT 10
#endif
/*
  Библиотека DHT
*/

// количество переходов синхронизации, которые нужно отслеживать. 2 * количество бит + запас
#define MAXTIMINGS 85
#define SERIALPRINT Serial

#define DEFAULT_IIC_ADDR    0x38
#define RESET_REG_ADDR      0xba
#define  HUMIDITY_INDEX     0
#define  TEMPRATURE_INDEX  1

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

#define DHT10 10
#define DHT20 10

class DHT {
  private:
    uint8_t data[6];
    uint8_t _pin, _type, _count;
    boolean read(void);
    unsigned long _lastreadtime;
    boolean firstreading;

  public:
    DHT(uint8_t pin, uint8_t type, uint8_t count = COUNT);
    DHT(uint8_t type);
    void begin(void);
    float readTemperature(bool S = false);
    float convertCtoF(float);
    float readHumidity(void);

    /** Общий интерфейс для получения значений температуры и влажности. Поддерживает все устройства DHT.

        @return 0 если калибровка не удалась, 1 если успешна.
     **/
    int readTempAndHumidity(float* data);

    // Цифровые интерфейсы DHT10 (i2c), только для DHT10.
    int i2cReadByte(uint8_t& byte);
    int i2cReadBytes(uint8_t* bytes, uint32_t len);
    int i2cWriteBytes(uint8_t* bytes, uint32_t len);
    int i2cWriteByte(uint8_t byte);

    /** Сброс датчика.
        @return 0 если калибровка не удалась, 1 если успешна.
     **/
    int DHT10Reset(void);

    /** Чтение регистра статуса. Проверка флага калибровки - бит [3]: 1 - калибровка выполнена, 0 - не выполнена.

        @return 0 если калибровка не удалась, 1 если успешна.
     **/
    int DHT10ReadStatus(void);

    /** Инициализация датчика, отправка 0x08, 0x00 в регистр 0xe1.
        @return 0 если успешна, ненулевое значение если неудачна.
     **/
    int setSystemCfg(void);

    /** Чтение буфера результатов температуры и влажности с датчика.
        Всего 6 байт, первый байт для регистра статуса, остальные 5 байт для данных температуры и влажности.
        @return 0 если успешна, ненулевое значение если неудачна.
     **/
    int readTargetData(uint32_t* data);

    /** Функция инициализации DHT10.
        Сброс датчика и ожидание завершения калибровки.
        @return 0 если успешна, ненулевое значение если неудачна.
     **/
    int DHT10Init(void);
};

#endif