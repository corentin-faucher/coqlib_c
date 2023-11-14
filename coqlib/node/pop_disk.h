//
//  pop_disk.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-28.
//

#ifndef pop_disk_h
#define pop_disk_h

#include <stdio.h>
#include "drawable.h"
#include "fluid.h"
#include "timer.h"
#include "chronometers.h"


typedef struct _PopDisk PopDisk;
void PopDisk_spawn(Node* refOpt, PopDisk** refererOpt,
                   uint32_t pngId, uint32_t tile, float deltaT,
                   float x, float y, float twoDy);
void popdisk_cancel(PopDisk** popRef);

void   PopMessage_spawn(Node* refOpt,
                        uint32_t framePngId, UnownedString str,
                        float x, float y, float maxWidth, float height,
                        float timeSec);

#endif /* pop_disk_h */
