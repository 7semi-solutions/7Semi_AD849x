/**
 * 7Semi AD849x Thermocouple Amplifier Library
 *
 * - Supports AD8494 / AD8495 style thermocouple amplifier breakout boards.
 * - Reads the amplifier output using an MCU ADC pin and converts it to temperature.
 * - Provides:
 *   - Raw ADC averaging (simple moving average)
 *   - Voltage conversion using Vref and ADC resolution
 *   - Temperature conversion using offset voltage + sensitivity
 *   - Optional calibration offset and IIR (exponential) filtering
 *
 * Notes / Quick Setup:
 * - Call begin(pin, vRef, adcResolution) once in setup().
 * - Set the correct offset voltage and sensitivity for your exact IC/module:
 *   - AD8495 is typically ~5mV/°C sensitivity (check your datasheet/module)
 *   - Offset voltage depends on module design (some are centered, some are not)
 * - If your ADC is 12-bit, adcResolution should be 4095 (max count), not 4096.
 *
 * Example:
 *   AD849x_7Semi tc;
 *   tc.begin(A0, 3.3, 4095);
 *   tc.setOffsetVoltage(1.25);
 *   tc.setSensitivity(0.005);
 *   float tC = tc.readCelsius();
 */

#include "7Semi_AD849x.h"

/* ---------- Constructor ---------- */
AD849x_7Semi::AD849x_7Semi()
{
    /** 
     * - Empty constructor.
     * - Real initialization is done in begin().
     */
}

/* ---------- Initialization ---------- */
void AD849x_7Semi::begin(uint8_t analogPin,
                         float vRef,
                         uint16_t adcResolution)
{
    /**
     * - Stores configuration for ADC reading and conversions.
     * - Initializes defaults:
     *   - offset = 0.0 (calibration offset in °C)
     *   - gain   = 1.0 (scaling factor)
     *   - avg_sample = 10 (ADC averaging count)
     *   - filtered_temperature = NAN (filter starts uninitialized)
     * - Configures the ADC input pin as INPUT.
     *
     * Important:
     * - adcResolution should be the maximum ADC count:
     *   - 10-bit: 1023
     *   - 12-bit: 4095
     */
    analog_pin = analogPin;
    reference_voltage = vRef;
    resolution = adcResolution;

    offset = 0.0;
    gain = 1.0;

    filtered_temperature = NAN;
    avg_sample = 10;

    pinMode(analog_pin, INPUT);
}

/* ---------- Configuration ---------- */
void AD849x_7Semi::setVref(float vRef)
{
    /**
     * - Sets ADC reference voltage used for raw -> voltage conversion.
     * - Example: 3.3V or 5.0V depending on your MCU analog reference.
     */
    reference_voltage = vRef;
}

void AD849x_7Semi::setADCResolution(uint16_t adcResolution)
{
    /**
     * - Sets ADC maximum count (not "number of bits").
     * - Example:
     *   - 10-bit ADC: 1023
     *   - 12-bit ADC: 4095
     */
    resolution = adcResolution;
}

void AD849x_7Semi::setOffsetVoltage(float offset)
{
    /**
     * - Sets the amplifier output offset voltage (in volts).
     * - This value depends on your AD849x IC and breakout/module design.
     * - Temperature conversion uses:
     *   tempC = (voltage - offset_voltage) / sensitivity
     */
    offset_voltage = offset;
}

void AD849x_7Semi::setSensitivity(float voltsPerDegC)
{
    /**
     * - Sets amplifier sensitivity in volts/°C.
     * - Typical example (verify from datasheet/module):
     *   - 0.005 V/°C (5mV/°C)
     */
    sensitivity = voltsPerDegC;
}

void AD849x_7Semi::setSampling(uint8_t samples)
{
    /**
     * - Sets number of ADC samples to average.
     * - Useful to reduce noise on the analog reading.
     * - Limits:
     *   - Minimum: 1
     *   - Maximum: 200 (to avoid long blocking reads)
     */
    if (samples == 0) samples = 1;
    if (samples > 200) samples = 200;
    avg_sample = samples;
}

uint8_t AD849x_7Semi::getSampling()
{
    /**
     * - Returns the current averaging sample count.
     */
    return avg_sample;
}

/* ---------- Reading ---------- */
int AD849x_7Semi::readRaw()
{
    /**
     * - Reads ADC multiple times and returns the averaged raw ADC count.
     * - Output range depends on your ADC resolution:
     *   - 0 to resolution (e.g., 0..4095)
     */
    uint32_t value = 0;

    for (uint8_t i = 0; i < avg_sample; i++)
    {
        value += analogRead(analog_pin);
    }

    return value / avg_sample;
}

float AD849x_7Semi::rawToVoltage(int raw)
{
    /**
     * - Converts raw ADC count to voltage using:
     *   voltage = (raw * Vref) / resolution
     *
     * Note:
     * - resolution must be the ADC max count (1023, 4095, etc.)
     */
    return (raw * reference_voltage) / resolution;
}

float AD849x_7Semi::readVoltage()
{
    /**
     * - Reads averaged ADC count and converts it to volts.
     */
    return rawToVoltage(readRaw());
}

/* ---------- Temperature ---------- */
float AD849x_7Semi::voltageToCelsius(float voltage)
{
    /**
     * - Converts amplifier output voltage to °C.
     * - Base formula:
     *   tempC = (voltage - offset_voltage) / sensitivity
     *
     * - Then applies user calibration:
     *   tempC = (tempC * gain) + offset
     *
     * Where:
     * - offset is updated by calibrate() (single point calibration)
     * - gain can be used for scaling (if you add a gain calibration in future)
     */
    float temp = (voltage - offset_voltage) / sensitivity;
    temp = (temp * gain) + offset;
    return temp;
}

float AD849x_7Semi::readCelsius()
{
    /**
     * - Reads voltage from ADC and converts it to °C.
     */
    return voltageToCelsius(readVoltage());
}

float AD849x_7Semi::readFahrenheit()
{
    /**
     * - Returns temperature in °F using:
     *   °F = (°C * 9/5) + 32
     */
    return (readCelsius() * 9.0 / 5.0) + 32.0;
}

float AD849x_7Semi::readKelvin()
{
    /**
     * - Returns temperature in Kelvin using:
     *   K = °C + 273.15
     */
    return readCelsius() + 273.15;
}

/* ---------- Calibration ---------- */
void AD849x_7Semi::calibrate(float actualTempC)
{
    /**
     * - Simple 1-point calibration.
     * - Measures current temperature and adjusts internal offset so that:
     *   measured + offset == actualTempC
     *
     * Usage example:
     * - Put thermocouple in a known stable temperature point
     *   (e.g., ice bath ~0°C, boiling water ~100°C at sea level),
     *   then call calibrate(knownTempC).
     */
    float measured = readCelsius();
    offset = actualTempC - measured;
}

/* ---------- Filtering ---------- */
float AD849x_7Semi::readFilteredTemperatureC(float alpha)
{
    /**
     * - Exponential moving average (IIR low-pass filter).
     * - alpha range:
     *   - 1.0 : no filtering (output follows current reading)
     *   - 0.0 : output never changes (not useful)
     *   - Typical: 0.05 to 0.3 depending on noise and response needed
     *
     * Formula:
     *   filtered = alpha * current + (1 - alpha) * previous_filtered
     *
     * Notes:
     * - On first call, it initializes filter output to the current reading.
     */
    float current = readCelsius();

    if (isnan(filtered_temperature))
    {
        filtered_temperature = current;
    }

    filtered_temperature = alpha * current + (1.0 - alpha) * filtered_temperature;
    return filtered_temperature;
}

/* ---------- Diagnostics ---------- */
uint8_t AD849x_7Semi::FaultDetect()
{
    /**
     * - Basic "sanity window" check for amplifier output voltage.
     * - Returns 1 when voltage looks valid, 0 otherwise.
     *
     * What it does:
     * - Reads voltage and checks it is not too close to 0V or Vref.
     * - Thresholds used:
     *   - lower: 0.1V
     *   - upper: (Vref - 0.1V)
     *
     * Notes:
     * - This is NOT a guaranteed open/short thermocouple detection method.
     * - Real fault behavior depends on your AD849x, thermocouple wiring,
     *   and breakout board design.
     * - Use this as a quick health check, not a safety-critical detector.
     */
    float v = readVoltage();
    return (v > 0.1 && v < (reference_voltage - 0.1));
}
