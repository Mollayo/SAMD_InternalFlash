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

#include "SAMD_InternalFlash.h"

void InternalFlashClass::write(uint32_t offset, const void *data, uint32_t size)
{
#if defined(__SAMD51__)
  // flash_address+offset should be a multiple of 8192 for __SAMD51__
  uint32_t new_buff_addr=offset-(offset%8192);
  // If the buffer is in used and the addr is different, we flush the buffer to use it with the new addr
  if (buff_in_used==true && new_buff_addr!=buff_addr)
    flush_buffer();
  
  buff_addr=new_buff_addr;
  // Initialise the buffer
  if (buff_in_used==false)
  {
    memcpy((void *)(buff), (const void *)(flash_address+buff_addr), 8192); 
    buff_in_used=true;
  }
  // Write the data to the buffer
  uint32_t buff_offset=offset%8192;
  memcpy((void *)(buff+buff_offset), (const void *)data, size);
  //Serial.printf("InternalFlashClass::write to buffer at offset %d (base addr: %d) with size %d",
  //								offset, buff_addr, size);
  //Serial.println();
#else
  fl.write(flash_address+offset, data, size);
  //Serial.printf("InternalFlashClass::write to buffer at offset %d with size %d", offset, size);
  //Serial.println();
#endif
}

void InternalFlashClass::erase(uint32_t offset, uint32_t size)
{
#if defined(__SAMD51__)
  // Do nothing for the __SAMD51__. Erase is done when the buffer is flushed
#else
  //volatile void *flash_address_offset = (volatile uint32_t *)(flash_address+offset);
  fl.erase(flash_address+offset, size);
#endif
}


void InternalFlashClass::read(uint32_t offset, void *data, uint32_t size)
{
  //Serial.printf("InternalFlashClass::read at offset %d and with size %d",offset,size);
  //Serial.println();
#if defined(__SAMD51__)
  // Check if the data which is read is located in the buffer
  uint32_t new_buff_addr=offset-(offset%8192);
  if (buff_in_used==true && new_buff_addr==buff_addr)
  {
    // Assume that buff_offset + size < sizeof(buff)
    uint32_t buff_offset=offset%8192;
    memcpy(data,(void *)(buff+buff_offset), size);
  }
  else
  {
    fl.read(flash_address+offset, data, size);
  }
#else
  fl.read(flash_address+offset, data, size);
#endif
}

InternalFlashClass::InternalFlashClass(const void *flash_addr, uint32_t size) :
  flash_address((volatile void *)flash_addr),
  flash_size(size)
{
}


void InternalFlashClass::flush_buffer()
{
// This is specific to __SAMD51__ since the writing has to be 8192 bytes long

#if defined(__SAMD51__)
  // If nothing in the buffer, we quite
  if (buff_in_used==false)
    return;

  //Serial.printf("InternalFlashClass::flush_buffer");
  //Serial.println();

  // Erase first
  fl.erase(flash_address+buff_addr, 8192);

  // Write the buffer
  //Serial.printf("InternalFlashClass::write at offset %d and with size %d", buff_addr, 8192);
  //Serial.println();
  fl.write(flash_address+buff_addr, (const void *)buff, 8192);
  // Indicate the buffer is empty
  buff_in_used=false;
#endif
}