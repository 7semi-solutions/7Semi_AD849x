/**
 * 7Semi AD849x Thermocouple Example
 *
 * - Reads AD849x amplifier output using ADC and converts to temperature.
 * - Prints:
 *   - Raw ADC count
 *   - Voltage
 *   - Temperature in °C, °F, K
 *   - Filtered temperature
 *   - Basic fault status
 *
 * Wiring (Typical):
 * - AD849x VOUT -> A0
 * - AD849x VCC  -> 5V
 * - AD849x GND  -> GND
 *
 * Notes:
 * - begin(A0, 5.00, 1023) means:
 *   - Vref = 5.00V
 *   - ADC max count = 1023 (10-bit ADC)
 * - Keep ADC resolution consistent:
 *   - If you use 10-bit ADC, use 1023 (NOT 1024) as the max count.
 * - Offset voltage and sensitivity depend on your module. Verify from datasheet/module.
 */

#include <7Semi_AD849x.h>

AD849x_7Semi thermo;

void setup()
{
    Serial.begin(115200);

    /**
     * Initialize Sensor
     *
     * Parameters:
     * - A0    : Analog pin connected to AD849x VOUT
     * - 5.00  : ADC reference voltage in volts (Vref)
     * - 1023  : ADC max count for 10-bit ADC (0..1023)
     */
    thermo.begin(A0, 5.00, 1023);

    /**
     * Optional configuration (use only what you really need)
     *
     * - setVref():
     *   - Use if your analog reference is different or changes.
     *
     * - setADCResolution():
     *   - Must be ADC max count (10-bit = 1023, 12-bit = 4095).
     *
     * - setOffsetVoltage() & setSensitivity():
     *   - Must match your AD849x module design and datasheet values.
     *
     * - setSampling():
     *   - ADC averaging count. Higher value = smoother voltage, slower update.
     */

    thermo.setVref(5.00);

    thermo.setADCResolution(1023);

    /** Module-specific parameters (example values only) */
    thermo.setOffsetVoltage(1.21);     // Example offset voltage (V at 0°C reference)
    thermo.setSensitivity(0.005);      // 5mV/°C typical (verify your module)

    /** ADC averaging */
    thermo.setSampling(10);

    Serial.println("AD849x Temperature Sensor Ready");
}

void loop()
{
    int raw = thermo.readRaw();
    float voltage = thermo.readVoltage();
    float tempC = thermo.readCelsius();
    float tempF = thermo.readFahrenheit();
    float tempK = thermo.readKelvin();
    float filteredC = thermo.readFilteredTemperatureC(0.10);

    /**
    * Basic sensor connection check
    *
    * - Checks whether the sensor output voltage is within a valid range.
    * - Used only to indicate whether the sensor is likely connected or disconnected.
    *
    * Notes:
    * - This check is based only on output voltage limits.
    * - It does NOT detect exact fault types (open, short, reversed, etc.).
    * - Use this only as a basic connection status indicator.
    */
    bool connected = thermo.FaultDetect();

    Serial.print("RAW: ");
    Serial.print(raw);

    Serial.print(" | Voltage: ");
    Serial.print(voltage, 3);
    Serial.print(" V");

    Serial.print(" | Temp: ");
    Serial.print(tempC, 2);
    Serial.print(" °C");

    Serial.print(" | ");
    Serial.print(tempF, 2);
    Serial.print(" °F");

    Serial.print(" | ");
    Serial.print(tempK, 2);
    Serial.print(" K");

    Serial.print(" | Filtered: ");
    Serial.print(filteredC, 2);
    Serial.print(" °C");

    Serial.print(" | Sensor: ");
    Serial.println(connected ? "OK" : "FAULT");

    delay(1000);
}
