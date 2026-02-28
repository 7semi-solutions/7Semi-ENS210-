/*
   7Semi ENS210 Example
   ----------------------------------------
   Demonstrates single-shot temperature and humidity reading.

   I2C Connection (Default Address: 0x43)

   ENS210    ->   Arduino
   ---------------------------
   VDD       ->   3.3V
   GND       ->   GND
   SDA       ->   SDA
   SCL       ->   SCL

   Notes:
   - Sensor is 3.3V device (do NOT use 5V unless board has level shifting)
   - Default I2C address: 0x43
   - I2C speed set to 400kHz (Fast Mode)
*/

#include <7Semi_ENS210.h>

#define ENS210_I2C_ADDR 0x43

ENS210_7Semi ens;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("ENS210 Temperature & Humidity Example");

    /*
       Initialize sensor

       Parameters:
       - I2C address
       - Wire instance
       - I2C clock frequency (400000 = 400 kHz)
    */
    if (!ens.begin(ENS210_I2C_ADDR, Wire, 400000))
    {
        Serial.println("Sensor not detected!");
        Serial.println("Check wiring and power supply.");
        while (1); // Stop execution
    }

    Serial.println("Sensor initialized successfully!");
}

void loop()
{
    /*
       Trigger single-shot measurement

       - Sensor performs one temperature and humidity conversion
    */
    ens.startSingleShot();

    /*
       Wait for conversion time

       Typical conversion time: ~130 ms
       Using 150 ms for safety margin
    */
    delay(150);

    float temperature;
    float humidity;

    /*
       Read temperature and humidity

       - Temperature returned in Celsius
       - Humidity returned in percent
       - CRC validated internally
    */
    if (!ens.read(temperature, humidity))
    {
        Serial.println("Read failed (CRC or I2C error)");
    }
    else
    {
        Serial.print("Temperature: ");
        Serial.print(temperature, 2);
        Serial.print(" °C  |  Humidity: ");
        Serial.print(humidity, 2);
        Serial.println(" %");
    }

    delay(2000); // Repeat every 2 seconds
}