//
//  node_base.inl
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-18.
//

static inline Vector2 node_deltas(Node const*const n) {
    return (Vector2) {{ n->w * n->sx / 2.f, n->h * n->sy / 2.f }};
}
static inline float node_deltaX(Node const*const n) {
    return n->w * n->sx / 2.f;
}
static inline float   node_deltaY(Node const*const n) {
    return n->h * n->sy / 2.f;
}
static inline float   node_deltaZ(Node const*const n) {
    return n->d * n->sz / 2.f;
}
static inline Box     node_referential(Node const*const n) {
    return (Box) {
        .center = n->xy,
        .deltas = n->scales.xy,
    };
}
static inline Box    node_hitbox(Node const*const n) {
    return (Box) {
        .center = n->xy,
        .deltas = {{ n->w * n->sx / 2.f, n->h * n->sy / 2.f }},
    };
}
