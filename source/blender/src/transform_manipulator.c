/**
* $Id:
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
 * The Original Code is Copyright (C) 2005 Blender Foundation
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#include "BLI_winstuff.h"
#endif

#include "MEM_guardedalloc.h"

#include "DNA_armature_types.h"
#include "DNA_curve_types.h"
#include "DNA_lattice_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meta_types.h"
#include "DNA_object_types.h"
#include "DNA_screen_types.h"
#include "DNA_scene_types.h"
#include "DNA_space_types.h"
#include "DNA_view3d_types.h"

#include "BKE_global.h"
#include "BKE_lattice.h"
#include "BKE_object.h"
#include "BKE_utildefines.h"

#include "BLI_arithb.h"
#include "BLI_editVert.h"

#include "BIF_editarmature.h"
#include "BIF_gl.h"
#include "BIF_mywindow.h"
#include "BIF_resources.h"
#include "BIF_screen.h"
#include "BIF_space.h"
#include "BIF_transform.h"

#include "BSE_edit.h"
#include "BSE_view.h"

#include "blendef.h"
#include "transform.h"

/* drawing defines */
#define MAN_SIZE	0.15
#define CYLEN	0.25
#define CYWID	0.08

/* return codes for select */

#define MAN_TRANS_X		1
#define MAN_TRANS_Y		2
#define MAN_TRANS_Z		4
#define MAN_TRANS_C		7

#define MAN_ROT_X		8
#define MAN_ROT_Y		16
#define MAN_ROT_Z		32
#define MAN_ROT_C		63


/* GLOBAL VARIABLE THAT SHOULD MOVED TO SCREEN MEMBER OR SOMETHING  */
extern TransInfo Trans;


/* transform widget center calc helper for below */
static void calc_tw_center(float *co)
{
	float *twcent= G.scene->twcent;
	float *min= G.scene->twmin;
	float *max= G.scene->twmax;
	
	DO_MINMAX(co, min, max);
	VecAddf(twcent, twcent, co);
}


/* centroid, boundbox, of selection */
/* returns total items selected */
static int calc_manipulator(ScrArea *sa)
{
	extern ListBase editNurb;
	View3D *v3d= sa->spacedata.first;
	Base *base;
	Object *ob=NULL;
	Nurb *nu;
	BezTriple *bezt;
	BPoint *bp;
	MetaElem *ml;
	EditVert *eve;
//	EditFace *efa;
	EditBone *ebo;
	int a, totsel=0;
	
	/* transform widget matrix */
	Mat4One(v3d->twmat);
	
	/* transform widget centroid/center */
	G.scene->twcent[0]= G.scene->twcent[1]= G.scene->twcent[2]= 0.0f;
	INIT_MINMAX(G.scene->twmin, G.scene->twmax);
	
	if(G.obedit) {
		ob= G.obedit;
		
		if(G.obedit->type==OB_MESH) {
			EditMesh *em = G.editMesh;
			eve= em->verts.first;
			while(eve) {
				if(eve->f & SELECT) {
					totsel++;
					calc_tw_center(eve->co);
				}
				eve= eve->next;
			}
		}
		else if (G.obedit->type==OB_ARMATURE){
			for (ebo=G.edbo.first;ebo;ebo=ebo->next){
				
				//	If this is an IK child and it's parent is being moved, don't count as selected
				if ((ebo->flag & BONE_IK_TOPARENT)&& (ebo->flag & BONE_ROOTSEL) && ebo->parent && (ebo->parent->flag & BONE_TIPSEL));
				else {
					if (ebo->flag & BONE_TIPSEL) {
						calc_tw_center(ebo->tail);
						totsel++;
					}
					if (ebo->flag & BONE_ROOTSEL) {
						calc_tw_center(ebo->head);
						totsel++;
					}
				}
			}
		}
		else if ELEM3(G.obedit->type, OB_CURVE, OB_SURF, OB_FONT) {
			nu= editNurb.first;
			while(nu) {
				if((nu->type & 7)==CU_BEZIER) {
					bezt= nu->bezt;
					a= nu->pntsu;
					while(a--) {
						if(bezt->f1) {
							calc_tw_center(bezt->vec[0]);
							totsel++;
						}
						if(bezt->f2) {
							calc_tw_center(bezt->vec[1]);
							totsel++;
						}
						if(bezt->f3) {
							calc_tw_center(bezt->vec[2]);
							totsel++;
						}
						bezt++;
					}
				}
				else {
					bp= nu->bp;
					a= nu->pntsu*nu->pntsv;
					while(a--) {
						if(bp->f1 & 1) {
							calc_tw_center(bp->vec);
							totsel++;
						}
						bp++;
					}
				}
				nu= nu->next;
			}
		}
		else if(G.obedit->type==OB_MBALL) {
			/* editmball.c */
			extern ListBase editelems;  /* go away ! */
			
			ml= editelems.first;
			while(ml) {
				if(ml->flag & SELECT) {
					calc_tw_center(&ml->x);
					totsel++;
				}
				ml= ml->next;
			}
		}
		else if(G.obedit->type==OB_LATTICE) {
			bp= editLatt->def;
			
			a= editLatt->pntsu*editLatt->pntsv*editLatt->pntsw;
			while(a--) {
				if(bp->f1 & 1) {
					calc_tw_center(bp->vec);
					totsel++;
				}
				bp++;
			}
		}
		
		/* selection center */
		if(totsel) {
			VecMulf(G.scene->twcent, 1.0f/(float)totsel);	// centroid!
			Mat4MulVecfl(G.obedit->obmat, G.scene->twcent);
			Mat4MulVecfl(G.obedit->obmat, G.scene->twmin);
			Mat4MulVecfl(G.obedit->obmat, G.scene->twmax);
		}
	}
	else if(G.obpose) {
		;
	}
	else if(G.f & (G_FACESELECT + G_VERTEXPAINT + G_TEXTUREPAINT +G_WEIGHTPAINT)) {
		;
	}
	else {
		
		/* we need the one selected object */
		ob= OBACT;
		if(ob && !(ob->flag & SELECT)) ob= NULL;
		
		base= (G.scene->base.first);
		while(base) {
			if(v3d->lay & base->lay) {
				
				if(base->flag & SELECT) {
					if(ob==NULL) ob= base->object;
					calc_tw_center(base->object->obmat[3]);
					totsel++;
				}
			}
			base= base->next;
		}
		
		/* selection center */
		if(totsel) {
			VecMulf(G.scene->twcent, 1.0f/(float)totsel);	// centroid!
		}
	}
	
	/* global, local or normal orientation? */
	if(ob) {
		// local....
		if(totsel==1 || v3d->around==V3D_LOCAL) {
			Mat4CpyMat4(v3d->twmat, ob->obmat);
			Mat4Ortho(v3d->twmat);
		}		
	}
	   
	return totsel;
}

static void manipulator_setcolor(char mode)
{
	float vec[4];
	
	vec[3]= 1.0;
	
	if(G.moving) {
		if(mode > 'Z') BIF_ThemeColor(TH_TRANSFORM);
		else {
			BIF_GetThemeColor3fv(TH_TRANSFORM, vec);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vec);
		}
	}
	else {
		switch(mode) {
		case 'x':
			glColor3ub(255, 100, 100);
			break;
		case 'y':
			glColor3ub(100, 255, 100);
			break;
		case 'z':
			glColor3ub(50, 50, 225);
			break;
		case 'X':
			vec[2]= vec[1]= 0.0; vec[0]= 1.0;
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vec);
			break;
		case 'Y':
			vec[0]= vec[2]= 0.0; vec[1]= 1.0;
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vec);
			break;
		case 'Z':
			vec[0]= vec[1]= 0.0; vec[2]= 1.0;
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vec);
			break;
		}
	}
}

static int Gval= 0xFFFF;	// defines drawmodus while moving...


static void draw_manipulator_translate(float mat[][4])
{
	GLUquadricObj *qobj = gluNewQuadric(); 
	
	if(G.moving) glTranslatef(Trans.vec[0], Trans.vec[1], Trans.vec[2]);
	
	glMultMatrixf(mat);
	
	/* if not shiftkey, center point as first, for selectbuffer order */
	if(G.f & G_PICKSEL) {
		if(!(G.qual & LR_SHIFTKEY)) {
			glLoadName(MAN_TRANS_C);
			glBegin(GL_POINTS);
			glVertex3f(0.0, 0.0, 0.0);
			glEnd();
		}
	}
	else {
		float vec[3];
		
		glDisable(GL_DEPTH_TEST);
		
		/* axis */
		glBegin(GL_LINES);
		if(Gval & MAN_TRANS_X) {
			manipulator_setcolor('x');
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(1.0, 0.0, 0.0);
		}		
		if(Gval & MAN_TRANS_Y) {
			manipulator_setcolor('y');
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, 1.0, 0.0);
		}		
		if(Gval & MAN_TRANS_Z) {
			manipulator_setcolor('z');
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 1.0);
		}
		glEnd();
		
		/* only has to be set when not in picking */
		gluQuadricDrawStyle(qobj, GLU_FILL); 
		gluQuadricNormals(qobj, GLU_SMOOTH);
		glEnable(GL_CULL_FACE);		// backface removal
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		
		/* center sphere */
		BIF_GetThemeColor3fv(TH_TRANSFORM, vec);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, vec);

		gluSphere(qobj, CYWID, 8, 6); 
	}	
	
	/* Z cylinder */
	glTranslatef(0.0, 0.0, 1.0 - CYLEN);
	if(Gval & MAN_TRANS_Z) {
		if(G.f & G_PICKSEL) glLoadName(MAN_TRANS_Z);
		manipulator_setcolor('Z');
		gluCylinder(qobj, CYWID, 0.0, CYLEN, 8, 1);
		gluQuadricOrientation(qobj, GLU_INSIDE);
		gluDisk(qobj, 0.0, CYWID, 8, 1); 
		gluQuadricOrientation(qobj, GLU_OUTSIDE);
	}	
	/* X cylinder */
	glTranslatef(1.0 - CYLEN, 0.0, -(1.0 - CYLEN));
	if(Gval & MAN_TRANS_X) {
		if(G.f & G_PICKSEL) glLoadName(MAN_TRANS_X);
		glRotatef(90.0, 0.0, 1.0, 0.0);
		manipulator_setcolor('X');
		gluCylinder(qobj, CYWID, 0.0, CYLEN, 8, 1);
		gluQuadricOrientation(qobj, GLU_INSIDE);
		gluDisk(qobj, 0.0, CYWID, 8, 1); 
		gluQuadricOrientation(qobj, GLU_OUTSIDE);
		glRotatef(-90.0, 0.0, 1.0, 0.0);
	}	
	/* Y cylinder */
	glTranslatef(-(1.0 - CYLEN), 1.0 - CYLEN, 0.0);
	if(Gval & MAN_TRANS_Y) {
		if(G.f & G_PICKSEL) glLoadName(MAN_TRANS_Y);
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		manipulator_setcolor('Y');
		gluCylinder(qobj, CYWID, 0.0, CYLEN, 8, 1);
		gluQuadricOrientation(qobj, GLU_INSIDE);
		gluDisk(qobj, 0.0, CYWID, 8, 1); 
	}
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	
	/* if shiftkey, center point as last, for selectbuffer order */
	if(G.f & G_PICKSEL) {
		if(G.qual & LR_SHIFTKEY) {
			glRotatef(90.0, 1.0, 0.0, 0.0);
			glTranslatef(0.0, -(1.0 - CYLEN), 0.0);
			glLoadName(MAN_TRANS_C);
			glBegin(GL_POINTS);
			glVertex3f(0.0, 0.0, 0.0);
			glEnd();
		}
	}
	
	gluDeleteQuadric(qobj);
	myloadmatrix(G.vd->viewmat);
	
	if(G.zbuf) glEnable(GL_DEPTH_TEST);		// shouldn't be global, tsk!
	
}

/* main call, does calc centers & orientation too */
void BIF_draw_manipulator(ScrArea *sa)
{
	View3D *v3d= sa->spacedata.first;
	float len1, len2, size, vec[3];
	int totsel;
	
	if(v3d->twflag & V3D_USE_MANIPULATOR); else return;
	
	if(G.moving==0) {
		v3d->twflag &= ~V3D_DRAW_MANIPULATOR;
		
		totsel= calc_manipulator(sa);
		if(totsel==0) return;
		
		v3d->twflag |= V3D_DRAW_MANIPULATOR;

		/* now we can define centre */
		switch(v3d->around) {
		case V3D_CENTRE:
		case V3D_LOCAL:
			v3d->twmat[3][0]= (G.scene->twmin[0] + G.scene->twmax[0])/2.0;
			v3d->twmat[3][1]= (G.scene->twmin[1] + G.scene->twmax[1])/2.0;
			v3d->twmat[3][2]= (G.scene->twmin[2] + G.scene->twmax[2])/2.0;
			break;
		case V3D_CENTROID:
			VECCOPY(v3d->twmat[3], G.scene->twcent);
			break;
		case V3D_CURSOR:
			VECCOPY(v3d->twmat[3], G.scene->cursor);
			break;
		}
		
		/* size calculus, depending ortho/persp settings, like initgrabz() */
		size= G.vd->persmat[0][3]*v3d->twmat[3][0]+ G.vd->persmat[1][3]*v3d->twmat[3][1]+ G.vd->persmat[2][3]*v3d->twmat[3][2]+ G.vd->persmat[3][3];
		
		VECCOPY(vec, G.vd->persinv[0]);
		len1= Normalise(vec);
		VECCOPY(vec, G.vd->persinv[1]);
		len2= Normalise(vec);
		
		size*= MAN_SIZE*(len1>len2?len1:len2);
		Mat4MulFloat3((float *)v3d->twmat, size);
	}
	
	if(v3d->twflag & V3D_DRAW_MANIPULATOR) {
		
		if(v3d->twmode==V3D_MANIPULATOR_TRANSLATE)
			draw_manipulator_translate(v3d->twmat);
	}
}



static int manipulator_selectbuf(ScrArea *sa)
{
	View3D *v3d= sa->spacedata.first;
	rctf rect;
	GLuint buffer[32];		// max 4 items per select, so large enuf
	short hits, mval[2];
	
	G.f |= G_PICKSEL;
	
	getmouseco_areawin(mval);
	rect.xmin= mval[0]-7;
	rect.xmax= mval[0]+7;
	rect.ymin= mval[1]-7;
	rect.ymax= mval[1]+7;
	
	/* get rid of overlay button matrix */
	persp(PERSP_VIEW);
	
	setwinmatrixview3d(&rect);
	Mat4MulMat4(G.vd->persmat, G.vd->viewmat, curarea->winmat);

	glSelectBuffer( MAXPICKBUF, (GLuint *)buffer);
	glRenderMode(GL_SELECT);
	glInitNames();	/* these two calls whatfor? It doesnt work otherwise */
	glPushName(-1);
	
	/* do the drawing */
	if(v3d->twmode==V3D_MANIPULATOR_TRANSLATE)
		draw_manipulator_translate(v3d->twmat);
	
	
	glPopName();	/* see above (pushname) */
	hits= glRenderMode(GL_RENDER);
	
	G.f &= ~G_PICKSEL;
	setwinmatrixview3d(0);
	Mat4MulMat4(G.vd->persmat, G.vd->viewmat, curarea->winmat);
	
	persp(PERSP_WIN);
	
	if(hits>0) {
		return buffer[3];
	}
	return 0;
}

int BIF_do_manipulator(ScrArea *sa)
{
	View3D *v3d= sa->spacedata.first;
	int val;
	
	if(!(v3d->twflag & V3D_USE_MANIPULATOR)) return 0;
	if(!(v3d->twflag & V3D_DRAW_MANIPULATOR)) return 0;
	
	// find the hotspots (Gval is for draw)
	Gval=val= manipulator_selectbuf(sa);
	
	switch(val) {
		case MAN_TRANS_C:
			ManipulatorTransform(TFM_TRANSLATION);
			break;
		case MAN_TRANS_X:
			if(G.qual & LR_SHIFTKEY) {
				Gval= MAN_TRANS_Y|MAN_TRANS_Z;
				BIF_setDualAxisConstraint(v3d->twmat[1], v3d->twmat[2]);
			}
			else
				BIF_setSingleAxisConstraint(v3d->twmat[0]);
			ManipulatorTransform(TFM_TRANSLATION);
			break;
		case MAN_TRANS_Y:
			if(G.qual & LR_SHIFTKEY) {
				Gval= MAN_TRANS_X|MAN_TRANS_Z;
				BIF_setDualAxisConstraint(v3d->twmat[0], v3d->twmat[2]);
			}
			else
				BIF_setSingleAxisConstraint(v3d->twmat[1]);
			ManipulatorTransform(TFM_TRANSLATION);
			break;
		case MAN_TRANS_Z:
			if(G.qual & LR_SHIFTKEY) {
				Gval= MAN_TRANS_X|MAN_TRANS_Y;
				BIF_setDualAxisConstraint(v3d->twmat[0], v3d->twmat[1]);
			}
			else
				BIF_setSingleAxisConstraint(v3d->twmat[2]);
			ManipulatorTransform(TFM_TRANSLATION);
			break;
	}
	
	Gval= 0xFFFF;
	
	return val;
}


