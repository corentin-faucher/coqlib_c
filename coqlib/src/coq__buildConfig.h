//
//  coq__buildConfig.h
//  Paramètres généraux divers de la lib.
//
//  Created by Corentin Faucher on 2025-01-15.
//

#ifndef COQ_BUILD_CONFIG_H
#define COQ_BUILD_CONFIG_H

// Pour les grosses meshes : taille a partir de laquelle on crée un buffer.
// Pour métal, quand on a moins de 4ko, un buffer est superflu...
#define Mesh_verticesSizeForBuffer 4096 // i.e. avec sizeof(Vertex) == 32 => max 128 vertex.
#define IUsBuffer_sizeForBuffer    4096 // i.e. avec sizeof(InstUnif) == 112 => max 36 iu.

// Avertissement quand on crée un buffer d'instance uniforms avec moins de 2 instances.
#define IUsBuffer_warningMaxCount true

// Mettre "true" si on veut finetuner l'alignement/cadrage des polices de caractères.
#define COQ_TEST_FONT false

#endif
