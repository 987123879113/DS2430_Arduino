/*
MIT License

Copyright (c) 2017 Tom Magnier
Modified 2018 by Nicolò Veronese

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include "DS2430.h"

DS2430::DS2430(OneWire &ow)
: _ow(ow)
{
  _skiprom = true;
}

void DS2430::begin(uint8_t serialNumber[ONE_WIRE_MAC_SIZE])
{
  memcpy(_serialNumber, serialNumber, ONE_WIRE_MAC_SIZE);
  _skiprom = false;
}

uint8_t DS2430::read(uint8_t address)
{
  uint8_t res = 0xFF;
  read(address, &res, 1);

  return res;
}

void DS2430::read(uint8_t address, uint8_t *buf, uint16_t len)
{
  _startTransmission();

  _ow.write(READ_MEMORY, 1);
  _ow.write(address, 1);

  for (int i = 0; i < len; i++)
  buf[i] = _ow.read();

  _ow.depower();
}

bool DS2430::write(uint8_t address, const uint8_t *buf, uint16_t count, bool verify /* = 0 */)
{
  bool ret = _write(address, buf, count, verify);
  _ow.depower();
  return ret;
}

bool DS2430::_write(uint8_t address, const uint8_t *buf, uint16_t count, bool verify)
{
  uint8_t error_count = 0;
  uint8_t buffer[DS2430_BUFFER_SIZE];

  //Address has to be aligned on an 8-byte boundary
  if (address >= DS2430_EEPROM_SIZE || address % 8 != 0)
    return false;

  // Prepare buffer data
  buffer[0] = WRITE_SCRATCHPAD;
  buffer[1] = address;
  memcpy(&buffer[DS2430_CMD_SIZE], buf, count);

  //Write scratchpad
  _startTransmission();
  _ow.write_bytes(buffer, DS2430_CMD_SIZE + count, 1); // Write CMD + Adr

  // Prepare buffer data
  buffer[0] = COPY_SCRATCHPAD;
  buffer[1] = 0xa5;

  //Copy scratchpad
  _startTransmission();
  _ow.write_bytes(buffer, DS2430_CMD_SIZE, 1); //Send authorization code (TA1, TA2, E/S)
  delay(15); // t_PROG = 12.5ms worst case.

  return true;
}
