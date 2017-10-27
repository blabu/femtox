#include "TaskMngr.h"
#include "MatrWork.h"

#ifdef __cplusplus
extern "C" {
#endif


void rotate90RightSqrtMatr(BaseSize_t size, BaseParam_t matr)
{
  if(size>11) return;
  byte_ptr copy = allocMem(size*size);
  byte_ptr original = (byte_ptr)matr;
  for(BaseSize_t i=0; i<size;i++)
    for(BaseSize_t j=0; j<size;j++)
    {
      copy[j*size + i] = original[j*size + i];
    }
  for(BaseSize_t i=0,k=size-1; i<size; i++,k--)
  {
    for(BaseSize_t j=0; j<size;j++)
    {
    	original[k*size+j] = copy[j*size + i];
    }
  }
  freeMem(copy);
}

void rotate90LeftSqrtMatr(BaseSize_t size, BaseParam_t matr)
{
  if(size>11) return;
  byte_ptr copy = allocMem(size*size);
  byte_ptr original = (byte_ptr)matr;
  for(BaseSize_t i=0; i<size;i++)
    for(BaseSize_t j=0; j<size;j++)
    {
      copy[j*size + i] = original[j*size + i];
    }
  for(BaseSize_t i=0,k=size-1; i<size; i++,k--)
  {
    for(BaseSize_t j=0; j<size;j++)
    {
    	original[k*size+j]=copy[j*size + i];
    }
  }
  freeMem(copy);
}

void swapByte(byte_ptr byte1, byte_ptr byte2) {
  unsigned char temp = *byte1;
  *byte1 = *byte2;
  *byte2 = temp;
}


void swapInt(unsigned int* int1, unsigned int* int2)
{
  unsigned int temp = *int1;
  *int1 = *int2;
  *int2 = temp;
}
#ifdef __cplusplus
}
#endif
