#include "7Semi_ENS210.h"

ENS210_7Semi::ENS210_7Semi() {

}

/**
 * Initialize ENS210 sensor
 *
 * - Configures I2C port
 * - Sets clock frequency
 * - Verifies device presence
 * - Reads device ID
 * - Performs soft reset
 *
 * - Returns false if device not detected
 */
bool ENS210_7Semi::begin(uint8_t i2cAddress,
                         TwoWire &i2cPort,
                         uint32_t i2cClock,
                         uint8_t sda,
                         uint8_t scl)
{
    i2c = &i2cPort;
    address = i2cAddress;

#if defined(ARDUINO_ARCH_ESP32)
    /**
     * ESP32 allows custom SDA/SCL pins
     */
    if (sda != 255 && scl != 255) i2c->begin(sda, scl);
    else i2c->begin();
#else
    /**
     * Other platforms use default I2C pins
     */
    i2c->begin();
    (void)sda;a
    (void)scl;
#endif

    i2c->setClock(i2cClock);
    delay(10);

    /**
     * Check if device responds on I2C bus
     */
    i2c->beginTransmission(address);
    if (i2c->endTransmission() != 0) return false;

    /**
     * Read Part ID for verification
     */
    if (!readDeviceId(part_id)) return false;

    /**
     * Perform sensor reset
     */
    if (!reset()) return false;

    /*
    * Disable low power mode
    */
    if (!writeReg(ENS210_REG_SYS_CTRL, 0x00))
        return false;

    /*
    * Enable temperature + humidity engine
    */
    if (!writeReg(ENS210_REG_SENS_RUN, 0x03))
        return false;
    delay(150);  // allow first conversion

    return true;
}

/**
 * Perform soft reset
 *
 * - Sends reset command
 * - Waits for boot time
 */
bool ENS210_7Semi::reset() {
    if (!writeReg(ENS210_REG_SYS_CTRL, 0x80)) return false;
    delay(ENS210_BOOTING_TIME_MS);
    return true;
}

/**
 * Start single-shot measurement
 *
 * - Triggers one temperature and humidity conversion
 */
bool ENS210_7Semi::startSingleShot() {
    return writeReg(ENS210_REG_SENS_START, 3);
}

/**
 * Start continuous measurement
 *
 * - Enables automatic repeated measurements
 */
bool ENS210_7Semi::startContinuous() {

    // Enable temperature + humidity engine
    if (!writeReg(ENS210_REG_SENS_RUN, 0x03))
        return false;

    // Trigger first measurement
    if (!writeReg(ENS210_REG_SENS_START, 0x03))
        return false;

    return true;
}

/**
 * Stop measurement
 *
 * - Disables continuous mode
 */
bool ENS210_7Semi::stop() {
    return writeReg(ENS210_REG_SENS_RUN, 0);
}

/**
 * Check system availability
 *
 * - Reads system status register
 * - Returns true if data is ready
 */
bool ENS210_7Semi::available() {
    uint8_t status;
    if (!readReg(ENS210_REG_SYS_STAT, &status, 1)) return false;
    return (status & 0x01);
}

/**
 * Read raw temperature and humidity values
 *
 * - Reads 6 bytes starting from temperature register
 * - Extracts 17-bit payload for each measurement
 * - Validates CRC7
 * - Checks valid bit
 * - Outputs 16-bit raw values
 *
 * - Returns false if CRC fails or data invalid
 */
/**
 * Read raw temperature and humidity registers
 *
 * - Extracts 17-bit payload (bit 16 = valid flag)
 * - Validates CRC7 for both values
 * - Outputs 16-bit measurement data
 * - Returns false if CRC or valid bit fails
 */
bool ENS210_7Semi::readRaw(uint32_t &temperature_raw,
                           uint32_t &humidity_raw)
{
    uint8_t buf[6];

    if (!readReg(ENS210_REG_T_VAL, buf, 6))
        return false;
    Serial.print("Raw T/H bytes: ");
    for (int i = 0; i < 6; i++) {
        if (buf[i] < 0x10) Serial.print("0"); // leading zero for formatting
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();


    /*
     * -------- Temperature Extraction --------
     */
    uint32_t temperature =
        ((uint32_t)(buf[2] & 0x01) << 16) |
        ((uint32_t)buf[1] << 8) |
        buf[0];

    uint8_t temp_crc = (buf[2] >> 1) & 0x7F;

    if ((temperature & 0x10000) == 0)        // Valid bit check
        return false;

    if (getCRC7(temperature) != temp_crc)    // CRC check
        return false;

    temperature_raw = temperature & 0xFFFF;


    /*
     * -------- Humidity Extraction --------
     */
    uint32_t humidity =
        ((uint32_t)(buf[5] & 0x01) << 16) |
        ((uint32_t)buf[4] << 8) |
        buf[3];

    uint8_t hum_crc = (buf[5] >> 1) & 0x7F;

    if ((humidity & 0x10000) == 0)
        return false;

    if (getCRC7(humidity) != hum_crc)
        return false;

    humidity_raw = humidity & 0xFFFF;

    return true;
}
/**
 * Read temperature and humidity in one transaction
 *
 * - Performs single I2C read of sensor data
 * - Temperature returned in Celsius
 * - Humidity returned in percent
 * - Returns false if CRC or communication fails
 */
bool ENS210_7Semi::read(float &temperatureC,
                        float &humidityPercent)
{
    uint32_t temperature_raw;
    uint32_t humidity_raw;

    if (!readRaw(temperature_raw, humidity_raw))
        return false;

    /*
     * Temperature raw resolution:
     * - 1 LSB = 1/64 Kelvin
     */
    float temperatureK = (temperature_raw & 0xFFFF) / 64.0;

    /*
     * Humidity raw resolution:
     * - 1 LSB = 1/512 percent
     */
    float humidity = (humidity_raw & 0xFFFF) / 512.0;

    temperatureC = temperatureK - 273.15;
    humidityPercent = humidity;

    return true;
}

/**
 * Calculate absolute humidity (g/m³)
 *
 * - Uses Magnus formula
 * - Based on temperature and relative humidity
 */
/**
 * Get absolute humidity in g/m³
 *
 * - Uses Magnus formula approximation
 * - Requires valid temperature and humidity reading
 * - Returns NAN if sensor read fails
 */
float ENS210_7Semi::getAbsoluteHumidity() {

    float temperature;
    float humidity;

    if (!read(temperature, humidity))
        return NAN;

    /*
     * Saturation vapor pressure (hPa)
     * Magnus approximation
     */
    float es = 6.112 * exp((17.67 * temperature) / (temperature + 243.5));

    /*
     * Absolute humidity (g/m³)
     */
    return (2.1674 * es * humidity) / (273.15 + temperature);
}
/**
 * Read device Part ID
 *
 * - 16-bit identifier
 */
bool ENS210_7Semi::readDeviceId(uint16_t &partID) {
    uint8_t buf[2];
    if (!readReg(ENS210_REG_PART_ID, buf, 2)) return false;
    partID = ((uint16_t)buf[1] << 8) | buf[0];
    return true;
}

/**
 * Read die revision
 *
 * - 8-bit revision value
 */
bool ENS210_7Semi::readDieRevision(uint8_t &dieRev) {
    return readReg(ENS210_REG_DIE_REV, &dieRev, 1);
}

/**
 * Read unique device ID
 *
 * - 8-byte factory-programmed value
 */
bool ENS210_7Semi::readUID(uint8_t *uid) {
    return readReg(ENS210_REG_UID, uid, 8);
}

/**
 * Read register(s) over I2C
 *
 * - Sends register address
 * - Requests specified number of bytes
 * - Stores data into buffer
 */
bool ENS210_7Semi::readReg(uint8_t reg, uint8_t *data, uint8_t len) {
    // if (!i2c) return false;
    i2c->beginTransmission(address);
    i2c->write(reg);
    if (i2c->endTransmission(false) != 0) return false;

    i2c->requestFrom(address, len);

    if (i2c->available() != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        data[i] = i2c->read();

    return true;
}

/**
 * Write single register over I2C
 *
 * - Sends register address
 * - Sends value
 */
bool ENS210_7Semi::writeReg(uint8_t reg, uint8_t value) {
    if (!i2c) return false;

    i2c->beginTransmission(address);
    i2c->write(reg);
    i2c->write(value);

    return (i2c->endTransmission() == 0);
}

/**
 * CRC7 Calculation for ENS210
 *
 * - Polynomial: x^7 + x^3 + 1
 * - Polynomial value: 137
 * - Initial vector: 127
 * - Payload size: 17 bits
 * - Uses polynomial long division method
 * - Returns 7-bit CRC
 */
uint8_t ENS210_7Semi::getCRC7(uint32_t val)
{
    uint32_t pol = 137;
    pol <<= 9;

    uint32_t bit = 65536;

    val <<= 7;
    bit <<= 7;
    pol <<= 7;

    val |= 127;

    /**
     * Perform polynomial division
     */
    while (bit & (131071 << 7))
    {
        if (val & bit)
            val ^= pol;

        bit >>= 1;
        pol >>= 1;
    }

    return (uint8_t)(val & 127);
}
