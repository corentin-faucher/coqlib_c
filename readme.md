#  coqlib c

## Tester avec Makefile

0. Installer les dependences GLEW et SDL.
```bash
    coqlib_c $ sudo apt-get install libglew-dev libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libglm-dev libunwind-dev
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

3. Essayer de modifier les objets affichés (ici avec l'editeur texte sublime).
```bash
    coqlib_test $ subl src/my_root.c
```

## Tester dans Xcode (macOS)

1. Créer un projet macOS -> App ; mom "xc_coqlib_test", interface "XIB" (pas important, on va effacer le .xib) ; language Objective-C. Placer dans le répertoire de `coqlib_c`.

2. Effacer les fichier créés automatiquement : AppDelegate.h et .m, Assets.xcassets, MainMenu.xib, main.m. -> Move to trash.

3. Dans le projet (xc_coqlib_test.xcodeproj la racine des fichiers), dans `Info`, effacer `MainMenu` de `Main nib file base name`. Dans `Build Settings` effacer `MainMenu` de `Main nib file base name`.

4. Toujours dans `Build Settings` : Search Paths -> Header Search Paths, ajouter "../coqlib/src".

5. Au projet (xc_coqlib_test la racine des fichiers), ajouter les groupes (`New Group`) : `coqlib` ; `Resources` ; `code`.

6. Au groupe `coqlib`, ajouter (Add Files to xc_coqlib_test) les repertoires `src` et `src_apple` de `coqlib` (remonter au dossier racine de coqlib_c). Choisir "Create groups" pour "Added folders".
   Retirer le sous-groupe `src/opengl_openal` (Remove references). On utilise en fait Metal et AVFoundation au lieu d'OpenGL et d'OpenAL respectivement (voirs les fichiers dans `src_apple`). 
   Retirer aussi les fichiers pour iOS : `src_apple/ios_...`.
   Les fichiers retiré, peuvent aussi seulement être retiré du "Target membership" (dans l'onglet "File inspector").

7. Au groupe `Resources`, ajouter (Add Files to xc_coqlib_test) les repertoires `pngs`, `pngs_coqlib` et `wavs` dans `coqlib_test/res`. Choisir "Create folder references" pour "Added folders". Ajouter aussi `Assets.xcassets` et les `.lproj` (localisations) de `coqlib_test/res_apple` avec "Create groups" pour "Added folders".

8. Au groupe `xc_coqlib_test`, ajouter les fichiers `AppDelegate.h`, `AppDelegate.m`, `main.m` et `Shaders.metal` de `coqlib_test/src_apple`.

9. Au groupe `code`, ajouter tous les fichiers en `my_*` de `coqlib_test/src`.

9. C'est tout. Run cmd-r.

10. On peut aussi ajouter le script pour trier automatiquement les fichiers en ordre alphabétique...
Cliquer le target `xc_coqlib_test` -> `Edit Scheme...` -> `Build` -> `Post action` -> `New run script action` :
```bash
perl "${PROJECT_DIR}/../coqlib_test/res_apple/sort_xcode_project" "${PROJECT_FILE_PATH}/project.pbxproj"
```

## Notations sur les fonctions

- `A_doStuf()` : Majuscule, une fonction global de la struct A, e.g. `A_create()` -> crée une instance de A.
- `a_doStuf(A* a)` : Minuscule, une fonction appliquée à une instance de la struct A, e.g. `a_update(my_a)`.
- `a_doPrivateStuff_(A* a)` : Underscore, pour les variables ou fonction "privées"/"internes". (On met à la fin car au début c'est réservé...)
- `A_create(...)` : create -> Création (alloc) et initialisation d'un objet avec allocation de mémoire *malloc*/*calloc*. On est responsable de detruire l'objet -> `a_destroy`/`a_release`.
- `A_spawn(...)` : Création (et init) d'un objet qui s'autodétruit (pas besoin de deinit/destroy).
- `a_init(A* a)` : Initialisation (seulement) d'un objet existant.
- `a_deinit(A* a)` : Préparation pour déallocation (dealloc les sous-objets).
- `a_destroy(A* a)` : Deinit et deallocation, e.g. call *deinit* et *free* sur l'objet.
- `a_set...` : Setters, définir une/des variable(s) d'un objet, e.g. `void a_setIsOn(A *a, Bool isOn)`.
- `a_(action)...` : Fonction quelconque appliquée à l'objet, e.g. void chrono_pause(Chrono *c).
- `a_(propriété)...` : Getter quelconque ("get" est sous-entendu),
     e.g. `Vector2 node_deltas(node* nd)` -> hit box (espace occupé) d'un noeud.
- `a_is...` : Is -> Getter de boolean, e.g. `Bool countdown_isRinging(Countdown *cd)`.
- `a_as...` : "as"/Downcasting, essaie de caster en tant que sous-struct, e.g. `Drawable *d = node_asDrawableOpt(node); if(d) draw(d);`.
- `a_arr_...` : arr ou array, fonction agissant sur un array d'éléments, e.g. Conversion de 4 "fluides" en un vecteur de 4 float : `Vector4 fl_array_toVec4(FluidPos *fl_arr);`.

## Notations sur les pointeurs

- `A** ptrRef` : Ref est pour "Référence" d'un pointeur, e.g. pour modifier où on pointe.
- `end` vs `last` : last pointe sur le dernier élément _actif_. end pointe juste après la fin de l'array, e.g. `while(p < end) { if(p->actif) doStuf(p); p++; }`, le dernier élément traité sera le "last".
- `head` vs `first` : First -> premier "actif". Head -> début de l'array (superflu en général, correspond au pointeur de l'array).
- `unowned` : On n'est *pas* propriétaire du contenu du pointeur, i.e. il ne faut pas dealloc/`free`.
- `myObjOpt` : Opt, pour les pointeurs/reference optionnels (de fonction, paramètre, variable...), i.e. *pouvant* être NULL. _Normalement_, s'il n'y a pas de *opt*, il est superflu de vérifier si le pointeur est NULL.

## Notations sur les arrays

- `count` vs `size` : On utilise "Count" pour le nombre d'éléments et "Size" pour la taille en bytes de l'array,
e.g. `A myArray[myArray_count]; myArray_size = myArray_count * sizeof(A);`

## Notes diverses sur le C...

- Pour afficher en hexadecimal (dans une formated string), on utilise,
  e.g. "%#010x". # pour 0x, x pour hexadec, 10 pour 10 chars (en comptant 0x).

-Pour identifier une fonction comme étant obsolete:
```
__attribute__((deprecated("utiliser `coq_calloc` + `node_init_` + `drawable_init_`.")))
```

-Attention ! Pour les array en 2D et plus... ! Il faut aller des crochets exterieur vers les crochets intérieurs...
Ou premier crochet -> grand pas, dernier crochet -> petit pas...
e.g. `int mat[4][3]` => `[[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12]]`,
ou encore `const char someSmallStrings[][4] = { "ab", "cd", "ef", "jk", "lm", "no", "pq" };`.
On accède au "haut" niveau en premier.

-Itérateur dans array multi-D: Soit `int mat[4][3];`, l'itérateur de ligne serait `int (*ligne)[3] = &mat[0];`.

-Truc pour initialiser un const dans une struct (ou pour `cast away the const`): `*(uint32_t*)&my_struct->constCount = theCount;`.

