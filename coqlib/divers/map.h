//
//  MyHashTable.h
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-24.
//

#ifndef MyHashTable_h
#define MyHashTable_h

#include "utils.h"

/// Struct pour un dictionnaire / map.
typedef struct _MyStringMap StringMap;

/// La taille maximale des strings pouvant etre stockées par defaut avec map_putAsString.
extern size_t Map_maxStringLenght;  // 128 par defaut.

/// Une hash map avec des strings en key.
/// Les valeurs sont des donnees (char valueData[...]) de taille size_value.
/// size_value est la taille par defaut. On peut toujours specifier la taille de valueData avec map_putWithSize.
StringMap* Map_create(uint32_t count, size_t size_value);
void       map_destroyAndNull(StringMap** const map, void (*value_deinit)(void*));
/// Pour l'affichage/debuging, il faut passer une fonction qui print un char value[] (un char*).
void  map_print(StringMap* map, void (*printValue)(const char*));
/// Store une copie des données pointées par valueDataOpt.
/// Retourne la référence de la copie crée.
/// Si on passe null à valueDataOpt, on crée une nouvelle entrée (si absent) avec les données à zero.
/// Ici on specifie la taille des donnees stockes.
char* map_putWithSize(StringMap* map, const char* key, const void* valueDataOpt, const size_t size_value);
/// Convenience version of map_putWithSize for strings.
char* map_putAsString(StringMap* map, const char* key, const char* string, Bool overwrite);
/// Convenience version of map_putWithSize using the usual size of values size_value of the map.
char* map_put(StringMap* map, const char* key, const void* valueDataOpt);
/// Enlever (free) un couple key/valueData.
void  map_removeKeyValue(StringMap* map, const char* key);
/// Optenir la valeur d'une string.
/// Retourne une *reference* vers les données value[] dans la map.
const char* map_valueRefOptOfKey(StringMap* map, const char* key);

/// Pour iterer dans la map. Retourne false s'il n'y a rien.
Bool        map_iterator_init(StringMap* map);
/// Se place sur le prochain element a afficher. Retourne false si ne trouve plus rien.
Bool        map_iterator_next(StringMap* map);
/// Retourne une *reference* des données value[] ou on est rendu.
const char* map_iterator_valueRefOpt(StringMap* map);

/// Applique la fonction block à tout les élément de la map.
void map_applyToAll(StringMap* map, void (*block)(char* valueData));


void _Map_test(void);

#endif /* MyHashTable_h */
