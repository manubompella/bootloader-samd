/*
 * bootloader_attiny.cpp
 *
 * Created: 10/6/2015 12:27:29 PM
 *  Author: ekt
 */



#include <string.h>

#include "TwoWire.h"
#include "SelfProgram.h"
#include "asf.h"

#define BOOTLOADER_VERSION 1

void writePage(uint16_t address, uint8_t *data);

// Commands
//

// GetNextDeviceID(previousID) -> uint16 nextID
// GetDeviceType() -> string deviceType
// ExitBootloader()

// SetDeviceType(string type)
// SetDeviceID(uint16_t newID)

// GetBootloaderVersion() -> uint16 version
// GetMCUSignature() -> uint32 signature
// ReadPage(uint16_t address) -> uint8[16]
// ErasePage(uint16_t address)
// WritePage(uint16_t address, uint8_t data[16])
// ReadEEPROM(uint8_t address, uint8_t len) -> uint8_t[]
// WriteEEPROM(uint8_t address, uint8_t data[], uint8_t len)



#define FUNCTION_GLOBAL_RESET 0

#define FUNCTION_EXIT_BOOTLOADER 100
#define FUNCTION_GET_BOOTLOADER_VERSION 101
#define FUNCTION_GET_NEXT_DEVICE_ID 102
#define FUNCTION_SET_DEVICE_ID 103
#define FUNCTION_GET_MCU_SIGNATURE 104
#define FUNCTION_READ_PAGE 105
#define FUNCTION_ERASE_PAGE 106
#define FUNCTION_WRITE_PAGE 107
#define FUNCTION_READ_EEPROM 108
#define FUNCTION_WRITE_EEPROM 109
#define FUNCTION_SET_BOOTLOADER_SAFE_MODE 110


SelfProgram selfProgram;
volatile bool bootloaderRunning = true;

uint16_t getUInt16(uint8_t *data) {
	return data[0] | (data[1] << 8);
}

uint32_t getUInt32(uint8_t *data) {
	return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

int checkDeviceID(uint8_t *data) {
	return (getUInt16(data) == selfProgram.getDeviceID());
}

int TwoWireCallback(uint8_t address, uint8_t *data, uint8_t len, uint8_t maxLen) {
	
	if (len < 3) {
		return 0;
	}
	
	switch (data[0]) {
		case FUNCTION_GLOBAL_RESET:
		case FUNCTION_EXIT_BOOTLOADER:
			bootloaderRunning = false;
			break;
		case FUNCTION_GET_BOOTLOADER_VERSION:
			if (len == 5 && checkDeviceID(data+2)) {
				// Return the device ID
				data[0] = BOOTLOADER_VERSION & 0xFF;
				data[1] = BOOTLOADER_VERSION >> 8;
				return 2;
			}
			break;
		case FUNCTION_GET_NEXT_DEVICE_ID:
			if (len == 5) {
				uint16_t previousID = getUInt16(data+2);
				uint16_t deviceID = selfProgram.getDeviceID();
				if (previousID < deviceID) {
					// Return the device ID
					data[0] = deviceID & 0xFF;
					data[1] = deviceID >> 8;
					return 2;
				}
			}
			break;
		case FUNCTION_SET_DEVICE_ID:
			if (len == 7 && checkDeviceID(data+2)) {
				selfProgram.storeDeviceID(getUInt16(data+4));
			}
			break;

		case FUNCTION_GET_MCU_SIGNATURE:
			if (len == 5 && checkDeviceID(data+2)) {
				uint32_t sig = selfProgram.getSignature();
				for (int i=0; i < 4; i++) {
					data[i] = static_cast<uint8_t>(sig >> 8*i);
				}
				return 4;
			}
			break;
		case FUNCTION_READ_PAGE:
			if (len == 10 && checkDeviceID(data+2)) {
				uint32_t address = getUInt32(data+4);
				uint8_t len = data[8];
				return selfProgram.readPage(address, data, len);
			}
			break;
		case FUNCTION_ERASE_PAGE:
			if (len == 9 && checkDeviceID(data+2)) {	
				uint32_t address = getUInt32(data+4);
				selfProgram.erasePage(address);
			}
			break;
		case FUNCTION_WRITE_PAGE:
			if (len >= 9 && checkDeviceID(data+2)) {
				uint32_t address = getUInt32(data+4);
				selfProgram.writePage(address, data+8, len-9);
			}
			break;
		case FUNCTION_READ_EEPROM:
			if (len == 10 && checkDeviceID(data+2)) {
				uint32_t address = getUInt32(data+4);
				uint8_t len = data[8];
				selfProgram.readEEPROM(address, data, len);
				return len;
			}
			break;
		case FUNCTION_WRITE_EEPROM:
			if (len >= 9  && checkDeviceID(data+2)) {
				uint16_t address = getUInt32(data+4);
				
				selfProgram.writeEEPROM(address, data+8, len-9);
			}
			break;
		case FUNCTION_SET_BOOTLOADER_SAFE_MODE:
			if (len == 4 && checkDeviceID(data+1)) {
				selfProgram.setSafeMode(data[3]);
			}
			break;
	}
	
	return 0;
}


int main(void)
{
	selfProgram.setLED(true);
	
	/* Check switch state to enter boot mode or application mode */
	selfProgram.checkBootMode();

	/* Initialize system */
	system_init();
	//delay_init();
	cpu_irq_enable();
		
	/* Get NVM default configuration and load the same */
	nvm_config config;
	nvm_get_config_defaults(&config);
	config.manual_page_write = false;
	nvm_set_config(&config);
	

	selfProgram.loadDeviceID();
	TwoWireInit(9);
	
		
	while (bootloaderRunning) {
	}

	selfProgram.startApplication();
}

