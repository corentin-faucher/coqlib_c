//
//  MyHashTable.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-24.
//

#include "util_map.h"

#include "util_base.h"
#include "../maths/math_base.h"

#define _MAP_MAX_NAME_COMPARE 64
#define _MAP_MAX_NAME_HASH    80

uint32_t _hash(const char *name) {
    uint32_t hash = 0;
    const char* p = name;
    const char* end = name + _MAP_MAX_NAME_HASH;
    while(p < end && *p) {
        // Change pas grand chose en fait... ? Faut juste un truc random.
        hash = (hash << 4) ^ *p ^ hash;
        p++;
    }
    return hash;
}
void print_string_(const char* str) {
    printf("%s", str);
}

typedef struct HashSlot HashSlot;
/// Contient les couples key/value.
/// -> size_t size_slot = sizeof(HashSlot) - 1 + size_value;
typedef struct HashSlot {
    HashSlot* next;
    char      key[_MAP_MAX_NAME_COMPARE];
//    size_t    size_value;   // Ajouter ?
    char      valueData[1]; // Taille variable... DataArray avec la value de taille size_value.
} HashSlot;
HashSlot* HashSlot_create_(const char* key, const char* valueDataOpt, size_t size_value) {
    size_t size_slot = sizeof(HashSlot) - 1 + size_value;
    HashSlot* hs = coq_calloc(1, size_slot);
    strncpy(hs->key, key, _MAP_MAX_NAME_COMPARE);
    // (s'assurer d'avoir une string fini pour l'affichage)
    hs->key[_MAP_MAX_NAME_COMPARE-1] = '\0';
    if(valueDataOpt)
        memcpy(hs->valueData, valueDataOpt, size_value);
    else
        memset(hs->valueData, 0, size_value);
    return hs;
}
void      hashslot_init_(HashSlot*const slot, char const*const key, char const*const valueDataOpt, size_t const value_size)
{
    strncpy(slot->key, key, _MAP_MAX_NAME_COMPARE);
    // (s'assurer d'avoir une string fini pour l'affichage)
    slot->key[_MAP_MAX_NAME_COMPARE-1] = '\0';
    if(valueDataOpt)
        memcpy(slot->valueData, valueDataOpt, value_size);
    else
        memset(slot->valueData, 0, value_size);
}
typedef struct {
    HashSlot* previous;
    HashSlot* slot;
} SlotPair;

SlotPair hashslot_foundFirstInListWithKey_(HashSlot* slot, const char*const key) {
    HashSlot* previous = NULL;
    for(; slot; previous = slot, slot = slot->next) {
        if(strncmp(key, slot->key, _MAP_MAX_NAME_COMPARE-1) == 0) {
            return (SlotPair) { .previous = previous, .slot = slot, };
        }
    }
    return (SlotPair) {};
}
/// Retourne la nouvelle tete de liste. (Peut etre NULL, si plus d'elements.)
HashSlot* hashslot_destroyWithKey_(HashSlot* const hsHead, const char* key, void (*value_deinitOpt)(void*)) {
    HashSlot* last = NULL;
    HashSlot* hs = hsHead;
    while(hs) {
        if(strncmp(key, hs->key, _MAP_MAX_NAME_COMPARE-1) == 0) {
            HashSlot* newHead = hsHead;
            if(hs == hsHead) {
                newHead = hs->next;
            } else {
                last->next = hs->next;
            }
            if(value_deinitOpt) value_deinitOpt(hs->valueData);
            coq_free(hs);
            return newHead;
        }
        last = hs;
        hs = hs->next;
    }
    printwarning("Key %s not found", key);
    return hsHead;
}
void      hashslot_print_(HashSlot const* hs, void (*printValue)(const char*)) {
    int i = 0;
    for(; hs; hs = hs->next, i++) {
        if(i > 0)
            printf("   ");
        printf(" %d) key: %s -> value: ", i, hs->key);
        printValue(hs->valueData);
        printf("\n");
    }
}

typedef struct StringMap {
    uint32_t const count;
    size_t const   value_size;
    size_t const   slot_size;
    uint32_t       it_index;
    HashSlot*      it_slot;
    char           table_data[1];
} StringMap;

StringMap* Map_create(uint32_t const count, size_t const value_size) {
    size_t const slot_size = sizeof(HashSlot) - 1 + value_size;
    size_t const map_size = sizeof(StringMap) - 1 + slot_size*count;
    StringMap*const map = coq_calloc(1, map_size);
    uint_initConst(&map->count, count);
    size_initConst(&map->value_size, value_size);
    size_initConst(&map->slot_size, slot_size);
    return map;
}
void       map_destroyAndNull(StringMap** const mapOptRef, void (*value_deinitOpt)(void*)) {
    if(*mapOptRef == NULL) return;
    StringMap*const map = *mapOptRef;
    *mapOptRef = NULL;
    char (*const tbsl_beg)[map->slot_size] = (char (*)[map->slot_size])map->table_data;
    char (*tbsl)[map->slot_size] =      tbsl_beg;
    char (*tbsl_end)[map->slot_size] = &tbsl_beg[map->count];
    for(; tbsl < tbsl_end; tbsl++) {
        HashSlot*const table_slot = (HashSlot*)tbsl;
        if(!table_slot->key[0]) continue;
        HashSlot* slot = table_slot->next;
        while(slot) {
            HashSlot*const toDelete = slot;
            slot = slot->next;
            if(value_deinitOpt) value_deinitOpt(toDelete->valueData);
            printf("*");
            coq_free(toDelete);
        }
    }
    // Destroy
    coq_free(map);
}
void  map_print(StringMap const*const map, void (*printValueOpt)(const char*)) {
    // Itérateur sur les slots.
    char (*const hs_beg)[map->slot_size] = (char (*)[map->slot_size])map->table_data;
    char (*hs)[map->slot_size] =      hs_beg;
    char (*hs_end)[map->slot_size] = &hs_beg[map->count];
    uint32_t index = 0;
    printf("[\n");
    for(; hs < hs_end; hs++, index++) {
        HashSlot const*const slot = (HashSlot const*)hs;
        if(!slot->key[0]) continue;
        printf(" hash %d : ", index);
        hashslot_print_(slot, printValueOpt ? printValueOpt : print_string_);
    }
    printf("].\n");
}
char const* map_put(StringMap*const map, const char*const key, void const*const valueDataOpt)
{
    if(!key) { printerror("No key."); return NULL; }
    if(!key[0]) { printerror("Empty key."); return NULL; }
    size_t hashKey = _hash(key) % map->count;
    char (*const table)[map->slot_size] = (char (*)[map->slot_size])map->table_data;
    HashSlot*const slot = (HashSlot*)&table[hashKey];
    // Emplacement vide. Premier element.
    if(!slot->key[0]) {
        hashslot_init_(slot, key, valueDataOpt, map->value_size);
        return slot->valueData;
    }
    // Sinon verifier si existe.
    SlotPair const pair = hashslot_foundFirstInListWithKey_(slot, key);
    // Retourner la valeur existante.
    if(pair.slot) return pair.slot->valueData;
    // Pas trouvé -> Ajout
    HashSlot*const newSlot = HashSlot_create_(key, valueDataOpt, map->value_size);
    newSlot->next = slot->next; // (insertion dans la liste)
    slot->next = newSlot;
    return newSlot->valueData;
}
char const* map_putAsString(StringMap*const map, const char* key, const char*const string) {
    if(string == NULL) {
        printwarning("No string to put in map.");
        return NULL;
    }
    size_t string_lenght = strnlen(string, map->value_size);
    if(string_lenght >= map->value_size) {
        printwarning("String too long.");
        char*const copy = coq_callocSimpleArray(map->value_size, char);
        strncpy(copy, string, map->value_size - 1);
        char const*const value = map_put(map, key, copy);
        coq_free(copy);
        return value;
    }
    return map_put(map, key, string);
}
void        map_removeKeyValue(StringMap*const map, const char*const key, 
                                void (*const value_deinitOpt)(void*)) 
{
    if(map->it_slot) {
        printerror("Removing key while iterator active.");
        map->it_slot = NULL;
    }
    size_t hashKey = _hash(key) % map->count;
    char (*const table)[map->slot_size] = (char (*)[map->slot_size])map->table_data;
    HashSlot*const table_slot = (HashSlot*)&table[hashKey];
    SlotPair const pair = hashslot_foundFirstInListWithKey_(table_slot, key);
    if(!pair.slot) { // Cas inexistant...
        printwarning("Key %s not found", key);
        return;
    }
    if(pair.previous) { // Cas slot supplémentaire, retirer.
        pair.previous->next = pair.slot->next;
        coq_free(pair.slot);
        return;
    }
    if(!table_slot->next) { // Cas juste la slot de table.
        *table_slot = (HashSlot) {}; // Reset à zero.
        return;
    }
    // Sinon le premier de la liste devient la slot de table et on l'efface.
    HashSlot*const oldFirst = table_slot->next;
    *table_slot = *oldFirst;
    coq_free(oldFirst);
}
const char* map_valueRefOptOfKey(StringMap const*const map, const char*const key) {
    size_t hashKey = _hash(key) % map->count;
    char (*const table)[map->slot_size] = (char (*)[map->slot_size])map->table_data;
    HashSlot*const table_slot = (HashSlot*)&table[hashKey];
    SlotPair const pair = hashslot_foundFirstInListWithKey_(table_slot, key);
    if(pair.slot) return pair.slot->valueData;
    return NULL;
}

bool map_iterator_init(StringMap* map) {
    map->it_index = 0;
    map->it_slot = NULL;
    char (*const hs_beg)[map->slot_size] = (char (*)[map->slot_size])map->table_data;
    char (*hs)[map->slot_size] =      hs_beg;
    char (*hs_end)[map->slot_size] = &hs_beg[map->count];
    for(; hs < hs_end; hs++, map->it_index++) {
        HashSlot*const slot = (HashSlot*)hs;
        if(slot->key[0]) {
            map->it_slot = slot;
            return true;
        }
    }
    return false;
}
bool map_iterator_next(StringMap* map) {
    if(map->it_slot == NULL)
        return false;
    // S'il y a d'autre slots dans la liste.
    if(map->it_slot->next) {
        map->it_slot = map->it_slot->next;
        return true;
    }
    // Sinon prochain dans la table.
    map->it_index++;
    char (*const hs_beg)[map->slot_size] = (char (*)[map->slot_size])map->table_data;
    char (*hs)[map->slot_size] =     &hs_beg[map->it_index];
    char (*hs_end)[map->slot_size] = &hs_beg[map->count];
    for(; hs < hs_end; hs++, map->it_index++) {
        HashSlot*const slot = (HashSlot*)hs;
        if(slot->key[0]) {
            map->it_slot = slot;
            return true;
        }
    }
    map->it_slot = NULL;
    return false;
}
// Utile ?
//bool map_iterator_removeAndNext(StringMap* map, void (*value_deinitOpt)(void*)) {
//    if(map->it_slot == NULL)
//        return false;
//    // Trouver le précément
//    HashSlot* last = NULL;
//    HashSlot* next = NULL;
//    HashSlot* hs = map->table[map->it_index];
//    bool didRemove = false;
//    while(hs) {
//        if(hs == map->it_hs) {
//            if(last == NULL) { // Retrait de head.
//                map->table[map->it_index] = hs->next;
//            } else {
//                last->next = hs->next;
//            }
//            next = hs->next;
//            if(value_deinitOpt) value_deinitOpt(hs->valueData);
//            coq_free(hs);
//            didRemove = true;
//            break;
//        }
//        last = hs;
//        hs = hs->next;
//    }
//    if(!didRemove)
//        printerror("Did not remove element.");
//    // S'il y a d'autre slots...
//    if(next) {
//        map->it_hs = next;
//        return true;
//    }
//    // Sinon chercher le prochain hash.
//    map->it_index++;
//    while(map->it_index < map->count) {
//        if(map->table[map->it_index] != NULL) {
//            map->it_hs = map->table[map->it_index];
//            return true;
//        }
//        map->it_index++;
//    }
//    map->it_hs = NULL;
//    return false;
//}
const char* map_iterator_valueRefOpt(StringMap* map) {
    if(map->it_slot == NULL)
        return NULL;
    return map->it_slot->valueData;
}

void map_applyToAll(StringMap* map, void (*block)(char* valueData)) {
    if(!map) return;
    if(!map_iterator_init(map)) return;
    
    do {
        block(map->it_slot->valueData);
    } while(map_iterator_next(map));
}

void test_printint_(const char* intRef) {
    if(intRef == NULL) {
        printf("Nothing to print!");
        return;
    }
    printf("%d", *((int*)intRef));
}


void Map_test_(void) {
    StringMap* ht = Map_create(10, sizeof(int));
    int value = 5;
    map_put(ht, "Gustave", &value);
    value = 7;
    map_put(ht, "Monique", &(int){1});
    
    map_put(ht, "", &(int) { 1 }); // -> Erreur.

    map_print(ht, test_printint_);

    value = 15;
    map_put(ht, "Roger", &value);
    value = 1;
    map_put(ht, "Alber", &value);
    value = -15;
    map_put(ht, "Gustave", &value);
    value = 42;
    map_put(ht, "Antoine", &value);

    map_print(ht, test_printint_);


    value = 15;
    map_put(ht, "Flaubert", &value);
    value = 1;
    map_put(ht, "George", &value);
    value = -15;
    map_put(ht, "Fluffy", &value);
    value = 42;
    map_put(ht, "Bertrand", &value);

    map_print(ht, test_printint_);
    
    map_removeKeyValue(ht, "Fluffy", NULL);
    
    map_print(ht, test_printint_);

    const char* gVal = map_valueRefOptOfKey(ht, "George");
    const char* zVal = map_valueRefOptOfKey(ht, "Zoe");
    printf("George value : ");
    test_printint_(gVal); 
    printf(".\nZoe value : ");
    test_printint_(zVal);
    printf(".\nTest fini, delete map...\n");
    map_destroyAndNull(&ht, NULL);
}


// Garbage
/* 
typedef struct StringMap {
    uint32_t  count;
    size_t    size_value;  // Taille des valeur par defaut.
    uint32_t  it_index;    // Pour iterer dans la map.
    HashSlot* it_hs;
    // TODO: Faire une version avec un array de HashSlot de taille fixe ? (au lieu d'un array de pointeurs) ??
    HashSlot* table[1];    // Liste de pointeurs vers les Hashslots. (array de taille sizeof(void*) * count.)
} StringMap;
StringMap* Map_create(uint32_t const count, size_t const size_value) {
    StringMap* ht = coq_callocArray(StringMap, HashSlot*, count);
    ht->count = count;
    ht->size_value = size_value;
    return ht;
}
void       map_destroyAndNull(StringMap** const mapOptRef, void (*value_deinitOpt)(void*)) {
    if(*mapOptRef == NULL) return;
    // Deinit
    HashSlot** p = (*mapOptRef)->table;
    HashSlot** end = &(*mapOptRef)->table[(*mapOptRef)->count];
    HashSlot* last;
    HashSlot* hs;
    while(p < end) {
        if(*p != NULL) {
            hs = *p;
            while(hs) {
                last = hs;
                hs = hs->next;
                if(value_deinitOpt)
                    value_deinitOpt(last->valueData);
                coq_free(last);
            }
        }
        p++;
    }
    // Destroy
    coq_free(*mapOptRef);
    // Null
    *mapOptRef = NULL;
}
void  map_print(StringMap* map, void (*printValueOpt)(const char*)) {
    HashSlot** p = map->table;
    HashSlot** end = &map->table[map->count];
    printf("[\n");
    uint32_t hash = 0;
    while(p < end) {
        if(*p != NULL) {
            printf(" hash %d : ", hash);
            if(printValueOpt)
                hashslot_print_(*p, printValueOpt);
            else
                hashslot_print_(*p, print_string_);
        }
        p++; hash++;
    }
    printf("].\n");
}
char const* map_putWithSize(StringMap* map, const char* key, const void* const valueDataOpt, const size_t size_value) {
    size_t hashKey = _hash(key) % map->count;
    HashSlot** const hsref = &map->table[hashKey];
    // Emplacement vide. Premier element.
    if(*hsref == NULL) {
        HashSlot* newHs = HashSlot_create_(key, valueDataOpt, size_value);
        *hsref = newHs;
        return newHs->valueData;
    }
    // Sinon verifier si existe.
    SlotPair const pair = hashslot_foundFirstInListWithKey_(*hsref, key);
    if(pair.slot == NULL) { // Pas trouvé -> Ajout (insertion dans la liste)
        HashSlot* newHs = HashSlot_create_(key, valueDataOpt, size_value);
        newHs->next = *hsref;
        *hsref = newHs;
        return newHs->valueData;
    }
    // Sinon remplacer la valeur existante ?
    // Non, finalement, pas de overwrite... size_value peut varier d'une slot a l'autre... (et d'une fois a l'autre)
//    if(overwrite) {
//        if(valueDataOpt)
//            memcpy(hs->valueData, valueDataOpt, size_value);
//        else
//            memset(hs->valueData, 0, size_value);
//    }
    // Retourner la valeur existante.
    return pair.slot->valueData;
}
char const* map_putAsString(StringMap* map, const char* key, const char* const string) {
    if(string == NULL) {
        printwarning("No string to put in map.");
        return NULL;
    }
    size_t size_value = strnlen(string, Map_maxStringLenght) + 1; // +1 pour null terminate...
    return map_putWithSize(map, key, string, size_value);
}
char const* map_put(StringMap* map, const char* key, const void* const valueDataOpt) {
    return map_putWithSize(map, key, valueDataOpt, map->size_value);
}
void  map_removeKeyValue(StringMap* map, const char* key, void (*value_deinitOpt)(void*)) {
    if(map->it_hs) {
        printerror("Removing key while iterator active.");
        map->it_hs = NULL;
    }
    size_t hashKey = _hash(key) % map->count;
    HashSlot** const hsHeadRef = &map->table[hashKey];
    *hsHeadRef = hashslot_destroyWithKey_(*hsHeadRef, key, value_deinitOpt);
}
const char* map_valueRefOptOfKey(StringMap* map, const char* key) {
    size_t hashKey = _hash(key) % map->count;
    HashSlot* hs = map->table[hashKey];
    if(hs == NULL)
        return NULL;
    SlotPair const pair = hashslot_foundFirstInListWithKey_(hs, key);
    if(pair.slot != NULL)
        return pair.slot->valueData;
    return NULL;
}
*/
