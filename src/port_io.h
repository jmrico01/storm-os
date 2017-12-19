#ifndef PORT_IO_H
#define PORT_IO_H

#include "types.h"

uint8 PortByteIn(uint16 port);

void PortByteOut(uint16 port, uint8 byte);

uint16 PortWordIn(uint16 port);

void PortWordOut(uint16 port, uint16 word);

#endif