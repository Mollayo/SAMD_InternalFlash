// Include for the USB mass storage
#include "Adafruit_TinyUSB.h"

// Include for the FAT file system
#include "SdFat.h"

// Include for the internal flash
#include "SAMD_InternalFlash.h"

// The tinyUSB flash drive
Adafruit_USBD_MSC usb_msc;

// Allocate the internal flash
#define DISK_BLOCK_NUM  400     // Number of blocks 400
#define DISK_BLOCK_SIZE 512     // Block size in bytes
InternalFlash(my_internal_storage, DISK_BLOCK_SIZE*DISK_BLOCK_NUM);

// The wrapper for the Adafruit SPI Flash
Adafruit_FlashTransport_InternalFlash flashTransport(&my_internal_storage);
Adafruit_InternalFlash_Wrapper flash(&flashTransport);

// The file system object from SdFat
FatFileSystem fatfs;


// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and 
// return number of copied bytes (must be multiple of block size) 
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  //Serial.printf("Reading at %d with size %d\n",lba,bufsize);
  my_internal_storage.read(lba*DISK_BLOCK_SIZE, buffer, bufsize);
  return bufsize;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  //Serial.printf("Writing at %d with size %d\n",lba,bufsize);
  // Erasing is absolutly necessary otherwise writing does not work
  my_internal_storage.erase(lba*DISK_BLOCK_SIZE,bufsize);
  my_internal_storage.write(lba*DISK_BLOCK_SIZE, buffer, bufsize);
  return bufsize;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  //Serial.printf("Flushing begin\n");
  // nothing to do
}


void setup() {

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "Mass Storage", "1.0");
  
  // Set disk size
  usb_msc.setCapacity(DISK_BLOCK_NUM, DISK_BLOCK_SIZE);

  // Set callbacks
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set Lun ready (internal flash is always ready)
  usb_msc.setUnitReady(true);
  usb_msc.begin();

  Serial.begin(115200);
  while ( !Serial ) delay(10);   // wait for native usb

  Serial.printf("Internal flash with address %d and size %d\n", my_internal_storage.get_flash_address(),my_internal_storage.get_flash_size());
  
  // Setup for the internal flash
  if (flash.begin())
    Serial.println("Internal flash successfully set up.");
  else
    Serial.println("Error: failed to set up the internal flash.");

  // The file system object from SdFat to read/write to the files in the internal flash
  if ( !fatfs.begin(&flash) )
  {
    Serial.println("Error: file system not existing. The internal flash drive should first be formated with Windows or fdisk on Linux.");
  }
  else
  {
    Serial.println("File system successfully open.");

    // List all the files in the internal flash drive
    File root;
    File file;
    if (!root.open("/")) {
      Serial.println("open root failed");
    }
    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.
    while (file.openNext(&root, O_RDONLY)) {
      file.printFileSize(&Serial);
      Serial.write(' ');
      file.printModifyDateTime(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      if (file.isDir()) {
        // Indicate a directory.
        Serial.write('/');
      }
      Serial.println();
      file.close();
    }

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File myFile = fatfs.open("test.txt", FILE_WRITE);
  
    // if the file opened okay, write to it:
    if (myFile) {
      Serial.println("Writing to test.txt...");
      myFile.println("testing 1, 2, 3.");
      // close the file:
      myFile.close();
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
