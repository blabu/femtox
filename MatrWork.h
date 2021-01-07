#ifndef MATR_WORK
#define MATR_WORK
#include "FemtoxTypes.h"
#include "FemtoxConf.h"

typedef struct {
    s64 smoothData;
    u08 beta; // Коэффициент фильтрации
}  Filter_t;

s32 arduinoMap(s32 val, s32 fromLow, s32 fromHight, s32 toLow, s32 toHight);
Filter_t getNewFilter(u08 beta);
s64 filterFirstOrder(Filter_t *f, s64 val);

void transpositionMatr(BaseSize_t sizeX, BaseSize_t sizeY, BaseParam_t matr);
void rotate90LeftSqrtMatr(BaseSize_t size, BaseParam_t matr);
void rotate90RightSqrtMatr(BaseSize_t size, BaseParam_t matr);

#endif //MATR_WORK
