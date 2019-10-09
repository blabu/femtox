/*
MIT License

Copyright (c) 2017 Oleksiy Khanin

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
 * */

#include "TaskMngr.h"
#include "MatrWork.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef NEED_MATRIX
#ifdef ALLOC_MEM
#define ALLOCATE_ENABLE
#endif
#ifdef ALLOC_MEM_LARGE
#define ALLOCATE_ENABLE
#endif
#ifdef ALLOCATE_ENABLE

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
#endif
#ifdef __cplusplus
}
#endif
#endif
