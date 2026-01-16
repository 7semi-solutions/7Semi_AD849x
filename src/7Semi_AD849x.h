/**
 * 7Semi AD849x Thermocouple Amplifier Library
 *
 * - Header file for AD849x_7Semi class.
 * - Provides clean APIs to configure ADC, read voltage, and convert to temperature.
 * - Includes optional averaging, calibration, and filtering support.
 *
 * Default parameters (change if your module differs):
 * - offset_voltage = 1.25V  (commonly represents 0°C reference on many modules)
 * - sensitivity    = 0.005V/°C (5mV/°C typical for AD849x family modules)
 *
 * IMPORTANT:
 * - offset_voltage and sensitivity depend on the exact IC/module. Verify from datasheet/module.
 * - adcResolution in begin() must be ADC maximum count (not "bits"):
 *   - 10-bit: 1023
 *   - 12-bit: 4095
 */

#pragma once

#ifndef _7SEMI_AD849X_H_
#define _7SEMI_AD849X_H_

#include <Arduino.h>

/* ---------- AD849x Class ---------- */
class AD849x_7Semi
{
public:
    /* ---------- Constructor ---------- */
    AD849x_7Semi();

    /* ---------- Initialization ---------- */
    /**
     * begin(analogPin, vRef, adcResolution)
     * - Initializes the library.
     * - Stores ADC pin, reference voltage, and ADC maximum count.
     * - Sets default values for:
     *   - offset = 0.0 (°C calibration offset)
     *   - gain = 1.0 (scaling factor)
     *   - avg_sample = 10 (averaging samples)
     *   - filtered_temperature = NAN (filter starts uninitialized)
     * - Configures the analog pin as INPUT.
     *
     * Parameters:
     * - analogPin: Arduino analog pin connected to AD849x output.
     * - vRef: ADC reference voltage in volts (example: 3.3 or 5.0).
     * - adcResolution: ADC maximum count (example: 1023 for 10-bit, 4095 for 12-bit).
     */
    void begin(uint8_t analogPin,
               float vRef,
               uint16_t adcResolution);

    /* ---------- Configuration ---------- */
    /**
     * setVref(vRef)
     * - Updates ADC reference voltage used in rawToVoltage().
     * - Use if your analog reference changes or differs from default used in begin().
     */
    void setVref(float vRef);

    /**
     * setADCResolution(adcResolution)
     * - Updates ADC maximum count used in rawToVoltage().
     * - This value is the max ADC reading, not the number of bits.
     * - Example: 4095 for 12-bit ADC.
     */
    void setADCResolution(uint16_t adcResolution);

    /**
     * setOffsetVoltage(offset)
     * - Sets offset voltage (in volts) used for temperature conversion.
     * - Temperature equation uses:
     *   tempC = (voltage - offset_voltage) / sensitivity
     */
    void setOffsetVoltage(float offset);

    /**
     * setSensitivity(voltsPerDegC)
     * - Sets amplifier sensitivity in V/°C.
     * - Example typical: 0.005 V/°C (5mV/°C), verify with your device/module.
     */
    void setSensitivity(float voltsPerDegC);

    /**
     * setSampling(samples)
     * - Sets number of ADC samples used for averaging in readRaw().
     * - Range clamped:
     *   - minimum: 1
     *   - maximum: 200
     * - Higher value reduces noise but increases read time.
     */
    void setSampling(uint8_t samples);

    /**
     * getSampling()
     * - Returns the current averaging sample count used by readRaw().
     */
    uint8_t getSampling();

    /* ---------- Reading ---------- */
    /**
     * readRaw()
     * - Reads analog input multiple times (avg_sample) and returns averaged ADC count.
     * - Output range: 0 to resolution (depending on ADC).
     */
    int readRaw();

    /**
     * rawToVoltage(raw)
     * - Converts ADC count to voltage using:
     *   voltage = (raw * reference_voltage) / resolution
     */
    float rawToVoltage(int raw);

    /**
     * readVoltage()
     * - Reads averaged ADC and returns the measured voltage in volts.
     */
    float readVoltage();

    /* ---------- Temperature ---------- */
    /**
     * voltageToCelsius(voltage)
     * - Converts voltage to temperature in °C using:
     *   tempC = (voltage - offset_voltage) / sensitivity
     * - Applies calibration/scaling:
     *   tempC = (tempC * gain) + offset
     */
    float voltageToCelsius(float voltage);

    /**
     * readCelsius()
     * - Reads voltage from ADC and returns temperature in °C.
     */
    float readCelsius();

    /**
     * readFahrenheit()
     * - Reads temperature in °C and converts to °F using:
     *   °F = (°C * 9/5) + 32
     */
    float readFahrenheit();

    /**
     * readKelvin()
     * - Reads temperature in °C and converts to Kelvin using:
     *   K = °C + 273.15
     */
    float readKelvin();

    /* ---------- Calibration ---------- */
    /**
     * calibrate(actualTempC)
     * - One-point calibration.
     * - Reads current temperature and adjusts internal offset so the output matches actualTempC.
     * - Useful when you have a known reference temperature.
     */
    void calibrate(float actualTempC);

    /* ---------- Filtering ---------- */
    /**
     * readFilteredTemperatureC(alpha)
     * - Returns exponentially filtered temperature (IIR / EMA filter).
     * - alpha range:
     *   - 1.0 -> no filtering (fast response)
     *   - 0.0 -> output stuck (not useful)
     * - Typical: 0.05 to 0.30
     * - First call initializes the filter with the current temperature.
     * * Alpha Value | Filter Behavior | Recommended Use
     * ------------|-----------------|------------------------------------
     * 0.05        | Very smooth     | Industrial environments, high noise
     * 0.10        | Smooth          | General thermocouple applications
     * 0.20        | Balanced        | Faster response with moderate noise
     * 0.30        | Fast response   | Low-noise systems
     * 1.00        | No filtering    | Debugging and raw data inspection
     * ------------|-----------------|------------------------------------
     */
    float readFilteredTemperatureC(float alpha = 0.1);

    /* ---------- Diagnostics ---------- */
    /**
     * FaultDetect()
     * - Basic output sanity check.
     * - Returns:
     *   - 1 if voltage is within (0.1V .. Vref-0.1V)
     *   - 0 otherwise
     * - Not a guaranteed open/short detection; only a quick health check.
     */
    uint8_t FaultDetect();

private:
    /* ---------- Hardware ---------- */
    uint8_t analog_pin;

    /* ---------- ADC ---------- */
    float reference_voltage;
    uint16_t resolution;

    /* ---------- AD849x Parameters ---------- */
    float offset_voltage = 1.25;   // Default reference voltage at 0°C (verify with your module)
    float sensitivity = 0.005;     // Default sensitivity in V/°C (verify with your module)

    /* ---------- Calibration ---------- */
    float offset;
    float gain;

    /* ---------- Filtering & Sampling ---------- */
    float filtered_temperature;
    uint8_t avg_sample = 10;
};
#endif
