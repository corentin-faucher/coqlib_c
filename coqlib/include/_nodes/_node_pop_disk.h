//
//  pop_disk.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-28.
//

#ifndef _coq_node_pop_disk_h
#define _coq_node_pop_disk_h

#include "_node_drawable.h"
#include "_node_fluid.h"
#include "_math/_math_chrono.h"

typedef struct _PopDisk PopDisk;
void PopDisk_spawn(Node* refOpt, PopDisk** refererOpt,
                   uint32_t pngId, uint32_t tile, float deltaT,
                   float x, float y, float twoDy);
void popdisk_cancel(PopDisk** popRef);

void   PopMessage_spawn(Node* refOpt,
                        uint32_t framePngId, UnownedString str,
                        float x, float y, float maxWidth, float height,
                        float timeSec, Rectangle rectDeltas);

#endif /* pop_disk_h */
