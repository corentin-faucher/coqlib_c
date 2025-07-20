//  graph_iusbuffer.h
//
//  Instance Uniforms buffer.
//  Contient un array d'"instance uniforms"
//  pour dessiner une série d'objets semblables, 
//  e.g. les glyphes d'une string de char. 
//
//  Created by Corentin Faucher on 2025-01-15.
//
#ifndef COQ_GRAPH_IUSBUFFER_H
#define COQ_GRAPH_IUSBUFFER_H
#include "graph_base.h"

// Buffer d'instance uniforms.
typedef struct IUsBuffer {
    // Nombre d'instances actives, 0 par défaut. (Voir `iustoedit_release` pour changement d'actual_count.)
    size_t const                 actual_count;
    size_t const                 actual_size;
    size_t const                 max_count;
    // En tant que simple array d'instance uniforms (peut être un lien vers MTLBuffer...)
    InstanceUniforms const*const iusOpt;
    // Lien vers MTLBuffer. Bridger de C à Objective-C avec `(__bridge id<MTLBuffer>)`.
    const void* const            mtlBufferOpt; 
    
    // ("privé")
    bool               _editing;
    const void* const  _mtlBufferOptA;
    const void* const  _mtlBufferOptB;
} IUsBuffer;
/// Init du buffer.
void   iusbuffer_init(IUsBuffer* iusbuffer, uint32_t maxCount, InstanceUniforms const* defaultIUOpt);
/// Libère l'espace du buffer (et array de piu si nécessaire)
void   iusbuffer_deinit(IUsBuffer* iusbuffer);

// Info pour préparer les Instance uniforms dans `renderer_updateInstanceUniforms`.
typedef struct IUsBufferToEdit {
    InstanceUniforms *const beg;
    InstanceUniforms *const end;
    InstanceUniforms       *iu;          // Itérateur du IUsBuffer initialisé sur iusBeg.
    bool                    init;        // Si init on fixe les deux buffer. (cas double buffer)
    IUsBuffer*const         _iusBuffer;
} IUsToEdit;
// Edition des Instances uniforms.
IUsToEdit   iusbuffer_retainIUsToEdit(IUsBuffer* iusBuffer);
// Met à jour les IU prêt à la lecture. Met à jour aussi actual_count en prenant la différence `iu - beg`.
void        iustoedit_release(IUsToEdit edited);
#define withIUsToEdit_beg(iusEdit, iusBuffer) \
    { IUsToEdit iusEdit = iusbuffer_retainIUsToEdit(iusBuffer); if(iusEdit.beg) {
#define withIUsToEdit_end(iusEdit) iustoedit_release(iusEdit); } \
    else { printerror("Cannot edit Instance uniforms buffer."); } }


#endif
