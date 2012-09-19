/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */
#ifndef __BKE_KEY_H__
#define __BKE_KEY_H__

/** \file BKE_key.h
 *  \ingroup bke
 *  \since March 2001
 *  \author nzc
 */
struct Key;
struct KeyBlock;
struct ID;
struct ListBase;
struct Curve;
struct Object;
struct Scene;
struct Lattice;
struct Mesh;

/* Kernel prototypes */
#ifdef __cplusplus
extern "C" {
#endif

void        BKE_key_free(struct Key *sc);
void        BKE_key_free_nolib(struct Key *key);
struct Key *BKE_key_add(struct ID *id);
struct Key *BKE_key_copy(struct Key *key);
struct Key *BKE_key_copy_nolib(struct Key *key);
void        BKE_key_make_local(struct Key *key);
void        BKE_key_sort(struct Key *key);

void key_curve_position_weights(float t, float data[4], int type);
void key_curve_tangent_weights(float t, float data[4], int type);
void key_curve_normal_weights(float t, float data[4], int type);

float *do_ob_key(struct Scene *scene, struct Object *ob);

struct Key      *BKE_key_from_object(struct Object *ob);
struct KeyBlock *BKE_keyblock_from_object(struct Object *ob);
struct KeyBlock *BKE_keyblock_from_object_reference(struct Object *ob);

struct KeyBlock *BKE_keyblock_add(struct Key *key, const char *name);
struct KeyBlock *BKE_keyblock_add_ctime(struct Key *key, const char *name, const short do_force);
struct KeyBlock *BKE_keyblock_from_key(struct Key *key, int index);
struct KeyBlock *BKE_keyblock_find_name(struct Key *key, const char name[]);
void             BKE_keyblock_copy_settings(struct KeyBlock *kb_dst, const struct KeyBlock *kb_src);
char            *BKE_keyblock_curval_rnapath_get(struct Key *key, struct KeyBlock *kb);
// needed for the GE
void BKE_key_evaluate_relative(const int start, int end, const int tot, char *basispoin, struct Key *key, struct KeyBlock *actkb, const int mode);

/* conversion functions */
void    BKE_key_convert_to_mesh(struct KeyBlock *kb, struct Mesh *me);
void    BKE_key_convert_from_mesh(struct Mesh *me, struct KeyBlock *kb);
void    BKE_key_convert_to_lattice(struct KeyBlock *kb, struct Lattice *lt);
void    BKE_key_convert_from_lattice(struct Lattice *lt, struct KeyBlock *kb);
void    BKE_key_convert_to_curve(struct KeyBlock *kb, struct Curve  *cu, struct ListBase *nurb);
void    BKE_key_convert_from_curve(struct Curve *cu, struct KeyBlock *kb, struct ListBase *nurb);
float (*BKE_key_convert_to_vertcos(struct Object *ob, struct KeyBlock *kb))[3];
void    BKE_key_convert_from_vertcos(struct Object *ob, struct KeyBlock *kb, float (*vertCos)[3]);
void    BKE_key_convert_from_offset(struct Object *ob, struct KeyBlock *kb, float (*ofs)[3]);

/* key.c */
extern int slurph_opt;

#ifdef __cplusplus
};
#endif

#endif // __BKE_KEY_H__
