//
//  coq__buildConfig.h
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-15.
//

#ifndef COQ_BUILD_CONFIG_H
#define COQ_BUILD_CONFIG_H

// Pour les grosses meshes, on crée un buffer (superflu pour les petites meshes)
#define Mesh_verticesSizeForBuffer 4096 // i.e. avec sizeof(Vertex) == 32 => max 128 vertex.
#define IUsBuffer_sizeForBuffer    4096 // i.e. avec sizeof(InstUnif) == 112 => max 36 iu.

// Avertissement quand on crée un buffer d'instance uniforms avec moins de 2 instances.
#define IUsBuffer_warningMaxCount true

// Pour finetuner l'alignement/cadrage des polices de caractères.
#define COQ_TEST_FONT false

#endif
