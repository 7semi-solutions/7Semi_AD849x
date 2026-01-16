# 7Semi AD849x Arduino Library

Arduino driver library for **Analog Devices AD849x thermocouple amplifier modules**.

This library provides a clean and lightweight API to read the analog output of AD849x amplifiers and convert it into accurate temperature values with optional filtering and calibration.

---

## Features

- Supports Analog Devices **AD849x thermocouple amplifier family**
  - Commonly used with K-type thermocouples
- ADC-based voltage reading
- Temperature conversion:
  - Celsius (°C)
  - Fahrenheit (°F)
  - Kelvin (K)
- Configurable parameters:
  - ADC reference voltage (Vref)
  - ADC resolution (max count)
  - Output offset voltage
  - Amplifier sensitivity (V/°C)
- Built-in ADC averaging for noise reduction
- Optional exponential (IIR / EMA) filtering for stable temperature output
- One-point temperature calibration support
- Basic sensor connection check (voltage range based)

---

## Important Notes

> ⚠️ Offset voltage and sensitivity depend on the **exact AD849x IC and breakout module**.  
> Always verify values from the datasheet or module documentation.

> ⚠️ The fault check is a **basic connection indicator only**.  
> It does NOT guarantee detection of open, short, or reversed thermocouples.

---

## Installation

### Manual Installation

1. Download or clone this repository
2. Copy the folder into your Arduino `libraries` directory  
   (usually located at `Documents/Arduino/libraries`)
3. Restart the Arduino IDE

### Arduino Library Manager

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for **7Semi_AD849x**
4. Click **Install**

---

