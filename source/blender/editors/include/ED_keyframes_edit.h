/**
 * $Id:
 *
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
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2008 Blender Foundation.
 * All rights reserved.
 *
 * 
 * Contributor(s): Joshua Leung
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#ifndef ED_KEYFRAMES_EDIT_H
#define ED_KEYFRAMES_EDIT_H

struct Ipo;
struct IpoCurve;
struct BezTriple;
struct Scene;

/* ************************************************ */
/* Common Macros and Defines */

/* --------- BezTriple Selection ------------- */

#define BEZSELECTED(bezt) ((bezt->f2 & SELECT) || (bezt->f1 & SELECT) || (bezt->f3 & SELECT))

#define BEZ_SEL(bezt)		{ (bezt)->f1 |=  SELECT; (bezt)->f2 |=  SELECT; (bezt)->f3 |=  SELECT; }
#define BEZ_DESEL(bezt)		{ (bezt)->f1 &= ~SELECT; (bezt)->f2 &= ~SELECT; (bezt)->f3 &= ~SELECT; }
#define BEZ_INVSEL(bezt)	{ (bezt)->f1 ^=  SELECT; (bezt)->f2 ^=  SELECT; (bezt)->f3 ^=  SELECT; }

/* --------- Tool Flags ------------ */

/* bezt validation */
typedef enum eEditKeyframes_Validate {
	BEZT_OK_FRAME	= 1,
	BEZT_OK_FRAMERANGE,
	BEZT_OK_SELECTED,
	BEZT_OK_VALUE,
} eEditKeyframes_Validate;

/* ------------ */

/* select tools */
typedef enum eEditKeyframes_Select {
	SELECT_REPLACE	=	(1<<0),
	SELECT_ADD		= 	(1<<1),
	SELECT_SUBTRACT	= 	(1<<2),
	SELECT_INVERT	= 	(1<<4),
} eEditKeyframes_Select;

/* snapping tools */
typedef enum eEditKeyframes_Snap {
	SNAP_KEYS_NEARFRAME	= 1,
	SNAP_KEYS_CURFRAME,
	SNAP_KEYS_NEARMARKER,
	SNAP_KEYS_NEARSEC,
} eEditKeyframes_Snap;

/* mirroring tools */
//typedef enum eEditKeyframes_Mirror {
	
//} eEditKeyframes_Mirror;

/* ************************************************ */
/* Editing API */

/* --- Generic Properties for Bezier Edit Tools ----- */

// XXX maybe a union would be more compact?
typedef struct BeztEditData {
	ListBase list;				/* temp list for storing custom list of data to check */
	struct Scene *scene;		/* pointer to current scene - many tools need access to cfra/etc.  */
	void *data;					/* pointer to custom data - not that useful? */
	float f1, f2;				/* storage of times/values as 'decimals' */
	int i1, i2;					/* storage of times/values as 'whole' numbers */
} BeztEditData;

/* ------- Function Pointer Typedefs --------------- */

	/* callback function that refreshes the IPO curve after use */
typedef void (*IcuEditFunc)(struct IpoCurve *icu);
	/* callback function that operates on the given BezTriple */
typedef short (*BeztEditFunc)(BeztEditData *bed, struct BezTriple *bezt);

/* ------------- Looping API ------------------- */

short icu_keys_bezier_loop(BeztEditData *bed, struct IpoCurve *icu, BeztEditFunc bezt_ok, BeztEditFunc bezt_cb, IcuEditFunc icu_cb);
short ipo_keys_bezier_loop(BeztEditData *bed, struct Ipo *ipo, BeztEditFunc bezt_ok, BeztEditFunc bezt_cb, IcuEditFunc icu_cb);

/* ------------ BezTriple Callback Getters --------------- */

/* accessories */
BeztEditFunc ANIM_editkeyframes_ok(short mode);

/* edit */
BeztEditFunc ANIM_editkeyframes_snap(short mode);
BeztEditFunc ANIM_editkeyframes_mirror(short mode);
BeztEditFunc ANIM_editkeyframes_select(short mode);
BeztEditFunc ANIM_editkeyframes_handles(short mode);
BeztEditFunc ANIM_editkeyframes_ipo(short mode);

/* ************************************************ */

// XXX all of these funcs will be depreceated!

void select_ipo_key(BeztEditData *bed, struct Ipo *ipo, float selx, short selectmode);
void select_icu_key(BeztEditData *bed, struct IpoCurve *icu, float selx, short selectmode);

short is_ipo_key_selected(struct Ipo *ipo);
void set_ipo_key_selection(struct Ipo *ipo, short sel);

void borderselect_ipo_key(struct Ipo *ipo, float xmin, float xmax, short selectmode);
void borderselect_icu_key(struct IpoCurve *icu, float xmin, float xmax, BeztEditFunc select_cb);


/* ************************************************ */

#endif /* ED_KEYFRAMES_EDIT_H */
