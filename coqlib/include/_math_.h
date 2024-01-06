//
//  _math_.h
//  Struct et fonction mathématique pratiques.
//
//  Created by Corentin Faucher on 2023-10-12.
//
#ifndef _coq_math_h
#define _coq_math_h

#include "_utils_.h"
#include <math.h>
#include <simd/simd.h>

// Si on veut l'allignement, utiliser
// struct __attribute__((aligned(16))) Vector { union {...};};
// Simd ou just __attribute__((aligned(16))) ??

/*-- Vector2 -------------------------------------------------*/
/// Struct de 2 float. (Non aligned)
/// On met le float array en premier pour une init plus simple. e.g. Vector2 v = {1,2}.
typedef union {
    float         f_arr[2];
    struct {float x, y;};
    struct {float w, h;};
//    simd_float2    v;  // Superflu ? (alignerait...)
} Vector2;
extern const Vector2 vector2_ones;
extern const Vector2 vector2_zeros;
// Operations usuelles (similaire à celles de simd...)
float   vector2_norm(Vector2 v);
float   vector2_dot(Vector2 v1, Vector2 v2);
float   vector2_distance(Vector2 v1, Vector2 v2);
Vector2 vector2_normalize(Vector2 v);
Vector2 vector2_minus(Vector2 v, Vector2 toSubtract);
Vector2 vector2_add(Vector2 v, Vector2 toAdd);
Vector2 vector2_times(Vector2 v, float f);
/// Produit vectoriel avec vec k (i.e. Rotation de 90deg dans le sens horaire).
Vector2 vector2_cross(Vector2 v);


/*-- Rectangle -------------------------------------------------*/
/// Structure "Rectangle" pour les zone de NSView, view SDL, etc.
/// L'origine peut être le coin *supérieur* gauche (y inversé)
/// OU le coin *inférieur* gauche  (y normal) dépendant de l'OS/environnement...
typedef union {
    float          f_arr[4];
    struct { float o_x, o_y, w, h; };
    struct {
        Vector2    origin;
        Vector2    size;
    };
} Rectangle;

/*-- Box (hitbox) -------------------------------------------------*/
/// Similaire à "Rectangle", mais avec le centre du rectangle et les *demis* largeurs/hauteurs.
/// (plus pratique pour savoir si à l'intérieur...)
typedef union {
    float          f_arr[4];
    struct { float c_x, c_y, Dx, Dy; };
    struct {
        Vector2    center;
        Vector2    deltas;
    };
} Box;

/*-- Marges (autour d'une view) -------------------------------------*/
/// Marges en pixels.
typedef struct {
    double top;
    double left;
    double bottom;
    double right;
} Margins;

/*-- Vector3 -------------------------------------------------*/
typedef union {
    float          f_arr[3];
    simd_float3    v;
    struct { float x, y, z; };
    struct { float r, g, b; };
} Vector3;
//typedef simd_float3 Vector3;
extern const Vector3 vector3_ones;
extern const Vector3 vector3_zeros;

void vector3_print(Vector3* v);


/*-- Vecteur de 4 floats. -------------------------------*/
// ici, aligned semble superflu. simd_float3 avec __attribute__((__ext_vector_type__(4)))
// fait la job.
typedef // __attribute__((__aligned__(16)))
union {
    /// En tant qu'array ordinaire de float, i.e. f_arr est un float*.
    float          f_arr[4];
    /// En tant qu'objet __attribute__((__ext_vector_type__(4))) float,
    /// i.e. sorte de "super" float où on peut faire les operation arithmetiques,
    /// e.g. v1 + v2, sur les 4 floats en même temps.
    simd_float4    v;
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
} Vector4;
// Pour afficher (besoin ?) ça serait genre
// #pragma clang diagnostic ignored "-Wformat-invalid-specifier"
// printf("My vector4 is %v4f.\n", myvec.v);


/*-- Matrices 4x4 --------------------------------------*/
typedef __attribute__((aligned(16)))union {
    float            m[16];  // En premier pour init des valeurs.
    struct { Vector4 v0, v1, v2, v3; };
} Matrix4;
//simd_float4x4 n'est pas pratique pour les init...
//typedef simd_float4x4 Matrix4;

/// Matrice avec des 1 sur la diagonale.
extern const Matrix4 matrix4_identity;
/*-- Fonction de manipulation des matrices... --*/
// Inits
void matrix4_initWithAndTranslate(Matrix4 *m, const Matrix4* ref, Vector3 t);
void matrix4_initWithRotateYAndTranslateYZ(Matrix4 *m, Matrix4 *ref,
                                           float thetaY, float ty, float tz);
void matrix4_initAsLookAt(Matrix4 *m, Vector3 eye, Vector3 center, Vector3 up);
void matrix4_initAsPerspective(Matrix4 *m, float theta, float ratio,
                               float nearZ, float farZ);
void matrix4_initAsPerspectiveDeltas(Matrix4 *m, float nearZ, float farZ, float middleZ,
                                     float deltaX, float deltaY);
// Operations
// Pour l'instant toute les opérations sont des produits
// s'ajoutant à DROITE de la matrice de modèle (donc "prior").
// Car on commence pas les noeuds racines pour l'affichage...
// (pas encore eu besoin d'opération "post" ou à gauche.)
void matrix4_scale(Matrix4 *m, float sx, float sy, float sz);
void matrix4_translate(Matrix4 *m, Vector3 t);
void matrix4_rotateX(Matrix4 *m, float theta);
void matrix4_rotateY(Matrix4 *m, float theta);
void matrix4_rotateZ(Matrix4 *m, float theta);
void matrix4_rotateYandTranslateYZ(Matrix4 *m, float thetaY, float ty, float tz);
// Print
void matrix4_print(Matrix4 *m);


/*-- Extensions de rand. -------------------------------*/

/// Nombre aléatoire (distr. lin.) dans l'interval [min, max] inclusivement.
float rand_floatIn(float min, float max);
/// Nomber aléatoire (distr. lin.) dans l'interval [mean - delta, mean + delta] inclusivement.
float rand_floatAt(float mean, float delta);
/// Nombre aléatoire positif (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement, i.e. un dé.
uint32_t rand_uint(uint32_t min, uint32_t max);
/// Nombre aléatoire entier (distr. lin. discrete)
/// dans l'interval [min, max] inclusivement.
int32_t rand_int(int32_t min, int32_t max);
/// Melanger un array de uint
void  rand_uintarr_shuffle(uint32_t* u_arr, uint32_t count);
/// Boolean random. Probabilite "p" d'etre true.
bool  rand_bool(float p);
/// "Arrondie" un float en int en gardant la meme valeur moyenne,
/// e.g.  5.75 -> 5 ou 6 avec P[5] = 25% et P[6] = 75%.
int64_t rand_float_toInt(float f);
/// Retourne un vecteur quelconque de rayon norm (de direction random).
Vector2 rand_vector2_ofNorm(float norm);
///// Position random dans la "box" (region rectangulaire voir plus haut).
Vector2 rand_vector2_inBox(Box box);


/*-- Extensions de uint. -------------------------------*/

uint32_t  uint_highestDecimal(uint32_t u);
uint32_t  uint_digitAt(uint32_t u, uint32_t decimal);
uint32_t  umaxu(uint32_t a, uint32_t b);
uint32_t  uminu(uint32_t a, uint32_t b);
void   uintarr_linspace(uint32_t* u_arr, uint32_t start,
                       uint32_t delta, uint32_t count);
void   uintarr_print(uint32_t* u_arr, uint32_t count);


/*-- Extensions de float. -------------------------------*/

/// Retourne une angle dans l'interval [-pi, pi].
float float_toNormalizedAngle(float f);
/** Retourne la plus grosse "subdivision" pour le nombre présent en base 10.
 * Le premier chiffre peut être : 1, 2 ou 5. Utile pour les axes de graphiques.
 * e.g.: 792 -> 500, 192 -> 100, 385 -> 200. */
float float_toRoundedSubDiv(float f);
/// Fonction droite "coupé", "en S", i.e.    __/
///                                         /
float float_truncated(float f, float delta);

#endif /* maths_h */
