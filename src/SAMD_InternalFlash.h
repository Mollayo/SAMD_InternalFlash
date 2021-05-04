/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  Written by Cristian Maglie

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SAMD_INTERNALFLASH_H_
#define SAMD_INTERNALFLASH_H_

#pragma once

#include <Arduino.h>
#include <FlashStorage.h>

#include "Adafruit_FlashTransport_InternalFlash.h"
#include "Adafruit_InternalFlash_Wrapper.h"


// Concatenate after macro expansion
#define PPCAT_NX(A, B) A ## B
#define PPCAT(A, B) PPCAT_NX(A, B)

#if defined(__SAMD51__)
  #define InternalFlash(name, size) \
  __attribute__((__aligned__(8192))) \
  static const uint8_t PPCAT(_data,name)[(size+8191)/8192*8192] = { }; \
  InternalFlashClass name(PPCAT(_data,name), size);
#else
  #define InternalFlash(name, size) \
  __attribute__((__aligned__(256))) \
  static const uint8_t PPCAT(_data,name)[(size+255)/256*256] = { }; \
  InternalFlashClass name(PPCAT(_data,name), size);
#endif

class InternalFlashClass {
public:
  InternalFlashClass(const void *flash_addr = NULL, uint32_t size = 0);

  void write(uint32_t offset, const void *data, uint32_t size);
  void erase(uint32_t offset, uint32_t size);
  void read(uint32_t offset, void *data, uint32_t size);

  uint32_t get_flash_size() const { return flash_size;}
  void *get_flash_address() const { return (void*)flash_address;}

  void flush_buffer();

private:
  const volatile void *flash_address;
  const uint32_t flash_size;

  FlashClass fl;

#if defined(__SAMD51__)
  uint8_t buff[8192];
  uint32_t buff_addr;
  bool buff_in_used;
#endif
};

#endif	// SAMD_INTERNALFLASH_H_

