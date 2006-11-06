/**
 * $Id$ 
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
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
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */
#ifndef DNA_MESHDATA_TYPES_H
#define DNA_MESHDATA_TYPES_H

struct Bone;

typedef struct MFace {
	unsigned int v1, v2, v3, v4;
	char pad, mat_nr;
	char edcode, flag;	/* we keep edcode, for conversion to edges draw flags in old files */
} MFace;

typedef struct MEdge {
	unsigned int v1, v2;
	char crease, pad;
	short flag;
} MEdge;

typedef struct MDeformWeight {
	int				def_nr;
	float			weight;
} MDeformWeight;

typedef struct MDeformVert {
	struct MDeformWeight *dw;
	int totweight;
	int flag;	// flag only in use for weightpaint now
} MDeformVert;

typedef struct MVert {
	float	co[3];
	short	no[3];
	char flag, mat_nr;
} MVert;

typedef struct MCol {
	char a, r, g, b;
} MCol;

typedef struct MSticky {
	float co[2];
} MSticky;

typedef struct MSelect {
	int index;
	int type;
} MSelect;

/* Multiresolution modeling */
typedef struct MultiresCol {
	float a, r, g, b, u, v;
} MultiresCol;
typedef struct MultiresFace {
	unsigned int v[4];
       	unsigned int mid;
	unsigned int childrenstart;
	char flag, pad[3];
} MultiresFace;
typedef struct MultiresEdge {
	unsigned int v[2];
	unsigned int mid;
} MultiresEdge;

typedef struct MultiresTexColFace {
	/* vertex colors and texfaces */
	void *tex_page;
	MultiresCol col[4];
	short tex_mode, tex_tile, tex_unwrap;
	char tex_flag, tex_transp;
} MultiresTexColFace;

typedef struct MultiresMapNode {
	struct MultiresMapNode *next, *prev;
	int Index, pad;
} MultiresMapNode;

typedef struct MultiresLevel {
	struct MultiresLevel *next, *prev;

	MVert *verts;
	MultiresFace *faces;
	MultiresTexColFace *texcolfaces;
	MultiresEdge *edges;
	ListBase *vert_edge_map;
	ListBase *vert_face_map;

	unsigned int totvert, totface, totedge, pad;
} MultiresLevel;

typedef struct Multires {
	ListBase levels;
	unsigned char level_count, current, newlvl, edgelvl, pinlvl, renderlvl;
	unsigned char use_col, use_tex;

	/* Vertex groups are stored only for the level 1 mesh, for all other
	 * levels it's calculated when multires_level_to_mesh() is called */
	MDeformVert *dverts;
} Multires;

typedef struct PartialVisibility {
	unsigned int *vert_map; /* vert_map[Old Index]= New Index */
	int *edge_map; /* edge_map[Old Index]= New Index, -1= hidden */
	MFace *old_faces;
	MEdge *old_edges;
	unsigned int totface, totedge, totvert, pad;
} PartialVisibility;

/* mvert->flag (1=SELECT) */
#define ME_SPHERETEST	2
#define ME_SPHERETEMP	4
#define ME_HIDE			16
#define ME_VERT_MERGED		(1<<6)

/* medge->flag (1=SELECT)*/
#define ME_EDGEDRAW			(1<<1)
#define ME_SEAM				(1<<2)
#define ME_FGON				(1<<3)
						// reserve 16 for ME_HIDE
#define ME_EDGERENDER		(1<<5)
#define ME_LOOSEEDGE		(1<<7)
#define ME_SEAM_LAST		(1<<8)
#define ME_SHARP			(1<<9)

/* puno = vertexnormal (mface) */
#define ME_FLIPV1		1
#define ME_FLIPV2		2
#define ME_FLIPV3		4
#define ME_FLIPV4		8
#define ME_PROJXY		16
#define ME_PROJXZ		32
#define ME_PROJYZ		64

/* edcode (mface) */
#define ME_V1V2			1
#define ME_V2V3			2
#define ME_V3V1			4
#define ME_V3V4			4
#define ME_V4V1			8

/* flag (mface) */
#define ME_SMOOTH			1
#define ME_FACE_SEL			2
						/* flag ME_HIDE==16 is used here too */ 

#endif
/* mselect->type */
#define ME_VSEl	0
#define ME_ESEl 1
#define ME_FSEL 2
