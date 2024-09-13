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

// Si on veut l'allignement, utiliser
// struct __attribute__((aligned(16))) Vector { union {...};};
// Simd ou just __attribute__((aligned(16))) ??


#pragma mark-- Vector2 -------------------------------------------------

/// Paire de 2 float. (Non aligned)
/// Pour une paire de unsigned, voir UintPair plus bas.
/// On met le float array en premier pour une init plus simple. e.g. Vector2 v =
/// {1,2}.
typedef union {
  float f_arr[2];
  struct {
    float x, y;  // Version position.
  };
  struct {
    float w, h;  // Version tailles.
  };
  struct {
    float theta, radial; // Coordonées polaires (voir conversion `vector2_polarToCartesian` et `vector2_cartesianToPolar`).
  };
  // Version alignée superflu ?
  //    float __attribute__((vector_size(8))) v;
  //    simd_float2    v;
} Vector2;
extern const Vector2 vector2_ones;
extern const Vector2 vector2_zeros;
// Operations usuelles (similaire à celles de simd...)
float vector2_norm(Vector2 v);
float vector2_norm2(Vector2 v);
/// Projection de v sur target, i.e. proj = (v • t) * t / ||t||^2.
Vector2 vector2_projOn(Vector2 v, Vector2 target);
float vector2_distance(Vector2 v1, Vector2 v2);
/// Normalise le vecteur (longueur == 1).
/// Si ~= 0 -> retourne un vecteur unitaire random (voir `rand_vector2_ofNorm,`.)
Vector2 vector2_normalize(Vector2 v);
Vector2 vector2_minus(Vector2 v, Vector2 toSubtract);
Vector2 vector2_add(Vector2 v, Vector2 toAdd);
Vector2 vector2_mean(Vector2 a, Vector2 b);
Vector2 vector2_opposite(Vector2 v);
// Il y a 4 sortes de produits avec les vecteurs :
//  - avec scalaire : _times ;
//  - membre à membre : _product ;
//  - produit scalaire : _dot ;
//  - produit vectoriel : _cross.
Vector2 vector2_times(Vector2 v, float f);
/// Produit vectoriel avec vec k (i.e. Rotation de 90deg dans le sens horaire).
Vector2 vector2_cross(Vector2 v);
/// Produit scalaire.
float   vector2_dot(Vector2 v1, Vector2 v2);
/// Produit membre à membre.
Vector2 vector2_product(Vector2 v1, Vector2 v2);

Vector2 vector2_polarToCartesian(Vector2 v);
Vector2 vector2_cartesianToPolar(Vector2 v);

/// Pour l'affichage en debug.
const char* vector2_toString(Vector2 const v); 


#pragma mark - Paire de unsigned  --------------------------------------

/// Paire de unsigned.
typedef struct UintPair {
  uint32_t uint0, uint1;
} UintPair;


#pragma mark - Rectangle -----------------------------------------------

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


#pragma mark - Box (hitbox) --------------------------------------------

/// Similaire à "Rectangle", mais avec le centre du rectangle et les *demis*
/// largeurs/hauteurs. (plus pratique pour savoir si à l'intérieur... Box peut
/// etre caster comme Rectangle et vice-versa, ça reste des arrays de 4
/// floats...)
typedef union {
  float f_arr[4];
  struct {
    float c_x, c_y, Dx, Dy;
  };
  struct {
    Vector2 center;
    Vector2 deltas;
  };
} Box;


#pragma mark - Marges (autour d une view) ------------------------------

/// Marges en pixels.
typedef struct {
  double top;
  double left;
  double bottom;
  double right;
} Margins;


#pragma mark - Vector3 -------------------------------------------------

typedef union {
  float f_arr[3];
  struct {
    float x, y, z;
  };
  struct {
    float r, g, b;
  };
} Vector3;
extern const Vector3 vector3_ones;
extern const Vector3 vector3_zeros;

// Operations usuelles (similaire à celles de simd...)
Vector3 vector3_normalize(Vector3 v);
Vector3 vector3_minus(Vector3 v, Vector3 toSubtract);
Vector3 vector3_add(Vector3 v, Vector3 toAdd);
Vector3 vector3_cross(Vector3 v1, Vector3 v2);
float   vector3_dot(Vector3 v1, Vector3 v2);
/// Produit membre à membre
Vector3 vector3_product(Vector3 v1, Vector3 v2);

/// Pour l'affichage, retourne un shared `const char*` (ne peut donc qu'afficher un vecteur à la fois).
const char* vector3_toString(Vector3 v);


#pragma mark-- Vecteur de 4 floats. ------------------------------------

typedef union {
  /// En tant qu'array ordinaire de float, i.e. f_arr est un float*.
  float f_arr[4];
  /// En tant qu'objet `__attribute__((vector_size(16)))` float,
  /// i.e. sorte de "super" float où on peut faire les operation arithmetiques
  /// sur 4 floats en même temps, e.g. v3 = v1 + v2;
  /// (L'optimisation fait en sorte d'utiliser le SIMD du processeur.)
  float __attribute__((vector_size(16))) v;
  //    simd_float4    v;
  struct {
    float x, y, z, w;
  }; // Position + w.
  struct {
    float r, g, b, a;
  }; // En tant que couleur.
} Vector4;
// Pour afficher (besoin ?) ça serait genre
// #pragma clang diagnostic ignored "-Wformat-invalid-specifier"
// printf("My vector4 is %v4f.\n", myvec.v);

extern const Vector4 vector4_ones;
extern const Vector4 vector4_zeros;

#pragma mark -- Matrices 4x4 ------------------------------------
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
  struct {
    Vector4 v0, v1, v2, v3;
  };
  Vector4 v_arr[4];
} Matrix4;

/// Matrice avec des 1 sur la diagonale.
extern const Matrix4 matrix4_identity;

// Inits
void matrix4_initWithAndTranslate(Matrix4 *m, const Matrix4 *ref, Vector3 t);
void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, const Matrix4 *ref,
                                           float thetaY, float ty, float tz);
void matrix4_initAsLookAt(Matrix4 *m, Vector3 eye, Vector3 center, Vector3 up);
void matrix4_initAsPerspective(Matrix4 *m, float theta, float ratio,
                               float nearZ, float farZ);
void matrix4_initAsPerspectiveDeltas(Matrix4 *m, float nearZ, float farZ,
                                     float middleZ, float twoDeltaX, float twoDeltaY);
/// Init typique pour la matrice de la root : Une projection ordinaire et un point de vue.
/// M = P x V. (projection x view)
void matrix4_initAsPerspectiveAndLookAt(Matrix4* m,
        Vector3 eye, Vector3 center, Vector3 up,
        float nearZ, float farZ, float middleZ, float deltaX, float deltaY);
                                     
#pragma mark - Opérations sur les matrices --
// ** Pour l'instant toute les opérations sont des produits
// s'ajoutant à DROITE de la matrice de modèle (donc "prior"). **
// Car on commence pas les noeuds racines pour l'affichage...
// (pas encore eu besoin d'opération "post" ou à gauche.)

Matrix4 matrix4_product(const Matrix4* a, const Matrix4* b);

/// Produit matriciel à droite, i.e. M = M x R. (pas eu besoin de produit matriciel !)
//void matrix4_timeRight(Matrix4* m, const Matrix4* right);

/// Scaling -> M = M x S (ajout à droite)
/// (Le scaling est simplement une diagonale avec les scaling en x, y, z.)
void matrix4_scale(Matrix4 *m, float sx, float sy, float sz);
/// Translate exploite la 4e coordonnée `w` / 4e colonne...
/// (La 4e colonne sera multipliée par le `w` == 1 (typiquement)
/// du vecteur position pour faire une translation.)
void matrix4_translate(Matrix4 *m, Vector3 t);
/// Rotation autour de l'axe x, i.e. permutation y/z.
void matrix4_rotateX(Matrix4 *m, float theta);
void matrix4_rotateY(Matrix4 *m, float theta);
void matrix4_rotateZ(Matrix4 *m, float theta);
void matrix4_rotateYandTranslateYZ(Matrix4 *m, float thetaY, float ty,
                                   float tz);
// Print
void matrix4_print(const Matrix4 *m);

#pragma mark-- Extensions de rand. -------------------------------*/

/// Nombre aléatoire (distr. lin.) dans l'interval [min, max] inclusivement.
float rand_floatIn(float min, float max);
/// Nomber aléatoire (distr. lin.) dans l'interval [mean - delta, mean + delta]
/// inclusivement.
float rand_floatAt(float mean, float delta);
/// Nombre aléatoire positif (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement, i.e. un dé.
uint32_t rand_uint(uint32_t min, uint32_t max);
/// Nombre aléatoire entier (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement.
int32_t rand_int(int32_t min, int32_t max);
/// Melanger un array de uint
void rand_uintarr_shuffle(uint32_t *u_arr, uint32_t count);
/// Boolean random. Probabilite "p" d'etre true.
bool rand_bool(float p);
/// "Arrondie" un float en int en gardant la meme valeur moyenne,
/// e.g.  5.75 -> 5 ou 6 avec P[5] = 25% et P[6] = 75%.
int64_t rand_float_toInt(float f);
/// Retourne un vecteur quelconque de rayon norm (de direction random).
Vector2 rand_vector2_ofNorm(float norm);
/// Position random dans la "box" (region rectangulaire voir plus haut).
Vector2 rand_vector2_inBox(Box box);

#pragma mark -- Extension de bool --

const char* bool_toString(bool b);

#pragma mark-- Extensions de uint. -------------------------------*/

uint32_t uint_highestDecimal(uint32_t u);
uint32_t uint_digitAt(uint32_t u, uint32_t decimal);
uint32_t umaxu(uint32_t a, uint32_t b);
uint32_t uminu(uint32_t a, uint32_t b);
/// Vérifie si `a` est entre min et max, i.e. équivalent de umaxu(min, uminu(max, a)). 
uint32_t ubetweenu(uint32_t min, uint32_t a, uint32_t max);
void uintarr_linspace(uint32_t *u_arr, uint32_t u_arr_count, uint32_t first,
                      uint32_t delta);
void uintarr_print(const uint32_t *u_arr, uint32_t u_arr_count);

/// Cast away the const pour init un float const.
void  uint_initConst(const uint32_t* f, uint32_t initValue);
void  size_initConst(const size_t* s, size_t initValue);


#pragma mark - Extension de int

int32_t imaxi(int32_t a, int32_t b);

#pragma mark-- Extensions de float. -------------------------------*/

/// Retourne une angle dans l'interval [-pi, pi].
float float_toNormalizedAngle(float f);
/** Retourne la plus grosse "subdivision" pour le nombre présent en base 10.
 * Le premier chiffre peut être : 1, 2 ou 5. Utile pour les axes de graphiques.
 * e.g.: 792 -> 500, 192 -> 100, 385 -> 200. */
float float_toRoundedSubDiv(float f);
/// Fonction droite "coupé", "en S", i.e.    __/
///                                         /
float float_truncated(float f, float delta);

/// Variante de fonction sigma. ____/‾‾‾‾
/// Commance à x0 et fini à x0 + deltaX.
float float_appearing(float x, float x0, float deltaX);

/// Fonction exponentielle decroissante avec pente de zéro au début, i.e. cas
/// particulier de l'amortissementd critique. lambda * (x + 1/lambda) *
/// exp(-lambda*x).
/// (A priori valide pour x >= 0)
float float_smoothOut(float x, float lambda);

/// Cast away the const pour init un float const.
void  float_initConst(const float* f, float initValue);

/// Vérifie si `a` est entre min et max, i.e. équivalent de fmaxf(min, fminf(max, a)). 
float fbetweenf(float min, float a, float max);

#endif /* maths_h */
