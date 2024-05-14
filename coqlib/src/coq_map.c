//
//  MyHashTable.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-24.
//

#include "coq_map.h"

#include "utils/util_base.h"

#define _MAP_MAX_NAME_COMPARE 64
#define _MAP_MAX_NAME_HASH    80

size_t Map_maxStringLenght = 128;

typedef struct HashSlot HashSlot;
/// Contient les couples key/value.
/// -> size_t size_slot = sizeof(HashSlot) - 1 + size_value;
typedef struct HashSlot {
    HashSlot* next;
    char      key[_MAP_MAX_NAME_COMPARE];
//    size_t    size_value;   // Ajouter ?
    char      valueData[1]; // Taille variable... DataArray avec la value de taille size_value.
} HashSlot;

typedef struct StringMap {
    uint32_t  count;
    size_t    size_value;  // Taille des valeur par defaut.
    uint32_t  it_index;    // Pour iterer dans la map.
    HashSlot* it_hs;
    // TODO: Faire une version avec un array de HashSlot de taille fixe ? (au lieu d'un array de pointeurs) ??
    HashSlot* table[1];    // Liste de pointeurs vers les Hashslots. (array de taille sizeof(void*) * count.)
} StringMap;

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

HashSlot*  _hashslot_create(const char* key, const char* valueDataOpt, size_t size_value) {
    size_t size_slot = sizeof(HashSlot) - 1 + size_value;
//    size_t size_value = size_slot - sizeof(HashSlot) + 1;
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
HashSlot* _hashslot_foundFirstInListWithKey(HashSlot* hs, const char* key) {
    while(hs) {
        if(strncmp(key, hs->key, _MAP_MAX_NAME_COMPARE-1) == 0) {
            return hs;
        }
        hs = hs->next;
    }
    return NULL;
}
/// Retourne la nouvelle tete de liste. (Peut etre NULL, si plus d'elements.)
HashSlot* _hashslot_destroyWithKey(HashSlot* const hsHead, const char* key, void (*value_deinitOpt)(void*)) {
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
void  _hashslot_print(HashSlot* hs, void (*printValue)(const char*)) {
    int i = 0;
    while(hs) {
        if(i > 0)
            printf("   ");
        printf(" %d) key: %s -> value: ", i, hs->key);
        printValue(hs->valueData);
        printf("\n");
        i++;
        hs = hs->next;
    }
}

StringMap* Map_create(uint32_t count, size_t size_value) {
    size_t map_size = sizeof(StringMap) + sizeof(HashSlot*) * (count - 1);
    StringMap* ht = coq_calloc(1, map_size);
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
void _print_string(const char* str) {
    printf("%s", str);
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
                _hashslot_print(*p, printValueOpt);
            else
                _hashslot_print(*p, _print_string);
        }
        p++; hash++;
    }
    printf("].\n");
}
char* map_putWithSize(StringMap* map, const char* key, const void* const valueDataOpt, const size_t size_value) {
    size_t hashKey = _hash(key) % map->count;
    HashSlot** const hsref = &map->table[hashKey];
    // Emplacement vide. Premier element.
    if(*hsref == NULL) {
        HashSlot* newHs = _hashslot_create(key, valueDataOpt, size_value);
        *hsref = newHs;
        return newHs->valueData;
    }
    // Sinon verifier si existe.
    HashSlot* hs = _hashslot_foundFirstInListWithKey(*hsref, key);
    if(hs == NULL) { // Pas trouvé -> Ajout (insertion dans la liste)
        HashSlot* newHs = _hashslot_create(key, valueDataOpt, size_value);
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
    return hs->valueData;
}
char* map_putAsString(StringMap* map, const char* key, const char* const string) {
    if(string == NULL) {
        printwarning("No string to put in map.");
        return NULL;
    }
    size_t size_value = strnlen(string, Map_maxStringLenght) + 1; // +1 pour null terminate...
    return map_putWithSize(map, key, string, size_value);
}
char* map_put(StringMap* map, const char* key, const void* const valueDataOpt) {
    return map_putWithSize(map, key, valueDataOpt, map->size_value);
}
void  map_removeKeyValue(StringMap* map, const char* key, void (*value_deinitOpt)(void*)) {
    if(map->it_hs) {
        printerror("Removing key while iterator active.");
        map->it_hs = NULL;
    }
    size_t hashKey = _hash(key) % map->count;
    HashSlot** const hsHeadRef = &map->table[hashKey];
    *hsHeadRef = _hashslot_destroyWithKey(*hsHeadRef, key, value_deinitOpt);
}
const char* map_valueRefOptOfKey(StringMap* map, const char* key) {
    size_t hashKey = _hash(key) % map->count;
    HashSlot* hs = map->table[hashKey];
    if(hs == NULL)
        return NULL;
    hs = _hashslot_foundFirstInListWithKey(hs, key);
    if(hs != NULL)
        return hs->valueData;
    return NULL;
}

bool map_iterator_init(StringMap* map) {
    map->it_index = 0;
    while(map->it_index < map->count) {
        if(map->table[map->it_index] != NULL) {
            map->it_hs = map->table[map->it_index];
            return true;
        }
        map->it_index++;
    }
    map->it_hs = NULL;
    return false;
}
bool map_iterator_next(StringMap* map) {
    if(map->it_hs == NULL)
        return false;
    // S'il y a d'autre slots...
    if(map->it_hs->next) {
        map->it_hs = map->it_hs->next;
        return true;
    }
    // Sinon chercher le prochain hash.
    map->it_index++;
    while(map->it_index < map->count) {
        if(map->table[map->it_index] != NULL) {
            map->it_hs = map->table[map->it_index];
            return true;
        }
        map->it_index++;
    }
    map->it_hs = NULL;
    return false;
}
bool map_iterator_removeAndNext(StringMap* map, void (*value_deinitOpt)(void*)) {
    if(map->it_hs == NULL)
        return false;
    // Trouver le précément
    HashSlot* last = NULL;
    HashSlot* next = NULL;
    HashSlot* hs = map->table[map->it_index];
    bool didRemove = false;
    while(hs) {
        if(hs == map->it_hs) {
            if(last == NULL) { // Retrait de head.
                map->table[map->it_index] = hs->next;
            } else {
                last->next = hs->next;
            }
            next = hs->next;
            if(value_deinitOpt) value_deinitOpt(hs->valueData);
            coq_free(hs);
            didRemove = true;
            break;
        }
        last = hs;
        hs = hs->next;
    }
    if(!didRemove)
        printerror("Did not remove element.");
    // S'il y a d'autre slots...
    if(next) {
        map->it_hs = next;
        return true;
    }
    // Sinon chercher le prochain hash.
    map->it_index++;
    while(map->it_index < map->count) {
        if(map->table[map->it_index] != NULL) {
            map->it_hs = map->table[map->it_index];
            return true;
        }
        map->it_index++;
    }
    map->it_hs = NULL;
    return false;
}
const char* map_iterator_valueRefOpt(StringMap* map) {
    if(map->it_hs == NULL)
        return NULL;
    return map->it_hs->valueData;
}

void map_applyToAll(StringMap* map, void (*block)(char* valueData)) {
    if(!map)
        return;
    if(!map_iterator_init(map))
        return;
    do {
        block(map->it_hs->valueData);
    } while(map_iterator_next(map));
}

void _printint(const char* intRef) {
    if(intRef == NULL) {
        printf("Nothing to print!\n");
        return;
    }
    printf("%d", *((int*)intRef));
}



void Map_test_(void) {
    StringMap* ht = Map_create(10, sizeof(int));
    int value = 5;
    map_put(ht, "Gustave", &value);
    value = 7;
    map_put(ht, "Monique", &value);

    map_print(ht, _printint);

    value = 15;
    map_put(ht, "Roger", &value);
    value = 1;
    map_put(ht, "Alber", &value);
    value = -15;
    map_put(ht, "Gustave", &value);
    value = 42;
    map_put(ht, "Antoine", &value);

    map_print(ht, _printint);


    value = 15;
    map_put(ht, "Flaubert", &value);
    value = 1;
    map_put(ht, "George", &value);
    value = -15;
    map_put(ht, "Fluffy", &value);
    value = 42;
    map_put(ht, "Bertrand", &value);

    map_print(ht, _printint);

    const char* gVal = map_valueRefOptOfKey(ht, "George");
    const char* zVal = map_valueRefOptOfKey(ht, "Zoe");
    printf("Values of George and Zoe");
    _printint(gVal); _printint(zVal);
}


