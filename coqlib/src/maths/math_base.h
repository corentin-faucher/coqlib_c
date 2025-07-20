//
//  _math_.h
//  Struct et fonction mathématique pratiques.
//
//  Created by Corentin Faucher on 2023-10-12.
//
#ifndef COQ_MATH_BASE_H
#define COQ_MATH_BASE_H

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*#if !defined(MAX)
#define MAX(a, b) \
    ({ __typeof__(a) _a_ = a; \
       __typeof__(b) _b_ = b; \
       (_a_ < _b_) ? _b_ : _a_;  \
    })
#endif */

// Si on veut l'allignement, utiliser
// struct __attribute__((aligned(16))) Vector { union {...};};
// Simd ou just __attribute__((aligned(16))) ??

// MARK: - Vector2
// Il y a 4 sortes de produits avec les vecteurs :
//  - avec scalaire : _times ;
//  - membre à membre : _product ;
//  - produit scalaire : _dot ;
//  - produit vectoriel : _cross.
/// Vector2 : Paire de 2 float. (Non aligned)
/// Pour une paire de unsigned, voir UintPair plus bas.
/// On met le float array en premier pour une init plus simple,
/// e.g. `Vector2 v = {1,2}`.
typedef union {
  float f_arr[2];
  struct {
    float x, y;  // Version position.
  };
  struct {
    float w, h;  // Version tailles.
  };
  struct {
    float theta, radial; // Coordonées polaires (voir conversions `vector2_polarToCartesian` et `vector2_cartesianToPolar`).
  };
  // Pas de version alignée. Sinon on une `struct { Vector2 v1, v2;};` prendrait 8 floats au lieu de 4...?
  //    float __attribute__((vector_size(8))) v;
  //    simd_float2    v;
} Vector2;
#define vector2_ones  (Vector2) {{ 1, 1, }}
#define vector2_zeros (Vector2) {{ 0, 0, }}
// Operations usuelles (similaire à celles de simd...)
// Semble mieux de faire des inlines pour les fonctions simples... (voir math_base.inl)
/// Norme euclidienne, i.e. sqrt(x^2 + y^2).
static inline float   vector2_norm(Vector2 const v);
/// Norme euclidienne carrée, i.e. x^2 + y^2.
static inline float   vector2_norm2(Vector2 const v);
/// Distance euclidienne entre deux points.
static inline float   vector2_distance(Vector2 const p1, Vector2 const p2);
static inline Vector2 vector2_add(Vector2 const v, Vector2 const toAdd);
static inline Vector2 vector2_minus(Vector2 const v, Vector2 const toSubtract);
static inline Vector2 vector2_opposite(Vector2 const v);
/// Produit membre à membre avec scalaire (i.e. float).
static inline Vector2 vector2_times(Vector2 const v, float const f);
/// Produit vectoriel avec vec k (i.e. Rotation de 90deg dans le sens horaire).
static inline Vector2 vector2_cross(Vector2 const v);
/// Produit scalaire.
static inline float   vector2_dot(Vector2 const v1, Vector2 const v2);
/// Produit membre à membre.
static inline Vector2 vector2_product(Vector2 const v1, Vector2 const v2);
static inline Vector2 vector2_mean(Vector2 const a, Vector2 const b);
/// Convertion d'un vecteur polair vers cartésien.
static inline Vector2 vector2_polarToCartesian(Vector2 const v);
static inline Vector2 vector2_cartesianToPolar(Vector2 const v);

/// Projection de v sur target, i.e. proj = (v • t) * t / ||t||^2.
Vector2 vector2_projOn(Vector2 v, Vector2 target);
/// Normalise le vecteur (longueur == 1).
/// Si ~= 0 -> retourne un vecteur unitaire random (voir `rand_vector2_ofNorm,`.)
Vector2 vector2_normalize(Vector2 v);
/// Garde la direction mais modifie la grandeur du vecteur.
Vector2 vector2_toNorm(Vector2 v, float newNorm);
/// Pour l'affichage en debug (le buffer des string est de taille 10, i.e. max de 10 vecteurs simultanés)
const char*const vector2_toString(Vector2 v); 

// MARK: - Paire de unsigned
/// Paire de unsigned.
typedef struct UintPair {
  uint32_t uint0, uint1;
} UintPair;
#define uintpair_zeros (UintPair) { 0, 0 }

// MARK: - Rectangle
/// Structure "Rectangle" pour les zone de NSView, view SDL, etc.
/// L'origine peut être le coin *supérieur* gauche (y inversé)
/// OU le coin *inférieur* gauche  (y normal) dépendant de l'OS/environnement...
typedef union {
  float f_arr[4];
  struct {
    float o_x, o_y, w, h;
  };
  struct {
    Vector2 origin;
    Vector2 size;
  };
} Rectangle;

typedef union {
  uint32_t u_arr[4];
  struct {
    uint32_t o_x, o_y, w, h;
  };
  struct {
    UintPair origin;
    UintPair size;
  };
} RectangleUint;

// Cas particulier où x/y pourraient être négatifs...
typedef union {
  struct {
    int32_t  o_x,   o_y;
    uint32_t width, height;
  };
} RectangleInt;
static inline RectangleInt rectangle_roundedToRectangleInt(Rectangle r);


// MARK: - Box : hitbox ou referentialBox
/// Similaire à "Rectangle", mais avec le centre du rectangle et les *demis*
/// largeurs/hauteurs. (plus pratique pour savoir si à l'intérieur...)
/// Sert aussi pour "entrer" ou "sortir" d'un référentiel. 
/// (Voir `vector2_referentialIn` et `vector2_referentialOut`.)
typedef union {
  float f_arr[4];
  struct {
    float c_x, c_y, Dx, Dy;
  };
  struct {
    Vector2 center;
    union {
        Vector2 deltas;  // En tant que hitbox.
        Vector2 scales;  // En tant que referentialBox.
    };
  };
} Box;

//static inline Box box_hitBoxFromReferential(Box referential, Vector2 sizes);
/// Vérifier si un point tombe dans une hitbox.
static inline bool vector2_isInBox(Vector2 pos, Box hitBox);

// Référentiel "identité" (pas de translation et scaling neutre).
#define box_identity (Box) {{ 0, 0, 1, 1 }}
/// On place la position `xy` dans le référentiel `referential`.
static inline Vector2 vector2_referentialIn(Vector2 xy, Box referential);
/// Resort du référentiel de la boîte.
static inline Vector2 vector2_referentialOut(Vector2 xy, Box referential);
/// On place `b` dans le référentiel `referential`.
static inline Box box_referentialIn(Box b, Box referential);
/// Resort `b` du `referential`.
static inline Box box_referentialOut(Box b, Box referential);

// MARK: - Marges (autour d une view)
/// Marges en pixels.
typedef struct {
  double top;
  double left;
  double bottom;
  double right;
} Margins;

// MARK: - Vector3
typedef union {
  float f_arr[3];
  struct {
    union {
        Vector2 xy;
        struct { float x, y; };
    };
    float z;
  };
  struct {
    float r, g, b;
  };
} Vector3;

#define vector3_ones  (Vector3) {{ 1, 1, 1 }}
#define vector3_zeros (Vector3) {{ 0, 0, 0 }}

static inline float   vector3_norm(Vector3 v);
static inline Vector3 vector3_minus(Vector3 v, Vector3 toSubtract);
static inline Vector3 vector3_add(Vector3 v, Vector3 toAdd);
static inline Vector3 vector3_times(Vector3 v, float f);
static inline Vector3 vector3_cross(Vector3 v1, Vector3 v2);
static inline float   vector3_dot(Vector3 v1, Vector3 v2);
static inline Vector3 vector3_product(Vector3 v1, Vector3 v2);
static inline Vector3 vector3_alphaBlend(Vector3 v0, Vector3 v1, float alpha);
static inline Vector3 vector3_max(Vector3 v1, Vector3 v2);
static inline Vector3 vector3_values(float f);

Vector3 vector3_normalize(Vector3 v);
/// Pour l'affichage, retourne un shared `const char*` (ne peut donc qu'afficher un vecteur à la fois).
const char* vector3_toString(Vector3 v);

// MARK: - Vector4 : struct de 4 floats alignés en mémoire.
// Pour les 4 floats, on utilise le simd, i.e. `__attribute__((aligned(16)))`.
// (SIMD : Single instruction multiple data.)
// └> Les Vector4 sont donc dans un espace "atomic" (pas de conflits ecriture/lecture entre threads),
//    et sont alloc sur un espace mémoire multiple de 16 octets, e.g. 0x3fd782a0 (finit avec 0).
// On peut initialiser un Vector4 avec une opération simd via le `.v` :
// `Vector4 const a = (Vector4) { .v = vect1.v + vect2.v, };`
typedef __attribute__((aligned(16))) union {
  /// En tant qu'array ordinaire de float. Permet un init du type `Vector4 a = {{ 1, 2, 3, 4, }};`.
  float f_arr[4];
  /// En tant qu'objet `__attribute__((vector_size(16)))` float,
  /// i.e. sorte de "super" float où on peut faire les operation arithmetiques
  /// sur 4 floats en même temps, e.g. v3.v = v1.v + v2.v;
  /// (L'optimisation fait en sorte d'utiliser le SIMD du processeur.)
  /// Noter qu'un vecteur simd de 128 bits est "atomic", du moins selon des expériences... ;)
  float __attribute__((vector_size(16))) v;
  // Position + w.
  struct {
    float x, y, z, w;
  };
  // En tant que couleur. 
  struct {
    float r, g, b, a;
  };
  // Seulement les 3 premières composantes.
  struct { Vector3 xyz; };
  // Seulement les 2 premières composantes.
  struct { Vector2 xy; };
} Vector4;
// Pour afficher (besoin ?) ça serait genre
// #pragma clang diagnostic ignored "-Wformat-invalid-specifier"
// printf("My vector4 is %v4f.\n", myvec.v);

#define vector4_ones  (Vector4){{ 1, 1, 1, 1 }}
#define vector4_zeros (Vector4){{ 0, 0, 0, 0 }}

// Pour Vector4, puisqu'on utilise simd, la plupart des opérations courantes n'ont pas besoin d'être définie...
// e.g.: `a.v = b.v + c.v;`.
static inline Vector4 vector4_max(Vector4 v1, Vector4 v2);
static inline Vector4 vector4_min(Vector4 v1, Vector4 v2);
/// Faire un Vector4 avec Vector3 + float, i.e. v4 = { v3, e3 }.
static inline Vector4 vector4_cat(Vector3 v, float e3);

// MARK: - 128 bits de data quelconques. 128 bits semble le max pour atomic, 
// i.e. pas de conflits lecture/écriture dans les threads.
/// 128 bits de data. Aligné via le Vector4.v `__attribute__((vector_size(16)))`. 
typedef union {
    Vector4 v;      // 4 floats (de 32 bits)
    char c_str[16]; // 16 chars (de 8 bits)
    struct { uint32_t u0, u1, u2, u3; };  // 4 uint ou int (de 32 bits)
    struct { UintPair u01,    u23; }; 
    struct { int32_t  i0, i1, i2, i3; };
    struct { void    *ptr0,  *ptr1; };    // 2 pointeurs de 64 bits.
} Data128;

// MARK: -- Matrices 4x4 ------------------------------------
/// Un peu d'algèbre linéaire...
/// Les vecteurs v0, v1, v2, v3 sont les *colonnes* dans la convention standard :
/// A x = b
/// [v0 v1 v2 v3] [x] = [b].
/// Par exemple, pour la translation, c'est v3 qui est le vecteur de translation 
/// (multiplié avec x3 (ou w) == 1 pour un point).
/// Chaque colonne vi est la transformation d'un vecteur de base,
/// i.e. (1, 0, 0, 0) multiplié par la matrice A devient v0. 
typedef __attribute__((aligned(16))) union {
  float f_arr[16]; // En premier pour init des valeurs.
  float __attribute__((vector_size(16))) v_arr[4];
  struct {
    Vector4 v0, v1, v2, v3;
  };
} Matrix4;

/// Matrice avec des 1 sur la diagonale.
extern const Matrix4 matrix4_identity;

// Inits
void matrix4_initWithAndTranslate(Matrix4 *m, const Matrix4 *ref, Vector3 t);
void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, const Matrix4 *ref,
                                           float thetaY, float ty, float tz);
void matrix4_initAsLookAt(Matrix4 *m, Vector3 eye, Vector3 center, Vector3 up);
static inline void matrix4_initAsPerspective(Matrix4 *m, float theta, float ratio,
                               float nearZ, float farZ);
static inline void matrix4_initAsPerspectiveDeltas(Matrix4 *m, float nearZ, float farZ,
                                     float middleZ, float deltaX, float deltaY);
/// Init typique pour la matrice de la root : Une projection ordinaire et un point de vue.
/// M = P x V. (projection x view)
/// deltaX/deltaY sont par rapport à où on regarde, i.e. `center` (qui est typiqument (0, 0, 0)).
void matrix4_initAsPerspectiveAndLookAt(Matrix4* m,
        Vector3 eye, Vector3 center, Vector3 up,
        float nearZ, float farZ, float deltaX, float deltaY);
                                     
// MARK: - Opérations sur les matrices --
// ** Pour l'instant toute les opérations sont des produits
// s'ajoutant à DROITE de la matrice de modèle (donc "prior"). **
// Car on commence pas les noeuds racines pour l'affichage...
// (pas encore eu besoin d'opération "post" ou à gauche.)

/// Produit matriciel ordinaire `A*B`. Pratique, pour tester des combinaisons, mais a priori lourd pour rien.
/// (Si le produit est souvent utilisé, faire une méthode particulière probablement moins lourde,
/// e.g. `matrix4_rotateYandTranslateYZ`.)
static inline Matrix4 matrix4_product(const Matrix4* a, const Matrix4* b);
/// Scaling -> M = M x S (ajout à droite)
/// (Le scaling est simplement une diagonale avec les scaling en x, y, z.)
/// Un peu superflu, facilement combinable avec d'autres opérations...
static inline void matrix4_scale(Matrix4 *m, Vector3 s);
/// Translate exploite la 4e coordonnée `w` / 4e colonne...
/// (La 4e colonne sera multipliée par le `w` == 1 (typiquement)
/// du vecteur position pour faire une translation.)
static inline void matrix4_translate(Matrix4 *m, Vector3 t);
/// Rotation autour de l'axe x, i.e. permutation y/z.
void matrix4_rotateX(Matrix4 *m, float theta);
void matrix4_rotateY(Matrix4 *m, float theta);
void matrix4_rotateZ(Matrix4 *m, float theta);
void matrix4_rotateYandTranslateYZ(Matrix4 *m, float thetaY, float ty,
                                   float tz);
// Print
void matrix4_print(const Matrix4 *m);

// MARK: - Extensions de rand.
/// Nombre aléatoire (distr. lin.) dans l'interval [min, max] inclusivement.
static inline float rand_floatIn(float min, float max);
/// Nomber aléatoire (distr. lin.) dans l'interval [mean - delta, mean + delta]
/// inclusivement.
static inline float rand_floatAt(float mean, float delta);
/// Nombre aléatoire positif (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement, i.e. un dé.
static inline uint32_t rand_uint(uint32_t min, uint32_t max);
/// Nombre aléatoire entier (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement.
static inline int32_t rand_int(int32_t min, int32_t max);
/// Boolean random. Probabilite "p" d'etre true.
static inline bool rand_bool(float p);
/// Position random dans la "box" (region rectangulaire voir plus haut).
static inline Vector2 rand_vector2_inBox(Box box);

/// Melanger un array de uint
void rand_uintarr_shuffle(uint32_t *u_arr, uint32_t count);
/// "Arrondie" un float en int en gardant la meme valeur moyenne,
/// e.g.  5.75 -> 5 ou 6 avec P[5] = 25% et P[6] = 75%.
int64_t rand_float_toInt(float f);
/// Retourne un vecteur quelconque de rayon norm (de direction random).
Vector2 rand_vector2_ofNorm(float norm);

/// Init un array de uint avec une fonction hash. (liste de nombre plus ou moins random)
void uintarr_initHashed(uint32_t *u_arr, size_t count, uint32_t seed);


// MARK: - Bool Extension
static inline char const* bool_toString(bool b);
static inline void bool_initConst(const bool* b, bool initValue);

// MARK: - Unsigned Extensions
// max, min : En faire des define ?
static inline uint32_t umaxu(uint32_t const a, uint32_t const b);
static inline uint32_t uminu(uint32_t const a, uint32_t const b);
/// Vérifie si `a` est entre min et max, i.e. équivalent de umaxu(min, uminu(max, a)). 
static inline uint32_t uint_clamp(uint32_t const u, uint32_t const min, uint32_t const max);
/// Cast away the const pour init un unsigned const.
static inline void  uint_initConst(const uint32_t* const u, uint32_t const initValue);
static inline void  size_initConst(const size_t* const s, size_t const initValue);
static inline void  voidptr_initConst(const void**const ptr, void*const init_ptr);

uint32_t uint_highestDecimal(uint32_t u);
uint32_t uint_digitAt(uint32_t u, uint32_t decimal);

void uintarr_linspace(uint32_t *u_arr, uint32_t u_arr_count, uint32_t first,
                      uint32_t delta);
void uintarr_print(const uint32_t *u_arr, uint32_t u_arr_count);

// MARK: - Extension de int
static inline int32_t imini(int32_t a, int32_t b);
static inline int32_t imaxi(int32_t a, int32_t b);
static inline int32_t int_clamp(int32_t i, int32_t min, int32_t max);

// MARK: - Extensions de float, fonction pratique sur les floats...
/// Retourne une angle dans l'interval ]-pi, pi].
static inline float float_toNormalizedAngle(float const f);
/// Fonction exponentielle decroissante avec pente de zéro au début, i.e. cas
/// particulier de l'amortissementd critique. lambda * (x + 1/lambda) *
/// exp(-lambda*x).
/// (A priori valide pour x >= 0)
static inline float float_smoothOut(float const x, float lambda);
/// Cast away the const pour init un float const.
static inline void  float_initConst(float const* f, float initValue);
/// Vérifie si `a` est entre min et max, i.e. équivalent de fmaxf(min, fminf(max, a)). 
static inline float float_clamp(float a, float min, float max);
/// Fonction droite "coupé", "en S", i.e.    __/
///                                         /
static inline float float_truncated(float f, float delta);
/// Variante de fonction sigma. ____/‾‾‾‾
/// Commence à x0 et fini à x0 + deltaX.
static inline float float_appearing(float x, float x0, float deltaX);
/// Interpolation entre f0 et f1 (alpha = 0 -> f0, alpha = 1 -> f1).
static inline float float_alphaBlend(float f0, float f1, float alpha);

/** Retourne la plus grosse "subdivision" pour le nombre présent en base 10.
 * Le premier chiffre peut être : 1, 2 ou 5. Utile pour les axes de graphiques.
 * e.g.: 792 -> 500, 192 -> 100, 385 -> 200. */
float float_toRoundedSubDiv(float f);

// MARK: - Flags de placement relatif. e.g. à droite, gauche, centré...
enum {
    relative_center =      0x00000,  // Centré par défaut.
    relative_left =        0x00100,  // Placé sur le côté gauche du parent/cadre.
    relative_right =       0x00200,
    relative_top =         0x00400,
    relative_bottom =      0x00800,
    relative_leftAlign =   0x01000,  // Côté gauche du contenu est aligné (sur le centre par défaut).
    relative_rightAlign =  0x02000,  // (Si on veut Left aligned classique, c'est `relative_left|relative_leftAlign`.)
    relative_topAlign =    0x04000,
    relative_bottomAlign = 0x08000,
    // Combinaisons utiles.
    relatives_top = relative_top | relative_topAlign,
    relatives_bottom = relative_bottom | relative_bottomAlign,
    relatives_left = relative_left | relative_leftAlign,
    relatives_right = relative_right | relative_rightAlign,
    relatives_topRight = relative_top | relative_topAlign |
                       relative_right | relative_rightAlign,
    relatives_topLeft = relative_left | relative_leftAlign |
                      relative_top | relative_topAlign,
    relatives_bottomRight = relative_right | relative_rightAlign |
                          relative_bottom | relative_bottomAlign,
    relatives_bottomLeft = relative_left | relative_leftAlign |
                         relative_bottom | relative_bottomAlign,
};

// MARK: - Les "digits" (chiffres) retrouvés dans les textures de digits.
typedef enum CoqDigit {
    digit_zero,
    digit_one,
    digit_two,
    digit_three,
    digit_four,
    digit_five,
    digit_six,
    digit_seven,
    digit_eight,
    digit_nine,
    digit_space,
    digit_unused1,
    digit_underscore,
    digit_plus,
    digit_minus,
    digit_mult,
    digit_div,
    digit_dot,
    digit_comma,
    digit_second,
    digit_percent,
    digit_equal,
    digit_question,
    digit_unused2,
} Digit;

// Définitions des inlines
#include "math_base.inl"

#endif /* maths_h */
