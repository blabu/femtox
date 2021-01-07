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

void rotate90RightSqrtMatr(BaseSize_t size, BaseParam_t matr){
  #ifdef ALLOC_MEM
    if(size>11) return;
  #endif
  BaseSize_t matrSize = size*size;
  byte_ptr copy = allocMem(matrSize);
  byte_ptr original = (byte_ptr)matr;
  memCpy(copy, original, matrSize);
  for(BaseSize_t i=0; i<size; i++) {
    BaseSize_t row = i*size;
    for(BaseSize_t j=0, k=size-1; j<size; j++,k--) {
    	original[row+j] = copy[k*size + i];
    }
  }
  freeMem(copy);
}

void rotate90LeftSqrtMatr(BaseSize_t size, BaseParam_t matr){
  #ifdef ALLOC_MEM
    if(size>11) return;
  #endif
  BaseSize_t matrSize = size*size;
  byte_ptr copy = allocMem(matrSize);
  byte_ptr original = (byte_ptr)matr;
  memCpy(copy, original, matrSize);
  for(BaseSize_t i=0,k=size-1; i<size; i++,k--) {
    BaseSize_t row = k*size;
    for(BaseSize_t j=0; j<size;j++) {
    	original[row+j]=copy[j*size + i];
    }
  }
  freeMem(copy);
}

void transpositionMatr(BaseSize_t sizeX, BaseSize_t sizeY, BaseParam_t matr) {
  BaseSize_t size = sizeX * sizeY;
  #ifdef ALLOC_MEM
    if(size > 127) return;
  #endif
  byte_ptr resMatr = allocMem(size);
  if(resMatr == NULL) return;
  byte_ptr origin = (byte_ptr)matr;
  for(BaseSize_t i = 0; i<sizeX; i++) {
    BaseSize_t row = i*sizeY;
    for(BaseSize_t j = 0; j<sizeY; j++) {
      BaseSize_t col = j*sizeX;
      resMatr[col + i] = origin[row + j];
    }
  }
  memCpy(matr, resMatr, size);
  freeMem(resMatr);
}

#endif //ALLOCATE_ENABLE
#endif //NEED_MATRIX

s32 arduinoMap(s32 val, s32 fromLow, s32 fromHight, s32 toLow, s32 toHight) {
    if(val < fromLow) return toLow;
    if(val > fromHight) return toHight;
    s32 deltaFrom = fromHight-fromLow;
    s32 deltaTo = toHight - toLow;
    if (deltaTo > deltaFrom) {
        s32 k = (s32)(deltaTo/deltaFrom);
        return (val-fromLow) * k + toLow;
    } else if(deltaTo < deltaFrom){
        s32 k = (s32)(deltaFrom/deltaTo);
        return (val-fromLow) / k + toLow;
    }
    return val;
}

// beta - must be from 0 to 10
Filter_t getNewFilter(u08 beta) {
    Filter_t f = {.smoothData = 0, .beta = beta};
    return f;
}

// source https://kiritchatterjee.wordpress.com/2014/11/10/a-simple-digital-low-pass-filter-in-c/
// формула y[i] = b*x[i] + (1-b)*y[i-1]
// где b < 1
// y[i] = y[i-1] + b*x[i] - b*y[i-1]
// y[i] = y[i-1] + b*(x[i]-y[i-1])
// y[i] = y[i-1] - b*(y[i-1]-x[i])
// Если b < 1 , этот коэфициент можно представить как 1/a
// Где a > 1 (b=0.1 тогда a = 10), это нужно для целочисленных вычислений
// Тогда формула преобретет вид
// y[i] = y[i-1] - (y[i-1]-x[i])/a
// res = a*y[i] = a*y[i-1] - y[i-1] - x[i]
// и результат работы фильтра y[i] = res/a 
s64 filterFirstOrder(Filter_t *f, s64 val) {
    f->smoothData = (f->smoothData << f->beta) - f->smoothData;
    f->smoothData += val;
    return f->smoothData >>= f->beta;
}

#ifdef __cplusplus
}
#endif
