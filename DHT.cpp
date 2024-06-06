/*  
Библиотека AlashDHT
*/

#include <math.h>
#include "DHT.h"
//#define NAN 0
#ifdef DEBUG
    #define DEBUG_PRINT(...)  Serial.println(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
#endif

DHT::DHT(uint8_t pin, uint8_t type, uint8_t count) {
    _pin = pin; // Номер пина
    _type = type; // Тип датчика DHT
    _count = count; // Пороговое значение битов
    firstreading = true; // Флаг первого чтения
}

DHT::DHT(uint8_t type) {
    if (_type != DHT10){
        DEBUG_PRINT("Ошибка: Не указаны пины\n");
    }
    _pin = 0; // По умолчанию, если не указано
    _type = type; // Тип датчика DHT
    _count = COUNT; // Пороговое значение битов по умолчанию
    firstreading = true; // Флаг первого чтения
}

void DHT::begin(void) {

    if (_type == DHT10) {
        if (DHT10Init()) {
            SERIALPRINT.println("Ошибка: Не удалось инициализировать DHT 11\n");
            while (1);
        }
    } else {
        // Установка пинов!
        pinMode(_pin, INPUT); // Установка режима пина на вход
        digitalWrite(_pin, HIGH); // Установка пина в состояние HIGH
        _lastreadtime = 0;
    }

}

/** 
 * Общий интерфейс для получения значений температуры и влажности. Поддерживает все устройства DHT.
 * @param data Массив для хранения показаний температуры и влажности.
 * @return 0 при успешной калибровке, 1 при успехе.
 */
int DHT::readTempAndHumidity(float* data) {
    uint32_t target_val[2] = {0};
    uint32_t cnt = 0;
    if (_type == DHT10) {
        while (DHT10ReadStatus() == 0) {
            DHT10Init();
            delay(30);
            cnt++;
            if (cnt > 3) {
                return -1;
            }
        }
        // Ожидание готовности данных
        while (readTargetData(target_val)) {
            cnt++;
            delay(50);
            if (cnt > 5) {
                return -1;
            }

        }
        data[0] = target_val[0] * 100.0 / 1024 / 1024;
        data[1] = target_val[1] * 200.0 / 1024 / 1024 - 50;

    } else {
        data[0] = readHumidity(); // Чтение влажности
        data[1] = readTemperature(); // Чтение температуры
        if (isnan(data[0]) || isnan(data[1])) {
            return -1; // Проверка на NaN значения
        }
    }
    return 0;
}

// Boolean S == Scale. True == Fahrenheit; False == Celsius
float DHT::readTemperature(bool S) {
    if(_type == DHT10){
        float temp[2];
        readTempAndHumidity(temp);
        if (S) {
            temp[1] = convertCtoF(temp[1]);
        }
        return temp[1];
    }
    else{
        float f;
        if (read()) {
            switch (_type) {
                case DHT11:
                    f = data[2];
                    if(data[3]%128<10){
                        f += data[3]%128/10.0f;
                    }else if(data[3]%128<100){
                        f += data[3]%128/100.0f;
                    }else{
                        f += data[3]%128/1000.0f;
                    }
                    if(data[3]>=128){ // Самый левый разряд указывает на отрицательный знак. 
                        f = -f;
                    }
                    if (S) {
                        f = convertCtoF(f);
                    }

                    return f;
                case DHT22:
                case DHT21:
                    f = data[2] & 0x7F;
                    f *= 256;
                    f += data[3];
                    f /= 10;
                    if (data[2] & 0x80) {
                        f *= -1;
                    }
                    if (S) {
                        f = convertCtoF(f);
                    }

                    return f;
            }
        }
    }
    DEBUG_PRINT("Чтение не удалось");
    return NAN;
}

float DHT::convertCtoF(float c) {
    return c * 9 / 5 + 32;
}

float DHT::readHumidity(void) {
    if(_type == DHT10){
        float temp[2];
        readTempAndHumidity(temp);
        return temp[0];
    }
    else{
        float f;
        if (read()) {
            switch (_type) {
                case DHT11:
                    f = data[0];
                    return f;
                case DHT22:
                case DHT21:
                    f = data[0];
                    f *= 256;
                    f += data[1];
                    f /= 10;
                    return f;
            }
        }
    }
    DEBUG_PRINT("Чтение не удалось");
    return NAN;
}

boolean DHT::read(void) {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    unsigned long currenttime;

    // Поднимаем пин в HIGH и ждем 250 миллисекунд
    digitalWrite(_pin, HIGH);
    delay(250);

    currenttime = millis();
    if (currenttime < _lastreadtime) {
        // Произошло переполнение
        _lastreadtime = 0;
    }
    if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
        return true; // Возвращаем последние корректные измерения
    }
    firstreading = false;

    _lastreadtime = millis();

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    // Теперь опускаем его в LOW на ~20 миллисекунд
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delay(20);
    digitalWrite(_pin, HIGH); // Установка вывода датчика в состояние HIGH (высокий уровень)
    delayMicroseconds(20); // Задержка в микросекундах
    pinMode(_pin, INPUT); // Установка вывода датчика в режим входа

    // Чтение временных интервалов
    for (i = 0; i < MAXTIMINGS; i++) {
        counter = 0;
        while (digitalRead(_pin) == laststate) { // Подсчет временных интервалов, пока состояние пина не изменится
            counter++;
            delayMicroseconds(1);
            if (counter == 255) { // Проверка на максимальное значение счетчика
                break;
            }
        }
        laststate = digitalRead(_pin);

        if (counter == 255) { // Проверка на максимальное значение счетчика
            break;
        }

        // Пропуск первых трех переходов
        if ((i >= 4) && (i % 2 == 0)) {
            // Запись каждого бита в массив данных
            data[j / 8] <<= 1;
            if (counter > _count) {
                data[j / 8] |= 1;
            }
            j++;
        }

    }

    //sei(); // Включение прерываний

    /*
        DEBUG_PRINTln(j, DEC);
        DEBUG_PRINT(data[0], HEX); DEBUG_PRINT(", ");
        DEBUG_PRINT(data[1], HEX); DEBUG_PRINT(", ");
        DEBUG_PRINT(data[2], HEX); DEBUG_PRINT(", ");
        DEBUG_PRINT(data[3], HEX); DEBUG_PRINT(", ");
        DEBUG_PRINT(data[4], HEX); DEBUG_PRINT(" =? ");
        DEBUG_PRINTln(data[0] + data[1] + data[2] + data[3], HEX);
    */

    // Проверка на то, что прочитано 40 бит и совпадает контрольная сумма
    if ((j >= 40) &&
            (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
        return true;
    }


    return false;

}

/*****************************************************************************/
/*****************************************************************************/

/** Сброс сенсора.
    @return 0 в случае неудачной калибровки, 1 в случае успеха.
 **/
int DHT::DHT10Reset(void) {
    if (_type == DHT10) {
        return i2cWriteByte(RESET_REG_ADDR); // Запись байта в регистр для сброса сенсора
    } else {
        return 0;
        SERIALPRINT.println("Эта функция поддерживается только для DHT10");
    }

}

/** Чтение регистра статуса. Проверка флага калибровки - бит[3]: 1 - калибровка выполнена успешно, 0 - калибровка не выполнена.

    @return 0 в случае неудачной калибровки, 1 в случае успеха.

 **/
int DHT::DHT10ReadStatus(void) {

    int ret = 0;
    uint8_t statu = 0;
    if (_type == DHT10) {
        ret = i2cReadByte(statu); // Чтение байта из регистра статуса
        if (ret) {
            SERIALPRINT.println("Не удалось прочитать байт\n");
        }
        if ((statu & 0x8) == 0x8) { // Проверка бита калибровки
            return 1;
        } else {
            return 0;
        }
    } else {
        SERIALPRINT.println("Эта функция поддерживается только для DHT10");
        return 0;
    }

}

/** Инициализация сенсора, отправка 0x08,0x00 в регистр 0xe1.
    @ return : 0 в случае успеха, ненулевое значение в случае ошибки.
 **/
int DHT::setSystemCfg(void) {
    uint8_t cfg_param[] = {0xe1, 0x08, 0x00}; // Параметры конфигурации
    if (_type == DHT10) {
        return i2cWriteBytes(cfg_param, sizeof(cfg_param)); // Запись массива байтов в регистр для настройки системы
    } else {
        SERIALPRINT.println("Эта функция поддерживается только для DHT10"); // Вывод сообщения об ошибке, если тип датчика не DHT10
        return 0;
    }
}


/** Чтение данных о температуре и влажности из буфера датчика.
    Общее количество байтов - 6, первый байт для регистра статуса, остальные 5 байтов для данных о температуре и влажности.
    @return 0 в случае успеха, ненулевое значение в случае ошибки.
 **/
int DHT::readTargetData(uint32_t* data) {
    uint8_t statu = 0;
    uint8_t bytes[6] = {0};
    uint8_t cfg_params[] = {0xac, 0x33, 0x00}; // Параметры конфигурации
    //int ret = 0;

    if (_type == DHT10) {

        if (i2cWriteBytes(cfg_params, sizeof(cfg_params))) { // Запись массива байтов в регистр для настройки системы
            return -1;
        }

        delay(75); // Задержка в миллисекундах
        // Проверка флага занятости устройства, бит[7]: 1 для занятости, 0 для простоя.
        while ((statu & 0x80) == 0x80) {
            SERIALPRINT.println("Устройство занято!");
            delay(200); // Задержка в миллисекундах
            if (i2cReadByte(statu)) { // Чтение байта
                return -1;
            }
        }

        if (i2cReadBytes(bytes, sizeof(bytes))) { // Чтение массива байтов
            return -1;
        }


        data[HUMIDITY_INDEX] = (data[HUMIDITY_INDEX] | bytes[1]) << 8;
        data[HUMIDITY_INDEX] = (data[HUMIDITY_INDEX] | bytes[2]) << 8;
        data[HUMIDITY_INDEX] = (data[HUMIDITY_INDEX] | bytes[3]);
        data[HUMIDITY_INDEX] = data[HUMIDITY_INDEX] >> 4;

        data[TEMPRATURE_INDEX] = (data[TEMPRATURE_INDEX] | bytes[3]) << 8;
        data[TEMPRATURE_INDEX] = (data[TEMPRATURE_INDEX] | bytes[4]) << 8;
        data[TEMPRATURE_INDEX] = (data[TEMPRATURE_INDEX] | bytes[5]);
        data[TEMPRATURE_INDEX] &= 0xfffff;

        return 0;
    } else {
        SERIALPRINT.println("Эта функция поддерживается только для DHT10"); // Вывод сообщения об ошибке, если тип датчика не DHT10
        return 0;
    }
}

/** Инициализация DHT10.
    Сброс сенсора и ожидание завершения калибровки.
    @return 0 в случае успеха, ненулевое значение в случае ошибки.
 **/
int DHT::DHT10Init(void) {
    int ret = 0;
    int cnt = 0;

    if (_type == DHT10) {

        delay(500); // Задержка в миллисекундах
        DHT10Reset(); // Сброс сенсора
        delay(300); // Задержка в миллисекундах

        ret = setSystemCfg(); // Установка конфигурации
        if (ret) {
            SERIALPRINT.println("Не удалось установить регистр конфигурации системы \n");
        }
        //SERIALPRINT.println("Настройка системы выполнена успешно!");

        delay(500); // Задержка в миллисекундах

        while (DHT10ReadStatus() == 0) {
            SERIALPRINT.println("Ошибка чтения статуса!");
            DHT10Reset(); // Сброс сенсора
            delay(500); // Задержка в миллисекундах
            if (setSystemCfg()) {
                SERIALPRINT.println("Не удалось установить регистр конфигурации системы \n");
            }
            delay(500); // Задержка в миллисекундах
            cnt++;
            if (cnt > 5) {
                return -1;
            }
        }
        return 0;
    } else {
        SERIALPRINT.println("Эта функция поддерживается только для DHT10"); // Вывод сообщения об ошибке, если тип датчика не DHT10
        return 0;
    }

}



/*****************************************************************************/
/*****************************************************************************/

int DHT::i2cReadByte(uint8_t& byte) {
    int cnt = 0;
    Wire.requestFrom(DEFAULT_IIC_ADDR, 1); // Запрос байта по I2C
    while (1 != Wire.available()) {
        cnt++;
        if (cnt >= 10) {
            return -1;
        }
        delay(1); // Задержка в миллисекундах
    }

    byte = Wire.read(); // Чтение байта
    return 0;
}

int DHT::i2cReadBytes(uint8_t* bytes, uint32_t len) {
    int cnt = 0;
    Wire.requestFrom(DEFAULT_IIC_ADDR, len); // Запрос массива байтов по I2C
    while (len != Wire.available()) {
        cnt++;
        if (cnt >= 10) {
            return -1;
        }
        delay(1); // Задержка в миллисекундах
    }
    for (int i = 0; i < len; i++) {
        bytes[i] = Wire.read(); // Чтение массива байтов
    }
    return 0;
}


int DHT::i2cWriteBytes(uint8_t* bytes, uint32_t len) {
    Wire.beginTransmission(DEFAULT_IIC_ADDR); // Начало передачи по I2C
    for (int i = 0; i < len; i++) {
        Wire.write(bytes[i]); // Запись б
        }
    return Wire.endTransmission();
}

int DHT::i2cWriteByte(uint8_t byte) {
    Wire.beginTransmission(DEFAULT_IIC_ADDR);
    Wire.write(byte);
    return Wire.endTransmission();
}



