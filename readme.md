#  coqlib c

## Tester avec Makefile

0.1 Installer Glad. Aller a https://glad.dav1d.de/ 
et generer les fichiers khrplatform.h, glad.h et glad.c 
(e.g. C, OpenGL 4.5, local files). Compiler le .c en .o :
```bash
   c -cc glad.c
```
Faire de l'objet une library (archive d'un fichier) :
```bash
   ar rcs libglad.a glad.o
```
Deplacer les headers dans les includes :
```bash
   sudo mkdir /usr/include/glad
   sudo cp -vi glad.h /usr/include/glad/glad.h
   sudo cp -vi khrplatform.h /usr/include/glad/khrplatform.h
```
Deplacer le .a avec les lib :
```
   sudo cp -vi libglad.a /usr/lib/x86_64-linux-gnu/libglad.a
```

0.2 Installer les dependences : GLFW (interface pour opengl), unwind (debugging), 
freetype (affichage de texte), OpenAL (son).
```bash
    coqlib_c $ sudo apt-get install libglfw3-dev libunwind-dev libfreetype-dev libopenal-dev
```

1. Aller dans le repertoire coqlib. Compiler la librarie.
```bash
    coqlib_c $ cd coqlib
    coqlib $ make
```

2. Aller dans l'exemple et compiler.
```bash
    coqlib $ cd ../coqlib_test
    coqlib_test $ make
```

3. Essayer de modifier les objets affichés (ici avec l'editeur texte sublime, on peut prendre gedit).
```bash
    coqlib_test $ subl src/my_root.c &
```

## Tester dans Xcode (macOS)

1. Créer un projet macOS -> App ; nom "xc_coqlib_test", interface "XIB" (pas important, on va effacer le .xib) ; language Objective-C. Placer dans le répertoire de `coqlib_c`.

2. Effacer les fichier créés automatiquement : AppDelegate.h et .m, Assets.xcassets, MainMenu.xib, main.m. -> Move to trash.

3. Dans le projet (xc_coqlib_test.xcodeproj la racine des fichiers), dans `Info`, effacer `MainMenu` de `Main nib file base name`. Dans `Build Settings` effacer `MainMenu` de `Main nib file base name`.

4. Toujours dans `Build Settings`, ajouter les search paths : Search Paths -> Header Search Paths, ajouter "../coqlib/src"
(ou "/Users/<name>/<projects>/coqlib_c/coqlib/src_apple") et "../coqlib/src_apple". 
Aussi pour Metal : Compiler header search paths : ".../coqlib_c/coqlib/src_apple".

5. Au projet (xc_coqlib_test la racine des fichiers), ajouter les groupes (`New Group`) : `coqlib` ; `Resources` ; `code`.

6. Au groupe `coqlib`, ajouter (Add Files to xc_coqlib_test) les repertoires `src` et `src_apple` de `coqlib` (remonter au dossier racine de coqlib_c). Choisir "Create groups" pour "Added folders".
   Retirer le sous-groupe `src/opengl_openal` (Remove references). On utilise en fait Metal et AVFoundation au lieu d'OpenGL et d'OpenAL respectivement (voirs les fichiers dans `src_apple`).
   Retirer aussi les fichiers pour iOS : `src_apple/ios_...`.
   Les fichiers retiré, peuvent aussi seulement être retiré du "Target membership" (dans l'onglet "File inspector").

7. Au groupe `Resources`, ajouter (Add Files to xc_coqlib_test) les repertoires `pngs`, `pngs_coqlib` et `wavs` dans `coqlib_test/res`. Choisir "Create folder references" pour "Added folders". Ajouter aussi `Assets.xcassets` et les `.lproj` (localisations) de `coqlib_test/res_apple` avec "Create groups" pour "Added folders".

8. Au groupe `xc_coqlib_test`, ajouter les fichiers `AppDelegate.h`, `AppDelegate.m`, `main.m` et `Shaders.metal` de `coqlib_test/src_apple`.

9. Au groupe `code`, ajouter tous les fichiers en `my_*` de `coqlib_test/src`.

9. C'est tout. Run cmd-r.

On peut ajouter aussi l'option `Hidden local variables` dans `Build Settings`.

10. On peut aussi ajouter le script pour trier automatiquement les fichiers en ordre alphabétique...
Cliquer le target `xc_coqlib_test` -> `Edit Scheme...` -> `Build` -> `Post action` -> `New run script action` :
```bash
perl "${PROJECT_DIR}/../coqlib_test/res_apple/sort_xcode_project" "${PROJECT_FILE_PATH}/project.pbxproj"
```

## Notations utilisées pour nommer les structures et leurs méthodes.

Soit la structure
```c
typedef struct A { int myInt; } A;
```
Une instance sera définie par `a`, e.g. `A a = { .myInt = 1};`.
Pour les méthodes, on a :
- `A_doStuf()` : Majuscule, méthode statique (une fonction global de la struct A), e.g. `A_create()` -> crée une instance de A.
- `a_doStuf(A* a)` : Minuscule, méthode d'instance (une fonction appliquée à une instance de la struct A), e.g. `a_update(&a)`.
- `a_doPrivateStuff_(A* a)` : Underscore, pour les variables ou fonction "privées"/"internes". (On met à la fin car au début c'est réservé...)
- `A_create(...)` : create -> Création (alloc) *et* initialisation d'un objet. *On est responsable de detruire l'objet* -> `a_destroy`/`a_release`.
- `A_spawn(...)` : Création (et init) d'un objet qui s'autodétruit (pas besoin de deinit/destroy).
- `a_init(A* a)` : Initialisation d'un objet existant.
- `a_deinit(A* a)` : Préparation pour déallocation (Ne dealloc que les sous-objets, pas l'objet lui-même).
- `a_destroy(A** aRef)` : Deinit et deallocation, e.g. call *deinit* et *free* sur l'objet.
- `a_set...` : Setters, définir une/des variable(s) d'un objet, e.g. `void a_setIsOn(A *a, Bool isOn)`.
- `a_(action)...` : Méthode d'instance quelconque, e.g. void chrono_pause(Chrono *c).
- `a_(propriété)...` : Getter quelconque ("get" est sous-entendu),
     e.g. `Vector2 node_deltas(node* n)` -> hit box (espace occupé) d'un noeud.
- `a_is...` : Is -> Getter de boolean, e.g. `Bool countdown_isRinging(Countdown *cd)`.
- `a_as...` : "as"/Downcasting, essaie de caster en tant que sous-struct, e.g. `Drawable *d = node_asDrawableOpt(node); if(d) draw(d);`.
- `a_arr_...` : arr ou array, fonction agissant sur un array d'éléments, e.g. Conversion de 4 "fluides" en un vecteur de 4 float : `Vector4 fl_array_toVec4(FluidPos *fl_arr);`.


## Notes sur les pointeurs

- Référencer : obtenir l'adresse d'une variable, ce fait avec l'opérateur `&`.
- Déréférencer : obtenir le contenu d'un pointeur (pour l'édition), ce fait avec l'opérateur `*`,
e.g. `int a = 1; int* ptr = &a; *ptr = 2;`. 

- `A** ptrRef` : Ref est pour "Référence" d'un pointeur, e.g. pour modifier où on pointe.
- `end` vs `last` : last pointe sur le dernier élément _actif_. end pointe juste après la fin de l'array, e.g. `while(p < end) { if(p->actif) doStuf(p); p++; }`, le dernier élément traité sera le "last".
- `head` vs `first` : First -> premier "actif". Head -> début de l'array (superflu en général, correspond au pointeur de l'array).
- `unowned` : on n'est *pas* propriétaire du contenu du pointeur, i.e. il ne faut pas dealloc/`free`.
- `given` : on donne l'`ownership` de l'objet. La fonction/objet qui reçoit devient responsable du dealloc.
- `myObjOpt` : Opt, pour les pointeurs/reference optionnels (de fonction, paramètre, variable...), i.e. *pouvant* être NULL. _Normalement_, s'il n'y a pas de *opt*, il est superflu de vérifier si le pointeur est NULL.


## Notations et trucs sur les arrays

- `count` vs `size` : On utilise "Count" pour le nombre d'éléments et "Size" pour la taille en bytes de l'array,
e.g. `A myArray[myArray_count]; myArray_size = myArray_count * sizeof(A);`.

-Attention ! Pour les array en 2D et plus, il faut aller des crochets exterieur vers les crochets intérieurs...
Ou premier crochet -> grand pas, dernier crochet -> petit pas...
e.g. `int mat[4][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}}`,
ou encore `const char someSmallStrings[][4] = { "ab", "cd", "ef", "jk", "lm", "no", "pq" };`.
On accède au "haut" niveau en premier.

-La convention utilisée pour les matrices est qu'une matrice est un array de vecteurs "colonnes", e.g. soit la matrice ci-haut : `int mat[4][3]`, il s'agit d'une matrice de 4 vecteurs de dim. 3, et cette matrice s'écrit (forme mathématique) :
```
mat = ( 1 4 7 10
        2 5 8 11
        3 6 9 12 ).
```
Par contre, pour les images, on considère un "bitmap" comme un ensemble de "lignes" de pixels avec la première ligne en haut de l'image.
```
int pixels[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
pixels = (1  2  3  4
          5  6  7  8
          9 10 11 12).
```
Ceci étant dis, il faut vérifier au cas pas cas... ;)

-Itérateur dans array multi-D: Soit `int mat[4][3];`, l'itérateur de ligne serait `int (*ligne)[3] = &mat[0];`.
Ce type de pointeur peut aussi être utilisé comme un array 2D, e.g. pour transformer un array 1D en array 2D :
```
int numbers[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int (*const matrix)[3] = (int(*)[3])numbers;
int element02 = matrix[2][0]; // (ligne 0, colonne 2, où les colonnes sont les vecteurs.)
```

-Pour retrouver la taille d'un array :
```
char* stringArray[] = { "allo", "un", ...};
size_t arrayCount = sizeof(stringArray) / sizeof(char*);
```


## Notes et trucs diverses sur le C...

- Pour afficher en hexadecimal (dans une formated string), on utilise,
  e.g. "%#010x". # pour 0x, x pour hexadec, 10 pour 10 chars (en comptant 0x).

-Pour identifier une fonction comme étant obsolete:
```
__attribute__((deprecated("Utiliser `coq_calloc` + `node_init_` + `drawable_init_`.")))
```

-Pour "étiqueter" une ligne comme "TODO", ajouter par exemple `#warning TODO: Réviser cette fonction.` ou juste `// TODO: à finir`, `// FIXME: bogue?` (habituellement détectés par l'IDE).

- Pour forcer un alignement compact des données, e.g. un `uint16_t` suivi d'un `uint32_t`, utiliser : `__attribute__((packed))`. Utile par exemple pour définir un header de fichier où la position de chaque variable est importante (voir header de bmp).

-Pour init une variable style Kotlin (.also, .let), utiliser `({ ... });`.
```
int my_var = ({
    int b = 5;
    int c = 3;
    b + c;  // -> my_var = b + c
});
```

-Pour utiliser les "Single instruction multiple data" ou SIMD, utiliser par exemple `__attribute__((vector_size(16)))`. Voir par exemple les `Vector4` dans `math_base.h`.

-Init d'une struct/union :
```
MyStruct const var = (MyStruct) {
    .member0 = 5 + 2,
    .member1 = "allo",
};
```
*Attention, quand on init un union il faut choisir une seule des sous-structure pour l'init.*
Par exemple
```
union MonUnion {
    struct { int   ia, ib, ic, id; };
    struct { float fa, fb, fc, fd; };
};
union MonUnion u = { .ia = 5, .fb = 0.5f, }; // -> ❌ Non ! juste `fb` sera init.
```

-Parfois, c'est pratique d'utiliser des variables "anonymes",
e.g. `(int){ 1 }`, cette variable peut être référencé : `&(int){1}` (rarement utile). 
Sinon les variable anonymes sont surtout utile pour passer les paramètres d'une fonction:
```
ComplexStruct* myStruct = ComplexStruct_create((ComplexStructInit) {
    .param1 = 1, .param2 = 5.f, .str = "allo", .flags = 0x00110,
});
```

-Truc pour initialiser un variable `const` dans une struct (ou pour `cast away the const`): `*(uint32_t*)&my_struct->constCount = theCount;`.
















