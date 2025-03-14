//
//  graph_iusbuffer.h
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-15.
//

#ifndef COQ_GRAPH_IUSBUFFER_H
#define COQ_GRAPH_IUSBUFFER_H

#include "graph_base.h"

// Buffer vers les uniforms d'instance. (`privé`, Implémentation dépend de l'engine graphique.)
typedef struct IUsBuffer IUsBuffer;
/// Création du buffer.
IUsBuffer*        IUsBuffer_create(uint32_t maxCount, InstanceUniforms const* defaultIUOpt);
/// Libère l'espace du buffer (et array de piu si nécessaire)
void              iusbufferref_releaseAndNull(IUsBuffer** iusref);

// Info pour préparer les Instance uniforms dans `renderer_updateInstanceUniforms`.
typedef struct IUsBufferToEdit {
    InstanceUniforms *const beg;
    InstanceUniforms *const end;
    InstanceUniforms       *iu;          // Itérateur du IUsBuffer initialisé sur iusBeg.
    IUsBuffer*const         _iusBuffer;
} IUsToEdit;
// Edition des Instances uniforms.
/// Édition à l'init.
IUsToEdit   iusbuffer_retainIUsToInit(IUsBuffer *iusBuffer);
/// Édition dans le renderer à la mise à jour des Instances Uniforms.
IUsToEdit   iusbuffer_rendering_retainIUsToEdit(IUsBuffer * iusBuffer);
void        iustoedit_release(IUsToEdit edited);

//#define with_IUsToInit(iusEdit)

// Info pour dessiner avec renderer.
typedef struct IUsBufferToDraw {
    size_t const            count;
    size_t const            size;
    InstanceUniforms const*const iusOpt;
    // Utiliser avec `(__bridge id<MTLBuffer>)` pour avoir un pointeur objective-C.
    void const*const        metal_bufferOpt;    
} IUsToDraw;
// Envoie au GPU.
IUsToDraw   iusbuffer_rendering_getToDraw(IUsBuffer const* iusBuffer);


#endif
