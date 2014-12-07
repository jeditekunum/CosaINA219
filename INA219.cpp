/**
 * @file INA219.cpp
 * @version 0.9
 *
 * @section License
 * Copyright (C) 2014, jediunix
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

#include "INA219.hh"

/**
 * This driver does not bother with the current or power registers of
 * the device (and the subsequent need for calibration and calculation).
 *
 * Current and power are simply calculated here from the shunt voltage
 * and resistance.
 */

const INA219::conversion_time_t INA219::conversion_time[] __PROGMEM = {
  //  {INA219::BUS_ADC_RESOLUTION_9BIT,                   84UL},
  //  {INA219::BUS_ADC_RESOLUTION_10BIT,                 148UL},
  //  {INA219::BUS_ADC_RESOLUTION_11BIT,                 276UL},
  {INA219::BUS_ADC_RESOLUTION_12BIT,                 532UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_1S_532US,        532UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_2S_1060US,      1060UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_4S_2130US,      2130UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_8S_4260US,      4260UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_16S_8510US,     8510UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_32S_17MS,      17020UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_64S_34MS,      34050UL},
  //  {INA219::BUS_ADC_RESOLUTION_12BIT_128S_69MS,     68100UL},

  //  {INA219::SHUNT_ADC_RESOLUTION_9BIT,                 84UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_10BIT,               148UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_11BIT,               276UL},
  {INA219::SHUNT_ADC_RESOLUTION_12BIT,               532UL}
  //  {INA219::SHUNT_ADC_RESOLUTION_12BIT_1S_532US,      532UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_12BIT_2S_1060US,    1060UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_12BIT_4S_2130US,    2130UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_12BIT_8S_4260US,    4260UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_12BIT_16S_8510US,   8510UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_12BIT_32S_17MS,    17020UL},
  //  {INA219::SHUNT_ADC_RESOLUTION_12BIT_128S_69MS,   68100UL}
};

uint32_t
INA219::conversion(uint16_t adc_resolution)
{
  for (uint8_t i = 0; i<membersof(conversion_time); i++)
    if (adc_resolution == (uint16_t)pgm_read_word(&conversion_time[i]))
      return ((uint32_t)pgm_read_dword(&conversion_time[i].us));
  return (0);
}

void
INA219::begin(range_t range)
{
  m_range = range;

  write_register(CONFIG, RESET);

  DELAY(50);  // recover from a possible powerdown (requires 40us)

  switch (m_range)
    {
    case RANGE_32V:
      configure(BUS_VOLTAGE_RANGE_32V |
                GAIN_8_320MV |
                BUS_ADC_RESOLUTION_12BIT |
                SHUNT_ADC_RESOLUTION_12BIT |
                MODE_SHUNT_AND_BUS_VOLTAGE_CONTINUOUS);
      break;

    case RANGE_16V:
      configure(BUS_VOLTAGE_RANGE_16V |
                GAIN_1_40MV |
                BUS_ADC_RESOLUTION_12BIT |
                SHUNT_ADC_RESOLUTION_12BIT |
                MODE_SHUNT_AND_BUS_VOLTAGE_CONTINUOUS);
      break;
    };

  m_active = true;
}

void
INA219::wait(register_t reg)
{
  switch (reg)
    {
    case BUS_VOLTAGE:
      if (m_bus_conversion_time > 1000) // us
        delay((m_bus_conversion_time / 1000) + 1); // ms
      else
        DELAY(m_bus_conversion_time + 250); // us
      break;

    case SHUNT_VOLTAGE:
      if (m_shunt_conversion_time > 1000) // us
        delay((m_shunt_conversion_time / 1000) + 1); // ms
      else
        DELAY(m_shunt_conversion_time + 250); // us
      break;

    default:
      break;
    };
}

void
INA219::configure(uint16_t config)
{
  m_config_value = config;

  m_bus_conversion_time = conversion(config & BUS_ADC_RESOLUTION_MASK);
  m_shunt_conversion_time = conversion(config & SHUNT_ADC_RESOLUTION_MASK);

  write_register(CONFIG, config);

  // If mode is continuous then wait for first converion to complete
  if (((m_config_value & MODE_MASK) == MODE_SHUNT_VOLTAGE_CONTINUOUS) ||
      ((m_config_value & MODE_MASK) == MODE_BUS_VOLTAGE_CONTINUOUS) ||
      ((m_config_value & MODE_MASK) == MODE_SHUNT_AND_BUS_VOLTAGE_CONTINUOUS))
    wait((m_bus_conversion_time > m_shunt_conversion_time) ? BUS_VOLTAGE : SHUNT_VOLTAGE);
}

void
INA219::end()
{
  m_active = false;

  write_register(CONFIG, MODE_POWERDOWN);
}

void
INA219::write_register(register_t reg, uint16_t value)
{
  value = swap(value);

  twi.begin(this);
  twi.write((uint8_t)reg, &value, sizeof(value));
  twi.end();
}

uint16_t
INA219::read_register(register_t reg)
{
  uint16_t res;

  if (!m_active)
    return (0);

  twi.begin(this);
  twi.write((uint8_t)reg);

  // If mode is triggered then wait for conversion to complete
  if ((((reg == SHUNT_VOLTAGE)) &&
       (((m_config_value & MODE_MASK) == MODE_SHUNT_VOLTAGE_TRIGGERED) ||
        ((m_config_value & MODE_MASK) == MODE_SHUNT_AND_BUS_VOLTAGE_TRIGGERED))) ||
      ((reg == BUS_VOLTAGE) &&
       (((m_config_value & MODE_MASK) == MODE_BUS_VOLTAGE_TRIGGERED) ||
        ((m_config_value & MODE_MASK) == MODE_SHUNT_AND_BUS_VOLTAGE_TRIGGERED))))
    wait(reg);

  twi.read(&res, sizeof(res));
  twi.end();

  res = swap(res);

  return (res);
}

uint16_t
INA219::current(uint16_t uv)
{
  // return 10ths of ma
  return ((uv * 10UL) / (uint32_t)m_resistor);
}

uint16_t
INA219::power(uint16_t uv)
{
  uint32_t calc;

  calc = uv / 10;
  calc *= calc; // **2

  // return tenths of mW
  return (calc / (uint32_t)m_resistor);
}
