#ifndef PROGRAM_SERIAL_X86
#define PROGRAM_SERIAL_X86

#include "FemtoxTypes.h"

extern const void*const ReceiveNewPackageLabel;

void initSerial(BaseSize_t baud);
void clearBuf();
void readBuf(BaseSize_t sz, BaseParam_t buff);
void sendBuf(BaseSize_t sz, byte_ptr buff);
void sendByte(u08 c);
void setReceiveTimeoutSerial(u16 tick);

#endif // PROGRAM_SERIAL_X86