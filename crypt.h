#ifndef _AES_H_
#define _AES_H_

#include "TaskMngr.h"

void setSeed(u32 seed);
u32 RandomSimple();
u32 RandomMultiply();
u16 CRC16(BaseSize_t size, byte_ptr msg);

// #define the macros below to 1/0 to enable/disable the mode of operation.
// CBC enables AES encryption in CBC-mode of operation.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif

//#define AES128 1
#define AES192 1
//#define AES256 1

#if defined(ECB) && (ECB == 1)

void AesEcbEncrypt(const byte_ptr input, const byte_ptr key, byte_ptr output);
void AesEcbDecrypt(const byte_ptr input, const byte_ptr key, byte_ptr output);

#endif // #if defined(ECB) && (ECB == !)

#if defined(CBC) && (CBC == 1)

void AesCbcEncrypt_buffer(byte_ptr output, byte_ptr input, u32 length, const byte_ptr key, const byte_ptr iv);
void AesCbcDecrypt_buffer(byte_ptr output, byte_ptr input, u32 length, const byte_ptr key, const byte_ptr iv);

#endif // #if defined(CBC) && (CBC == 1)


#endif //_AES_H_
