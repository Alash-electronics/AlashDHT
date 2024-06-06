# AlashDHT
AlashDHT — это библиотека для Arduino, ESP8266 и ESP32, которая позволяет легко считывать данные о температуре и влажности с цифровых датчиков DHT (DHT11, DHT21, DHT22, DHT10, DHT20), обеспечивая простую интеграцию и точные измерения для различных проектов.


Вот описание каждой функции библиотеки "AlashDHT.h" с примерами использования:

1. `DHT()`: Конструктор класса DHT, создает объект для работы с датчиком влажности и температуры.
   
DHT dht(DHTPIN, DHTTYPE);


3. `begin()`: Инициализирует работу с датчиком влажности и температуры.

dht.begin();


4. `readTemperature()`: Считывает температуру в градусах Цельсия.

float temperature = dht.readTemperature();


5. `convertCtoF()`: Конвертирует температуру из градусов Цельсия в градусы Фаренгейта.

float temperature_F = dht.convertCtoF(temperature_C);


6. `readHumidity()`: Считывает влажность в процентах.

float humidity = dht.readHumidity();


7. `readTempAndHumidity()`: Считывает и одновременно температуру и влажность.

float temp_hum_val[2] = {0};
if (!dht.readTempAndHumidity(temp_hum_val)) {
    float temperature = temp_hum_val[0];
    float humidity = temp_hum_val[1];
    // Далее используйте значения temperature и humidity
} else{     // Обработка ошибки
}


8. `DHT10Reset()`: Сброс состояния датчика DHT10.

dht.DHT10Reset();


9. `DHT10ReadStatus()`: Чтение статуса датчика DHT10.

uint8_t status = dht.DHT10ReadStatus();


10. `setSystemCfg()`: Устанавливает конфигурацию системы для датчика DHT10.

dht.setSystemCfg();


11. `readTargetData()`: Читает целевые данные из датчика DHT10.

uint8_t data[5];
dht.readTargetData(data);


12. `DHT10Init()`: Инициализация датчика DHT10.

dht.DHT10Init();


Эти функции предоставляют возможность управления и считывания данных с датчика влажности и температуры. Используйте их в своем коде в зависимости от ваших потребностей.
