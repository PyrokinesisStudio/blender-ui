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
 * Contributor(s): Joseph Eagar
 *
 * ***** END GPL LICENSE BLOCK *****
 */
 
#ifndef __BKE_IDPROP_H__
#define __BKE_IDPROP_H__

/** \file BKE_idprop.h
 *  \ingroup bke
 *  \author Joseph Eagar
 */

#include "DNA_ID.h"

#include "BLI_compiler_attrs.h"

struct IDProperty;
struct ID;

typedef union IDPropertyTemplate {
	int i;
	float f;
	double d;
	struct {
		char *str;
		short len;
		char subtype;
	} string;
	struct ID *id;
	struct {
		short type;
		short len;
	} array;
	struct {
		int matvec_size;
		float *example;
	} matrix_or_vector;
} IDPropertyTemplate;

/* ----------- Property Array Type ---------- */

IDProperty *IDP_NewIDPArray(const char *name) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
IDProperty *IDP_CopyIDPArray(IDProperty *array) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

void IDP_FreeIDPArray(IDProperty *prop);

/* shallow copies item */
void IDP_SetIndexArray(struct IDProperty *prop, int index, struct IDProperty *item) ATTR_NONNULL();
struct IDProperty *IDP_GetIndexArray(struct IDProperty *prop, int index) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
void IDP_AppendArray(struct IDProperty *prop, struct IDProperty *item);
void IDP_ResizeIDPArray(struct IDProperty *prop, int len);

/* ----------- Numeric Array Type ----------- */
/*this function works for strings too!*/
void IDP_ResizeArray(struct IDProperty *prop, int newlen);
void IDP_FreeArray(struct IDProperty *prop);

/* ---------- String Type ------------ */
IDProperty *IDP_NewString(const char *st, const char *name, int maxlen) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(2 /* 'name 'arg */); /* maxlen excludes '\0' */
void IDP_AssignString(struct IDProperty *prop, const char *st, int maxlen) ATTR_NONNULL(); /* maxlen excludes '\0' */
void IDP_ConcatStringC(struct IDProperty *prop, const char *st) ATTR_NONNULL();
void IDP_ConcatString(struct IDProperty *str1, struct IDProperty *append) ATTR_NONNULL();
void IDP_FreeString(struct IDProperty *prop) ATTR_NONNULL();

/*-------- ID Type -------*/
void IDP_LinkID(struct IDProperty *prop, ID *id);
void IDP_UnlinkID(struct IDProperty *prop);

/*-------- Group Functions -------*/

/** Sync values from one group to another, only where they match */
void IDP_SyncGroupValues(struct IDProperty *dest, struct IDProperty *src) ATTR_NONNULL();
void IDP_ReplaceGroupInGroup(struct IDProperty *dest, struct IDProperty *src) ATTR_NONNULL();
void IDP_ReplaceInGroup(struct IDProperty *group, struct IDProperty *prop) ATTR_NONNULL();
void IDP_MergeGroup(IDProperty *dest, IDProperty *src, const int do_overwrite) ATTR_NONNULL();
int IDP_AddToGroup(struct IDProperty *group, struct IDProperty *prop) ATTR_NONNULL();
int IDP_InsertToGroup(struct IDProperty *group, struct IDProperty *previous, 
                      struct IDProperty *pnew) ATTR_NONNULL(1 /* group */, 3 /* pnew */);
void IDP_RemFromGroup(struct IDProperty *group, struct IDProperty *prop) ATTR_NONNULL();

IDProperty *IDP_GetPropertyFromGroup(struct IDProperty *prop, const char *name) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
IDProperty *IDP_GetPropertyTypeFromGroup(struct IDProperty *prop, const char *name, const char type) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
void *IDP_GetGroupIterator(struct IDProperty *prop) ATTR_WARN_UNUSED_RESULT;
IDProperty *IDP_GroupIterNext(void *vself) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
void IDP_FreeIterBeforeEnd(void *vself) ATTR_NONNULL();

/*-------- Main Functions --------*/
struct IDProperty *IDP_GetProperties(struct ID *id, int create_if_needed) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
struct IDProperty *IDP_CopyProperty(struct IDProperty *prop) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

int IDP_EqualsProperties_ex(IDProperty *prop1, IDProperty *prop2, const int is_strict) ATTR_WARN_UNUSED_RESULT;

int IDP_EqualsProperties(struct IDProperty *prop1, struct IDProperty *prop2) ATTR_WARN_UNUSED_RESULT;

struct IDProperty *IDP_New(const int type, const IDPropertyTemplate *val, const char *name) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

void IDP_FreeProperty(struct IDProperty *prop);

void IDP_ClearProperty(IDProperty *prop);

void IDP_UnlinkProperty(struct IDProperty *prop);

#define IDP_Int(prop)                     ((prop)->data.val)
#define IDP_Float(prop)        (*(float *)&(prop)->data.val)
#define IDP_Double(prop)      (*(double *)&(prop)->data.val)
#define IDP_String(prop)         ((char *) (prop)->data.pointer)
#define IDP_Array(prop)                   ((prop)->data.pointer)
#define IDP_IDPArray(prop) ((IDProperty *) (prop)->data.pointer)

#ifdef DEBUG
/* for printout only */
void IDP_spit(IDProperty *prop);
#endif

#endif /* __BKE_IDPROP_H__ */
