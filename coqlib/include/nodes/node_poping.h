//
//  node_poping.h
//  Petites structures de noeud qui `spawn`. (Apparaît et s'autodétruit)
//
//  Created by Corentin Faucher on 2023-10-28.

#ifndef _coq_node_pop_disk_h
#define _coq_node_pop_disk_h

#include "node_fluid.h"
#include "coq_timer.h"
#include "graphs/graph_texture.h"
#include "nodes/node_structs.h"

#pragma mark - Poping Base

typedef struct PopingNode_ {
    union {
        Node   n;      // Peut être casté comme un noeud
        Fluid  f;      // ou smooth.
    };
    Timer*     timer;
} PopingNode;

typedef struct coq_View View;
/// Set la vue "en avant", la texture par défaut et le bruit par défaut pour les PopingNode.
void        PopingNode_init(View* frontView, const char* pngNameOpt, uint32_t soundId);
void        PopingNode_setTexture(uint32_t pngId);
/// Crée un noeud (sans structure) qui s'autodétruit.
/// Si refOpt == NULL -> placé dans la frontView.
PopingNode* PopingNode_spawn(Node* refOpt, float x, float y, float width, float height, size_t structSizeOpt,
                             float timeSec, const PopingInfo* popInfoOpt);
/// Convenience constructor situe le noeud au-dessus de nodeOver (en x,y).
/// Placé dans frontScreen si inFrontScreen, sinon dans les descendant de nodeOver.
/// width et height sont relatifs à la hauteur de nodeOver.
PopingNode* PopingNode_spawnOver(Node* const nodeOver, float width_rel, float height_rel, size_t structSizeOpt,
                                  float timeSec, const PopingInfo* popInfoOpt, bool inFrontScreen);
/// Ajuste la position pour être visible dans l'écran.
void        popingnode_checkForScreenSpilling(PopingNode* pn);

#pragma mark - PopDisk, un disque de `progression`, disparaît une fois plein. (Sous-strut de PopingNode)

typedef struct PopDisk PopDisk;
void PopDisk_spawn(Node* refOpt, PopDisk** refererOpt,
                   uint32_t pngId, uint32_t tile, float deltaT,
                   float x, float y, float twoDy);
void popdisk_cancel(PopDisk** popRef);


#pragma mark - Sparkles ! (des feux d'artifices)
/// Exemple d'implémentation de PopingNode...
void Sparkle_spawnAt(float xabs, float yabs, float delta, Texture* texOpt);
void Sparkle_spawnOver(Node* nd, float deltaRatio);


#pragma mark - PopMessage, message qui s'autodétruit (e.g. error).
/// Autre exemple d'implémentation de PopingNode...
void PopMessage_spawnAt(float xabs, float yabs, float twoDxOpt, float twoDy, float timeSec,
                        uint32_t framePngId, StringDrawable str, FramedStringParams params);
void PopMessage_spawnOver(Node* n, float widthOpt_rel, float height_rel, float timeSec,
                        uint32_t framePngId, StringDrawable str, FramedStringParams params, bool inFrontScreen);



#endif /* pop_disk_h */
