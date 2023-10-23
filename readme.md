#  coqlib c

## Notations sur les fonctions

Node_... : Une fonction "static" (Avec majuscules), e.g. les "create".
```node_...(Node*)``` : Une fonction appliquée à une instance, ici de struc "Node".
```_MyStuff...``` : Underscore pour les variables ou fonction "privées"/"internes".

...Opt : Fonction, paramètre ou variable pouvant être NULL.
..._create : Création et initialisation d'un objet avec allocation de mémoire (malloc).
..._init : Initialisation (sans allocation) d'un objet.
..._set... : Définir une/des variable d'un objet, e.g. void smtrans_setIsOn(SmTrans *st, int isOn).
..._update... : Setter indirect, e.g. ChoronoRender_update(60.f).
..._(action)... : Fonction quelconque appliquée à l'objet, e.g. void chrono_pause(Chrono *c).
..._(propriété)... : Getter ("get" est sous-entendu),
     e.g. Vector2 node_dims(node* nd) : ici get w/h d'un noeud.
..._is... : Getter de boolean.

