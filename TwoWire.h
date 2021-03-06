/*
 * TwoWire.h
 *
 * Created: 7/16/2015 7:19:10 PM
 *  Author: ekt
 */ 


#ifndef TWOWIRE_H_
#define TWOWIRE_H_

#include <inttypes.h>

void TwoWireUpdate();
void TwoWireInit(uint8_t broadcastAddrss, bool useInterrupts=true);
void TwoWireSetDeviceAddress(uint8_t address);
uint8_t TwoWireGetDeviceAddress();

int TwoWireCallback(uint8_t address, uint8_t *buffer, uint8_t len, uint8_t maxLen);

#endif /* TWOWIRE_H_ */