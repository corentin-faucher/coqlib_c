//
//  node_poping.h
//  Petites structures de noeud qui `spawn`. (Apparaît et s'autodétruit)
//
//  Created by Corentin Faucher on 2023-10-28.

#ifndef coq_node_poping_h
#define coq_node_poping_h

#include "node_fluid.h"
#include "node_structs.h"
#include "node_string.h"

#include "../coq_timer.h"
#include "../graphs/graph_texture.h"

// MARK: - Poping Base

typedef struct coq_View View;
typedef struct PopingNode_ PopingNode_;
/// Set la vue "en avant" pour les
/// PopingNode.
void PopingNode_setFrontView(View* frontView);

/// Crée un noeud (sans structure) qui s'autodétruit.
/// callBackOpt (optionel) forction de mise à jour à chaque "tic".
void PopingNode_spawn(PopingNode_** refererOpt, float x, float y, float width, float height,
                      float timeSec, PopingInfo popInfo, void (*callBackOpt)(Fluid*,Countdown*));
/// Une fois la structure init, ouvrir.
void popingnode_last_open(void);
void popingnoderef_cancel(PopingNode_** popingref);

/// Convenience constructor situe le noeud au-dessus de nodeOver (en x,y).
/// Placé dans frontScreen si inFrontScreen, sinon dans les descendant de
/// nodeOver. width et height sont relatifs à la hauteur de nodeOver.
void PopingNode_spawnOver(Node *const nodeOver, PopingNode_** refererOpt, float width_rel, float height_rel,
                          float timeSec, PopingInfo popInfo, void (*callBackOpt)(Fluid*,Countdown*));
/// Accès au dernier PopingNode créé pour créer sa structure. 
/// ** Attention, ne pas garder de reference ver un poping node -> s'autodétruit ! 
/// ** On peut donc se retrouver avec un noeud dealloc....              
extern Fluid* popingnode_last_notSharedOpt_;
/// Ajuste la position pour être visible dans l'écran.
void popingnode_last_checkForScreenSpilling(void);


// MARK: - Quelques exemples de PopingNode...

// MARK: - PopDisk, un disque de `progression`, disparaît une fois plein.
void   PopDisk_spawnOverAndOpen(Node *const nodeOverOpt, PopingNode_ **const refererOpt,
                   uint32_t const pngId, uint32_t const tile, float const deltaT,
                   float x, float y, float twoDyRel);

// MARK: - Sparkles ! (des feux d artifices)
void Sparkle_init(Texture* sparkleTex, uint32_t sparkleSoundId);
/// Exemple d'implémentation de PopingNode...
void Sparkle_spawnAtAndOpen(float xabs, float yabs, float delta, Texture *texOpt);
void Sparkle_spawnOverAndOpen(Node *nd, float deltaRatio);

// MARK: - PopMessage, message qui s autodétruit (e.g. error).
/// Autre exemple d'implémentation de PopingNode...
void PopMessage_spawnAtAndOpen(float xabs, float yabs, float twoDxOpt, float twoDy,
                        float timeSec, uint32_t framePngId, NodeStringInit str,
                        FramedStringParams params);
void PopMessage_spawnOverAndOpen(Node *n, float widthOpt_rel, float height_rel,
                          float timeSec, uint32_t framePngId,
                          NodeStringInit str, FramedStringParams params);

#endif /* pop_disk_h */
