/*
   7Semi ENS210 Continuous Mode Example
   ----------------------------------------
   Demonstrates continuous measurement mode
   using data-ready polling instead of fixed delay.

   I2C Connection (Default Address: 0x43)

   ENS210    ->   Arduino
   ---------------------------
   VDD       ->   3.3V
   GND       ->   GND
   SDA       ->   SDA
   SCL       ->   SCL

   Notes:
   - Sensor operates at 3.3V
   - Default I2C address: 0x43
   - Continuous mode automatically updates measurements
*/

#include <7Semi_ENS210.h>

/* ---------------- I2C Configuration ---------------- */

// Default I2C address
#define ENS210_I2C_ADDR 0x43

// I2C clock speed (100000 = 100 kHz, 400000 = 400 kHz Fast Mode)
#define ENS210_I2C_CLOCK 400000

/*
   I2C Pin Configuration

   For most Arduino boards:
   - Use default pins
   - Keep values as 255

   For ESP32:
   - Set custom SDA/SCL pins below
*/

#define ENS210_I2C_SDA 255
#define ENS210_I2C_SCL 255

// Example for ESP32 custom pins:
// #define ENS210_I2C_SDA   21
// #define ENS210_I2C_SCL   22

/* --------------------------------------------------- */


ENS210_7Semi ens;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ENS210 Continuous Mode Example");

  /*
       Initialize sensor

       Parameters:
       - I2C address
       - Wire instance
       - I2C clock speed
       - SDA pin (ESP32 only)
       - SCL pin (ESP32 only)
    */
  if (!ens.begin(ENS210_I2C_ADDR,
                 Wire,
                 ENS210_I2C_CLOCK,
                 ENS210_I2C_SDA,
                 ENS210_I2C_SCL)) {
    Serial.println("Sensor not detected!");
    while (1)
      ;
  }

  /*
       Start continuous measurement

       - Sensor automatically performs repeated conversions
    */
  if (!ens.startContinuous()) {
    Serial.println("Failed to start continuous mode!");
    while (1)
      ;
  }

  Serial.println("Continuous mode started.");
}

void loop() {
  float temperature;
  float humidity;

  if (ens.read(temperature, humidity)) {
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.print(" °C  |  Humidity: ");
    Serial.print(humidity, 2);
    Serial.println(" %");
  } else {
    Serial.println("Read error (CRC or I2C)");
  }
  delay(50);
}