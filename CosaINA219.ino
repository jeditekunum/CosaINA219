/**
 * @file CosaINA219.ino
 * @version 0.8
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

#include "Cosa/IOStream.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/OutputPin.hh"

#include "INA219.hh"

OutputPin led(Board::LED);

IOStream ios(&uart);

INA219 sensor;


void setup()
{
  uart.begin(9600);

  led.on();
  sleep(2);
  led.off();

  ios << PSTR("CosaINA219: started") << endl;

  sensor.begin();

  ios << PSTR("bus conversion every ") << sensor.bus_time() << PSTR(" us") << endl;
  ios << PSTR("shunt conversion every ") << sensor.shunt_time() << PSTR(" us") << endl;
}

void loop()
{
  uint16_t uv;
  uint16_t whole, fraction;

  ios << PSTR("bus ") << sensor.bus() << PSTR(" mV, ");

  uv = sensor.shunt();
  ios << PSTR("shunt ") << uv << PSTR(" uV, ");

  fraction = sensor.current(uv);
  whole = fraction / 10;
  fraction %= 10;
  ios << PSTR("current ") << whole << PSTR(".") << fraction << PSTR(" mA, ");

  fraction = sensor.power(uv);
  whole = fraction / 10;
  fraction %= 10;
  ios << PSTR("power ") << whole << PSTR(".") << fraction << PSTR(" mW");

  ios << endl;

  sleep(5);
}
