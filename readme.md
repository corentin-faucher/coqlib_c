#  coqlib c

## Tester avec Makefile

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

## Tester dans Xcode

1. Créer un projet macOS -> App ; mom "coqlib_test_xcode", interface "XIB" (pas important, on va effacer le .xib) ; language Objective-C.

2. Effacer les fichier créé automatiquement (AppDelegate.h et .m, Assets.xcassets, MainMenu.xib, main.m). -> Move to trash.

3. Dans le projet (coqlib_test_xcode.xcodeproj la racine des fichiers), dans `Info`, effacer `MainMenu` de `Main nib file base name`. Dans `Build Settings` effacer `MainMenu` de `Main nib file base name`.

4. Toujours dans `Build Settings` : Search Paths -> Header Search Paths, ajouter "../coqlib/include".

5. Au projet (coqlib_test_xcode la racine des fichiers), ajouter les groupes (`New Group`) : `coqlib` ; `Resources`.

6. Au groupe/dossier `coqlib`, ajouter (Add Files to coqlib_test_xcode) les repertoires `include`, `src` et `src_apple` de `coqlib` (remonter au dossier racine de coqlib_c). Choisir "Create groups" pour "Added folders".

7. Au groupe `Resources`, ajouter (Add Files to coqlib_test_xcode) les repertoires `pngs`, `pngs_coqlib` et `wavs` dans `coqlib_test/res`. Choisir "Create folder references" pour "Added folders". Ajouter aussi `Assets.xcassets` et `Strings` dans `coqlib_test/res_apple`.

8. Au groupe `coqlib_test_xcode`, ajouter les fichiers `AppDelegate.h`, `AppDelegate.m`, `main.m` et `Shaders.metal` de `coqlib_test/src_apple`. Ajouter aussi les fichiers `my_enums.c`, `my_root.c` de `coqlib_test/src` et `my_enums.h`, `my_root.h` de `coqlib_test/include`.

9. C'est tout. Run cmd-r.




## Notations sur les fonctions

- `A_doStuf()` : Majustcule, une fonction global de la struct A, e.g. `A_create()` -> crée une instance de A.
- `a_doStuf(A* a)` : Minuscule, une fonction appliquée à une instance de la struct A, e.g. `a_update(my_a)`.
- `_a_doPrivateStuff(A* a)` : Underscore, pour les variables ou fonction "privées"/"internes".
- `A_create(...)` : create -> Création et initialisation d'un objet avec allocation de mémoire *malloc*/*calloc*. On est responsable de detruire l'objet -> _destroy.
- `A_spawn(...)` : Création (et init) d'un objet qui s'autodétruit (pas besoin de deinit/destroy).
- `a_init(A* a)` : Initialisation (seulement) d'un objet existant.
- `a_deinit(A* a)` : Préparation pour déallocation (dealloc les sous-objets).
- `a_destroy(A* a)` : Deinit et destruction *free* d'un objet.
- `a_set...` : Setters, définir une/des variable(s) d'un objet, e.g. `void a_setIsOn(A *a, Bool isOn)`.
- `a_update()` : Setter indirect, e.g. ChoronoRender_update(60.f).
- `a_(action)...` : Fonction quelconque appliquée à l'objet, e.g. void chrono_pause(Chrono *c).
- `a_(propriété)...` : Getter quelconque ("get" est sous-entendu),
     e.g. `Vector2 node_deltas(node* nd)` -> hit box (espace occupé) d'un noeud.
- `a_is...` : Is -> Getter de boolean, e.g. Bool countdown_isRinging(Countdown *cd).
- `a_as...` : "as"/Downcasting, essaie de caster en tant que sous-struct, e.g. `Drawable *d = node_asDrawableOpt(node); if(d) draw(d);`.
- `a_arr_...` : arr ou array, fonction agissant sur un array d'éléments, e.g. Conversion de 4 "fluides" en un vecteur de 4 float : `Vector4 fl_array_toVec4(FluidPos *sp);`.

## Notations sur les pointeurs

- `A** ptrRef` : Ref est pour "Référence" d'un pointeur, e.g. pour modifier où on pointe.
- `pEnd` vs `pLast` : last-> pointe sur le dernier élément ou plutôt le dernier actif. end-> pointe juste après la fin de l'array, e.g. `while(p < end) { if(p->actif) doStuf(p); p++; }`, le dernier élément traité sera le "last".
- `pHead` vs `pFirst` : First -> premier "actif". Head -> début de l'array (superflu en général, correspond au pointeur de l'array).
- `unowned` : On n'est *pas* propriétaire du contenu du pointeur, i.e. pas besoin de `free`.
- `myRefOpt` : Opt, pour les pointeurs/reference optionnels (de fonction, paramètre, variable...), i.e. *pouvant* être NULL. _Normalement_, s'il n'y a pas de *opt*, il est superflu de vérifier si le pointeur est NULL.

## Notations sur les arrays

- `count` vs `size` : On utilise "Count" pour le nombre d'éléments et "Size" pour la taille en bytes de l'array,
e.g. `A myArray[myArrayCount]; myArraySize = myArrayCount * sizeof(A);`

