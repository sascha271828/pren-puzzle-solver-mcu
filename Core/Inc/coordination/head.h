#ifndef __HEAD_H__
#define __HEAD_H__


#include <stdint.h>
#include <stdbool.h>

void HeadPickUpPiece(void);
void HeadPlacePiece(void);
void HeadRotatePiece(int32_t degrees);

bool Head_IsBusy(void);


#endif /* __HEAD_H__ */