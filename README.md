# 7Semi ENS210 Library

Arduino library for the **ENS210 High-Accuracy Temperature and Humidity Sensor**.

Designed for reliability, CRC validation, and cross-platform compatibility (ESP32, AVR, and other Arduino-compatible boards).

---

##  Features

-  Temperature in Celsius
-  Relative Humidity (%)  
-  Absolute Humidity (g/m³)  
-  CRC7 data validation  
-  Single-shot mode  
-  Continuous measurement mode  
-  I2C clock configuration  
-  Custom SDA/SCL support (ESP32)  

---

##  Supported Platforms

- ESP32  
- AVR (UNO, Nano, Mega)  
- Any board supporting `Wire` (I2C)

---

##  Hardware Connection

| ENS210 Pin | Arduino / ESP32 |
|------------|------------------|
| VDD        | 3.3V             |
| GND        | GND              |
| SDA        | SDA              |
| SCL        | SCL              |

Default I2C Address: `0x43`

---

##  Installation

### Method 1 – Manual Installation

1. Download ZIP from GitHub
2. Extract into your Arduino `libraries` folder
3. Restart Arduino IDE

### Method 2 – Arduino Library Manager (Future Release)

Search for:
