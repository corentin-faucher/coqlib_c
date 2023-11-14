#  coqlib c

## Notations sur les fonctions

A_doStuf() : Majustcule, une fonction global de la struct A, e.g. A_create() -> crée une instance de A.
a_doStuf(A* a) : Minuscule, une fonction appliquée à une instance de la struct A, e.g. a_update(my_a).
_a_doPrivateStuff(A* a) : Underscore, pour les variables ou fonction "privées"/"internes".

myRefOpt : Opt, pour les pointeurs/reference optionnelle (de fonction, paramètre, variable...), i.e. *pouvant* être NULL.
A_create(...) : create -> Création et initialisation d'un objet avec allocation de mémoire *malloc*/*calloc*. On est responsable de detruire l'objet -> _destroy.
A_spawn(...) : Création (et init) d'un objet qui s'autodétruit (pas besoin de deinit/destroy).
a_init... : Initialisation (seulement) d'un objet existant.
a_deinit(A* a) : Préparation pour déallocation (dealloc les sous-objets).
a_destroy(A* a) : Deinit et destruction *free* d'un objet.
a_set... : Définir une/des variable d'un objet, e.g. void a_setIsOn(A *a, Bool isOn).
a_update() : Setter indirect, e.g. ChoronoRender_update(60.f).
a_(action)... : Fonction quelconque appliquée à l'objet, e.g. void chrono_pause(Chrono *c).
a_(propriété)... : Getter quelconque ("get" est sous-entendu),
     e.g. Vector2 node_deltas(node* nd) -> hit box (espace occupé) d'un noeud.
a_is... : Is -> Getter de boolean, e.g. Bool countdown_isRinging(Countdown *cd).

## Notations sur les pointeurs

? A** ptrRef : Ref est pour "Référence" d'un pointeur, e.g. pour modifier la valeur du pointeur.
pEnd vs pLast : Last-> pointe sur le dernier élément ou plutôt le dernier actif. End-> Pointe juste après la fin de l'array, e.g. while(p < end) { if(p->actif){...}... }, le dernier élément traité sera "last".
pHead vs pFirst : First -> premier "actif". Head -> début de l'array (superflu en général, correspond au pointeur de l'array).

## Notations sur les arrays

On utilise "Count" pour le nombre d'éléments et "Size" pour la taille en bytes de l'array,
e.g. A myArray[myArrayCount]; myArraySize = myArrayCount * sizeof(A);

