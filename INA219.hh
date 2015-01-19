/**
 * @file INA219.hh
 * @version 0.8
 *
 * @section License
 * Copyright (C) 2014-2015, jeditekunum
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef COSA_INA219_HH
#define COSA_INA219_HH

#include "Cosa/TWI.hh"

/**
 * Driver for the INA219 2-wire current sensor.
 *
 */
class INA219 : private TWI::Driver {
public:
  /**
   * Range.
   */
  enum range_t {
    RANGE_32V,
    RANGE_16V,
  } __attribute__((packed));

  /**
   * Construct INA219 TWI current sensor device access to given
   * chip address.
   * @param[in] addr chip address (0..7)
   * @param[in] resistor shunt in milli-ohms
   */
  INA219(uint8_t addr = 0, uint16_t resistor = 100) :
    TWI::Driver(0x40 | (addr & 0x7)),
    m_resistor(resistor),
    m_active(false)
  {}

  /**
   * Begin.
   * @param[in] range.
   */
  void begin(range_t range = RANGE_32V);

  /**
   * End.
   */
  void end();

  /**
   * Get bus voltage in mV.
   */
  uint32_t bus()
    __attribute__((always_inline))
  {
    return ((read_register(BUS_VOLTAGE) >> 3) * 4);
  }

  /**
   * Get shunt voltage in uV.
   */
  uint16_t shunt()
    __attribute__((always_inline))
  {
    return (read_register(SHUNT_VOLTAGE) * 10); // 10uV resolution
  }

  /**
   * Calc current in tenths of mA from uV.
   */
  uint16_t current(uint16_t uv);

  /**
   * Get current in tenths of mA.
   */
  uint16_t current()
    __attribute__((always_inline))
  {
    return (current(shunt()));
  }

  /**
   * Calc power in tenths of mW from uV.
   */
  uint16_t power(uint16_t uv);

  /**
   * Get power in tenths of mW.
   */
  uint16_t power()
    __attribute__((always_inline))
  {
    return (power(shunt()));
  }

  /**
   * Get rate of bus value updates (us per conversion).
   */
  uint32_t bus_time()
    __attribute__((always_inline))
  {
    return (m_bus_conversion_time);
  }

  /**
   * Get rate of shunt value updates (us per conversion).
   */
  uint32_t shunt_time()
    __attribute__((always_inline))
  {
    return (m_shunt_conversion_time);
  }

protected:
  /**
   * Registers
   */
  enum register_t {
    CONFIG        = 0x0, // R/W
    SHUNT_VOLTAGE = 0x1, // R
    BUS_VOLTAGE   = 0x2  // R
  }; __attribute__((packed));

  /** Resistor in milli-ohms */
  uint16_t m_resistor;

  /** Active? */
  bool m_active;

  /** Last range */
  range_t m_range;

  /** Get conversion time resolution (make sure to mask) */
  uint32_t conversion(uint16_t adc_resolution);

  /**
   * Wait for conversion
   */
  void wait(register_t reg);

  /** Configure */
  void configure(uint16_t config);

  /** Last config value - for wait */
  uint16_t m_config_value;

  /** Min conversion times in ms */
  uint32_t m_bus_conversion_time;
  uint32_t m_shunt_conversion_time;

  /**
   * Write register
   */
  void write_register(register_t reg, uint16_t word);

  /**
   * Read register
   * @param[in] register
   */
  uint16_t read_register(register_t reg);

  /**
   * Config Register
   */
  enum {
    RESET                                   = 0x8000,

    BUS_VOLTAGE_RANGE_MASK                  = 0x2000,
    BUS_VOLTAGE_RANGE_16V                   = 0x0000,
    BUS_VOLTAGE_RANGE_32V                   = 0x2000,

    GAIN_MASK                               = 0x1800,
    GAIN_1_40MV                             = 0x0000,
    GAIN_2_80MV                             = 0x0800,
    GAIN_4_160MV                            = 0x1000,
    GAIN_8_320MV                            = 0x1800,

    BUS_ADC_RESOLUTION_MASK                 = 0x0780,
    BUS_ADC_RESOLUTION_9BIT                 = 0x0080,  // 1 x 9-bit bus sample
    BUS_ADC_RESOLUTION_10BIT                = 0x0100,  // 1 x 10-bit bus sample
    BUS_ADC_RESOLUTION_11BIT                = 0x0200,  // 1 x 11-bit bus sample
    BUS_ADC_RESOLUTION_12BIT                = 0x0180,  // 1 x 12-bit bus sample (power-on default)
    BUS_ADC_RESOLUTION_12BIT_1S_532US       = 0x0400,  // 1 x 12-bit bus sample
    BUS_ADC_RESOLUTION_12BIT_2S_1060US      = 0x0480,  // 2 x 12-bit bus samples, averaged
    BUS_ADC_RESOLUTION_12BIT_4S_2130US      = 0x0500,  // 4 x 12-bit bus samples, averaged
    BUS_ADC_RESOLUTION_12BIT_8S_4260US      = 0x0580,  // 8 x 12-bit bus samples, averaged
    BUS_ADC_RESOLUTION_12BIT_16S_8510US     = 0x0600,  // 16 x 12-bit bus samples, averaged
    BUS_ADC_RESOLUTION_12BIT_32S_17MS       = 0x0680,  // 32 x 12-bit bus samples, averaged
    BUS_ADC_RESOLUTION_12BIT_64S_34MS       = 0x0700,  // 64 x 12-bit bus samples, averaged
    BUS_ADC_RESOLUTION_12BIT_128S_69MS      = 0x0780,  // 128 x 12-bit bus samples, averaged

    SHUNT_ADC_RESOLUTION_MASK               = 0x0078,
    SHUNT_ADC_RESOLUTION_9BIT               = 0x0000,  // 1 x 9-bit shunt sample
    SHUNT_ADC_RESOLUTION_10BIT              = 0x0008,  // 1 x 10-bit shunt sample
    SHUNT_ADC_RESOLUTION_11BIT              = 0x0010,  // 1 x 11-bit shunt sample
    SHUNT_ADC_RESOLUTION_12BIT              = 0x0018,  // 1 x 12-bit shunt sample (power-on default)
    SHUNT_ADC_RESOLUTION_12BIT_1S_532US     = 0x0040,  // 1 x 12-bit shunt sample
    SHUNT_ADC_RESOLUTION_12BIT_2S_1060US    = 0x0048,  // 2 x 12-bit shunt samples, averaged
    SHUNT_ADC_RESOLUTION_12BIT_4S_2130US    = 0x0050,  // 4 x 12-bit shunt samples, averaged
    SHUNT_ADC_RESOLUTION_12BIT_8S_4260US    = 0x0058,  // 8 x 12-bit shunt samples, averaged
    SHUNT_ADC_RESOLUTION_12BIT_16S_8510US   = 0x0060,  // 16 x 12-bit shunt samples, averaged
    SHUNT_ADC_RESOLUTION_12BIT_32S_17MS     = 0x0068,  // 32 x 12-bit shunt samples, averaged
    SHUNT_ADC_RESOLUTION_12BIT_64S_34MS     = 0x0070,  // 64 x 12-bit shunt samples, averaged
    SHUNT_ADC_RESOLUTION_12BIT_128S_69MS    = 0x0078,  // 128 x 12-bit shunt samples, averaged

    MODE_MASK                               = 0x0007,
    MODE_POWERDOWN                          = 0x0000,
    MODE_SHUNT_VOLTAGE_TRIGGERED            = 0x0001,
    MODE_BUS_VOLTAGE_TRIGGERED              = 0x0002,
    MODE_SHUNT_AND_BUS_VOLTAGE_TRIGGERED    = 0x0003,
    MODE_ADCOFF                             = 0x0004,
    MODE_SHUNT_VOLTAGE_CONTINUOUS           = 0x0005,
    MODE_BUS_VOLTAGE_CONTINUOUS             = 0x0006,
    MODE_SHUNT_AND_BUS_VOLTAGE_CONTINUOUS   = 0x0007
  } __attribute__((packed));

  struct conversion_time_t {
    short unsigned int resolution;
    uint32_t           us;
  } __attribute__((packed));

  static const conversion_time_t conversion_time[] __PROGMEM;
};

#endif
