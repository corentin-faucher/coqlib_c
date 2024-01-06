//
//  drawable_curve.c
//  AnimalCounting
//
//  Created by Corentin Faucher on 2023-12-22.
//

#include "_nodes/_node_drawable_curve.h"




Drawable* Drawable_createCurve(Node* refOpt, Texture* tex, Vector2* v_arr, uint32_t v_count, float delta) {
/*
    uint32_t index = 0;
    // Les deux points du segment et le next.
    while(index <) {}
    Vector2 v0=v_arr[index], v1=v_arr[index+1], v2=v_arr[index+2];
    // Quatre coin d'un segment a calculer.
    Vector2 v0u, v0d, v1u, v1dl, v1dr, v1ul, v1ur;
    Vector2 v01 = vector2_minus(v1, v0);
    Vector2 v12 = vector2_minus(v2, v1);
    if(index != 0) {
        //  v0u, v0d sont récupére des calculs du segment précédent...
        
    } else {
        v0d = vector2_cross(v01);
        float f0d = vector2_norm(v0d);
        v0d = vector2_times(v0d, delta / f0d);
        v0u = vector2_times(v0d, -1.f);
    }
    // Calcul des perpendiculaires up and down.
    float f01 = vector2_norm(v01);
    float f12 = vector2_norm(v12);
    Vector2 v01d = vector2_times(vector2_cross(v01), delta / f01);
    Vector2 v12d = vector2_times(vector2_cross(v12), delta / f12);
    Vector2 v01u = vector2_times(v01d, -1.f);
    Vector2 v12u = vector2_times(v12d, -1.f);
    // Calcul des point à droite.
    // Courbe vers le bas ?
    Vector2 vEps = vector2_minus(v12d, v01d);
    if(vector2_dot(vEps, v01) < 0.f) {
        v1dl = vector2_add(v01d, vector2_times(v01, vector2_dot(v12d, v01) / (f01*f01)));
        v1dr = vector2_add(v12d, vector2_times(v12, vector2_dot(v01d, v12) / (f12*f12)));
        v1ul = v01u;
        v1ur = v12u;
    } else {
        v1dl = v01d;
        v1dr = v12d;
        v1ul = vector2_add(v01u, vector2_times(v01, vector2_dot(v12u, v01) / (f01*f01)));
        v1ur = vector2_add(v12u, vector2_times(v12, vector2_dot(v01u, v12) / (f12*f12)));
    }
    
    
    
    
    
    Drawable* d = Drawable_create(refOpt, tex, mesh_sprite, flag_drawableDontRespectRatio, 0);
    return d;
 
 */
    return NULL;
}
