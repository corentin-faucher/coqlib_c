//
//  node_drawable.inl
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-17.
//
/// Met à jour le uvRect pour être la tile (i,j) de la texture présente (avec m x n subdivisions).
static inline void drawable_setTile(Drawable *const d, uint32_t const i, uint32_t const j) {
    d->n.renderIU.uvRect = texturedims_uvRectOfTile(d->texr.dims, i, j);
}
/// Ne fait que mettre à jour `uvRect.o_x` (ne vérifie pas le reste du uvRect).
static inline void drawable_setTileI(Drawable *const d, uint32_t const i) {
    d->n.renderIU.uvRect.o_x = (float)( i % d->texr.dims.m) * d->texr.dims.Du;
}
/// Ne fait que mettre à jour `uvRect.o_y` (ne vérifie pas le reste du uvRect).
static inline void drawable_setTileJ(Drawable *const d, uint32_t const j) {
    d->n.renderIU.uvRect.o_y = (float)( j % d->texr.dims.n) * d->texr.dims.Dv;
}
/// Set le rectangle uv. (on peut aussi le setter directement...)
static inline void drawable_setUVRect(Drawable *const d, Rectangle const uvrect) {
    d->n.renderIU.uvRect = uvrect;
}
