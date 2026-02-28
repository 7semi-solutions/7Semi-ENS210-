#ifndef _7SEMI_ENS210_H_
#define _7SEMI_ENS210_H_

#include <Arduino.h>
#include <Wire.h>

/**
 * Default I2C address of ENS210
 *
 * - 7-bit address
 * - Can be changed if hardware supports it
 */
#define ENS210_DEFAULT_ADDRESS 0x43

/**
 * ENS210 Register Map
 *
 * - Used internally for I2C communication
 */
#define ENS210_REG_PART_ID     0x00
#define ENS210_REG_DIE_REV     0x02
#define ENS210_REG_UID         0x04
#define ENS210_REG_SYS_CTRL    0x10
#define ENS210_REG_SYS_STAT    0x11
#define ENS210_REG_SENS_RUN    0x21
#define ENS210_REG_SENS_START  0x22
#define ENS210_REG_SENS_STOP   0x23
#define ENS210_REG_SENS_STAT   0x24
#define ENS210_REG_T_VAL       0x30
#define ENS210_REG_H_VAL       0x33

/**
 * Boot and conversion timing (milliseconds)
 *
 * - Boot time required after reset
 * - Single-shot temperature/humidity conversion time
 * - Continuous conversion period
 */
#define ENS210_BOOTING_TIME_MS     3
#define ENS210_THCONV_SINGLE_MS    130
#define ENS210_THCONV_CONT_MS      130

/**
 * System status register masks
 *
 * - ACTIVE mask indicates measurement ready
 */
#define ENS210_SYS_ACTIVE_MASK     0x01

/**
 * ENS210 Temperature & Humidity Sensor Class
 *
 * - Supports single-shot and continuous modes
 * - Provides temperature in Kelvin, Celsius, Fahrenheit
 * - Provides relative and absolute humidity
 * - Performs CRC7 validation automatically
 * - Designed for 7Semi professional embedded libraries
 */
class ENS210_7Semi {
public:

    /**
     * Constructor
     *
     * - Does not initialize hardware
     * - begin() must be called before use
     */
    ENS210_7Semi();

    /**
     * Initialize sensor
     *
     * - Configures I2C interface
     * - Verifies device presence
     * - Reads device ID
     * - Performs reset
     *
     * - Returns false if sensor not detected
     */
    bool begin(uint8_t i2cAddress = ENS210_DEFAULT_ADDRESS,
               TwoWire &i2cPort = Wire,
               uint32_t i2cClock = 400000,
               uint8_t sda = 255,
               uint8_t scl = 255);

    /**
     * Perform software reset
     */
    bool reset();

    /**
     * Start single-shot measurement
     */
    bool startSingleShot();

    /**
     * Start continuous measurement mode
     */
    bool startContinuous();

    /**
     * Stop continuous measurement
     */
    bool stop();

    /**
     * Check if new measurement is available
     */
    bool available();

    /**
     * Read raw sensor values
     *
     * - Outputs raw temperature and humidity
     * - CRC validated
     */
    bool readRaw(uint32_t &temperature_raw, uint32_t &humidity_raw);

    /**
     * Read sensor values
     *
     * - Outputs temperature in Celceus and humidity in percentage
     * - CRC validated
     */
    bool read(float &temperatureC, float &humidityPercent);

    /**
     * Absolute humidity in g/m³
     */
    float getAbsoluteHumidity();

    /**
     * Read device part ID
     */
    bool readDeviceId(uint16_t &partID);

    /**
     * Read silicon die revision
     */
    bool readDieRevision(uint8_t &dieRev);

    /**
     * Read unique 64-bit device ID
     */
    bool readUID(uint8_t *uid);

    /**
     * Enable or disable debug output
     *
     * - Useful during development
     */
    void enableDebug(bool dbg) { debugENS210 = dbg; }

private:
    /**
     * I2C device address
     */
    uint8_t address;

    /**
     * Pointer to I2C interface
     */
    TwoWire *i2c;

    /**
     * Debug flag
     */
    bool debugENS210 = false;

    /**
     * Cached device information
     */
    uint16_t part_id;
    uint8_t die_revision;
    uint8_t uid[8];

    /**
     * Low-level register read
     */
    bool readReg(uint8_t reg, uint8_t *data, uint8_t len);

    /**
     * Low-level register write
     */
    bool writeReg(uint8_t reg, uint8_t value);

    /**
     * CRC7 calculation
     *
     * - Validates 17-bit payload
     */
    uint8_t getCRC7(uint32_t payload);
};

#endif