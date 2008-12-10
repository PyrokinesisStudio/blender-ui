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
 * Contributor(s): Blender Foundation
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <string.h>
#include <stdio.h>

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_arithb.h"
#include "BLI_rand.h"

#include "BKE_global.h"
#include "BKE_screen.h"
#include "BKE_utildefines.h"

#include "ED_area.h"
#include "ED_screen.h"
#include "ED_screen_types.h"

#include "WM_api.h"
#include "WM_types.h"
#include "wm_subwindow.h"

#include "BIF_gl.h"
#include "BIF_glutil.h"

#include "UI_interface.h"
#include "UI_resources.h"
#include "UI_view2d.h"

#ifndef DISABLE_PYTHON
#include "BPY_extern.h"
#endif

#include "screen_intern.h"

/* general area and region code */

static void region_draw_emboss(ARegion *ar)
{
	short winx, winy;
	
	winx= ar->winrct.xmax-ar->winrct.xmin;
	winy= ar->winrct.ymax-ar->winrct.ymin;
	
	/* set transp line */
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	/* right  */
	glColor4ub(0,0,0, 50);
	sdrawline(winx, 0, winx, winy);
	
	/* bottom  */
	glColor4ub(0,0,0, 80);
	sdrawline(0, 0, winx, 0);
	
	/* top  */
	glColor4ub(255,255,255, 60);
	sdrawline(0, winy, winx, winy);

	/* left  */
	glColor4ub(255,255,255, 50);
	sdrawline(0, 0, 0, winy);
	
	glDisable( GL_BLEND );
}

void ED_region_pixelspace(const bContext *C, ARegion *ar)
{
	int width= ar->winrct.xmax-ar->winrct.xmin+1;
	int height= ar->winrct.ymax-ar->winrct.ymin+1;
	
	wmOrtho2(C->window, -0.375, (float)width-0.375, -0.375, (float)height-0.375);
	wmLoadIdentity(C->window);	
}

void ED_region_do_listen(ARegion *ar, wmNotifier *note)
{
	
	/* generic notes first */
	switch(note->type) {
		case WM_NOTE_WINDOW_REDRAW:
		case WM_NOTE_AREA_REDRAW:
		case WM_NOTE_REGION_REDRAW:
		case WM_NOTE_GESTURE_REDRAW:
		case WM_NOTE_SCREEN_CHANGED:
			ar->do_draw= 1;
			break;
		default:
			if(ar->type->listener)
				ar->type->listener(ar, note);
	}
}

/* only internal decoration, AZone for now */
void ED_area_do_draw(bContext *C, ScrArea *sa)
{
	AZone *az;
	
	/* hrmf, screenspace for zones */
	wm_subwindow_set(C->window, C->window->screen->mainwin);
	
	/* temporary viz for 'action corner' */
	for(az= sa->actionzones.first; az; az= az->next) {
		
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glColor4ub(0, 0, 0, 80);
		if(az->type==AZONE_TRI) sdrawtrifill(az->x1, az->y1, az->x2, az->y2);
		//if(az->type==AZONE_TRI) sdrawtri(az->x1, az->y1, az->x2, az->y2);
		glDisable( GL_BLEND );
	}
	
}

void ED_region_do_draw(bContext *C, ARegion *ar)
{
	ARegionType *at= ar->type;
	
	wm_subwindow_set(C->window, ar->swinid);
	
	if(ar->swinid && at->draw) {
		UI_SetTheme(C->area);
		at->draw(C, ar);
		UI_SetTheme(NULL);
	}
	else {
		float fac= 0.1*ar->swinid;
		
		fac= fac - (int)fac;
		
		glClearColor(0.5, fac, 1.0f-fac, 0.0); 
		glClear(GL_COLOR_BUFFER_BIT);
		
		/* swapbuffers indicator */
		fac= BLI_frand();
		glColor3f(fac, fac, fac);
		glRecti(20,  2,  30,  12);
	}

	if(C->area)
		region_draw_emboss(ar);
	
	/* XXX test: add convention to end regions always in pixel space, for drawing of borders/gestures etc */
	ED_region_pixelspace(C, ar);
	
	ar->do_draw= 0;
}

/* *************************************************************** */

/* dir is direction to check, not the splitting edge direction! */
static int rct_fits(rcti *rect, char dir, int size)
{
	if(dir=='h') {
		return rect->xmax-rect->xmin - size;
	}
	else { // 'v'
		return rect->ymax-rect->ymin - size;
	}
}

static void region_rect_recursive(ARegion *ar, rcti *remainder)
{
	int prefsizex, prefsizey;
	
	if(ar==NULL)
		return;
	
	/* clear state flags first */
	ar->flag &= ~RGN_FLAG_TOO_SMALL;
	if(ar->next==NULL)
		ar->alignment= RGN_ALIGN_NONE;
	
	prefsizex= ar->type->minsizex;
	prefsizey= ar->type->minsizey;
	
	/* hidden is user flag */
	if(ar->flag & RGN_FLAG_HIDDEN);
	/* XXX floating area region, not handled yet here */
	else if(ar->alignment == RGN_ALIGN_FLOAT);
	/* remainder is too small for any usage */
	else if( rct_fits(remainder, 'v', 1)<0 || rct_fits(remainder, 'h', 1) < 0 ) {
		ar->flag |= RGN_FLAG_TOO_SMALL;
	}
	else if(ar->alignment==RGN_ALIGN_NONE) {
		/* typically last region */
		ar->winrct= *remainder;
		BLI_init_rcti(remainder, 0, 0, 0, 0);
	}
	else if(ar->alignment==RGN_ALIGN_TOP || ar->alignment==RGN_ALIGN_BOTTOM) {
		
		if( rct_fits(remainder, 'v', prefsizey) < 0 ) {
			ar->flag |= RGN_FLAG_TOO_SMALL;
		}
		else {
			int fac= rct_fits(remainder, 'v', prefsizey);

			if(fac < 0 )
				prefsizey += fac;
			
			ar->winrct= *remainder;
			
			if(ar->alignment==RGN_ALIGN_TOP) {
				ar->winrct.ymin= ar->winrct.ymax - prefsizey;
				remainder->ymax= ar->winrct.ymin-1;
			}
			else {
				ar->winrct.ymax= ar->winrct.ymin + prefsizey;
				remainder->ymin= ar->winrct.ymax+1;
			}
		}
	}
	else if(ar->alignment==RGN_ALIGN_LEFT || ar->alignment==RGN_ALIGN_RIGHT) {
		
		if( rct_fits(remainder, 'h', prefsizex) < 0 ) {
			ar->flag |= RGN_FLAG_TOO_SMALL;
		}
		else {
			int fac= rct_fits(remainder, 'h', prefsizex);
			
			if(fac < 0 )
				prefsizex += fac;
			
			ar->winrct= *remainder;
			
			if(ar->alignment==RGN_ALIGN_RIGHT) {
				ar->winrct.xmin= ar->winrct.xmax - prefsizex;
				remainder->xmax= ar->winrct.xmin-1;
			}
			else {
				ar->winrct.xmax= ar->winrct.xmin + prefsizex;
				remainder->xmin= ar->winrct.xmax+1;
			}
		}
	}
	else {
		/* percentage subdiv*/
		ar->winrct= *remainder;
		
		if(ar->alignment==RGN_ALIGN_HSPLIT) {
			if( rct_fits(remainder, 'h', prefsizex) > 4) {
				ar->winrct.xmax= (remainder->xmin+remainder->xmax)/2;
				remainder->xmin= ar->winrct.xmax+1;
			}
			else {
				BLI_init_rcti(remainder, 0, 0, 0, 0);
			}
		}
		else {
			if( rct_fits(remainder, 'v', prefsizey) > 4) {
				ar->winrct.ymax= (remainder->ymin+remainder->ymax)/2;
				remainder->ymin= ar->winrct.ymax+1;
			}
			else {
				BLI_init_rcti(remainder, 0, 0, 0, 0);
			}
		}
	}
	/* for speedup */
	ar->winx= ar->winrct.xmax - ar->winrct.xmin + 1;
	ar->winy= ar->winrct.ymax - ar->winrct.ymin + 1;
	
	region_rect_recursive(ar->next, remainder);
}

static void area_calc_totrct(ScrArea *sa, int sizex, int sizey)
{
	
	if(sa->v1->vec.x>0) sa->totrct.xmin= sa->v1->vec.x+1;
	else sa->totrct.xmin= sa->v1->vec.x;
	if(sa->v4->vec.x<sizex-1) sa->totrct.xmax= sa->v4->vec.x-1;
	else sa->totrct.xmax= sa->v4->vec.x;
	
	if(sa->v1->vec.y>0) sa->totrct.ymin= sa->v1->vec.y+1;
	else sa->totrct.ymin= sa->v1->vec.y;
	if(sa->v2->vec.y<sizey-1) sa->totrct.ymax= sa->v2->vec.y-1;
	else sa->totrct.ymax= sa->v2->vec.y;
	
	/* for speedup */
	sa->winx= sa->totrct.xmax-sa->totrct.xmin+1;
	sa->winy= sa->totrct.ymax-sa->totrct.ymin+1;
}

#define AZONESPOT		12
void area_azone_initialize(ScrArea *sa) 
{
	AZone *az;
	if(sa->actionzones.first==NULL) {
		/* set action zones - should these actually be ARegions? With these we can easier check area hotzones */
		/* (ton) for time being just area, ARegion split is not foreseen on user level */
		az= (AZone *)MEM_callocN(sizeof(AZone), "actionzone");
		BLI_addtail(&(sa->actionzones), az);
		az->type= AZONE_TRI;
		az->pos= AZONE_SW;
		
		az= (AZone *)MEM_callocN(sizeof(AZone), "actionzone");
		BLI_addtail(&(sa->actionzones), az);
		az->type= AZONE_TRI;
		az->pos= AZONE_NE;
	}
	
	for(az= sa->actionzones.first; az; az= az->next) {
		if(az->pos==AZONE_SW) {
			az->x1= sa->v1->vec.x+1;
			az->y1= sa->v1->vec.y+1;
			az->x2= sa->v1->vec.x+AZONESPOT;
			az->y2= sa->v1->vec.y+AZONESPOT;
		} 
		else if (az->pos==AZONE_NE) {
			az->x1= sa->v3->vec.x;
			az->y1= sa->v3->vec.y;
			az->x2= sa->v3->vec.x-AZONESPOT;
			az->y2= sa->v3->vec.y-AZONESPOT;
		}
	}
}

/* used for area initialize below */
static void region_subwindow(wmWindowManager *wm, wmWindow *win, ARegion *ar)
{
	if(ar->flag & (RGN_FLAG_HIDDEN|RGN_FLAG_TOO_SMALL)) {
		if(ar->swinid)
			wm_subwindow_close(win, ar->swinid);
		ar->swinid= 0;
	}
	else if(ar->swinid==0)
		ar->swinid= wm_subwindow_open(win, &ar->winrct);
	else 
		wm_subwindow_position(win, ar->swinid, &ar->winrct);
}

static void ed_default_handlers(wmWindowManager *wm, ListBase *handlers, int flag)
{
	/* note, add-handler checks if it already exists */
	
	if(flag & ED_KEYMAP_UI) {
		UI_add_region_handlers(handlers);
	}
	if(flag & ED_KEYMAP_VIEW2D) {
		ListBase *keymap= WM_keymap_listbase(wm, "View2D", 0, 0);
		WM_event_add_keymap_handler(handlers, keymap);
	}
	if(flag & ED_KEYMAP_MARKERS) {
		ListBase *keymap= WM_keymap_listbase(wm, "Markers", 0, 0);
		WM_event_add_keymap_handler(handlers, keymap);
	}
}


/* called in screen_refresh, or screens_init, also area size changes */
void ED_area_initialize(wmWindowManager *wm, wmWindow *win, ScrArea *sa)
{
	ARegion *ar;
	rcti rect;
	
	/* set typedefinitions */
	sa->type= BKE_spacetype_from_id(sa->spacetype);
	
	if(sa->type==NULL) {
		sa->spacetype= SPACE_VIEW3D;
		sa->type= BKE_spacetype_from_id(sa->spacetype);
	}
	
	for(ar= sa->regionbase.first; ar; ar= ar->next)
		ar->type= ED_regiontype_from_id(sa->type, ar->regiontype);
	
	/* area sizes */
	area_calc_totrct(sa, win->sizex, win->sizey);
	
	/* region rect sizes */
	rect= sa->totrct;
	region_rect_recursive(sa->regionbase.first, &rect);
	
	/* default area handlers */
	ed_default_handlers(wm, &sa->handlers, sa->type->keymapflag);
	/* checks spacedata, adds own handlers */
	if(sa->type->init)
		sa->type->init(wm, sa);
	
	/* region windows, default and own handlers */
	for(ar= sa->regionbase.first; ar; ar= ar->next) {
		region_subwindow(wm, win, ar);
		
		/* default region handlers */
		ed_default_handlers(wm, &ar->handlers, ar->type->keymapflag);

		if(ar->type->init)
			ar->type->init(wm, ar);
		
	}
	area_azone_initialize(sa);
}

/* externally called for floating regions like menus */
void ED_region_init(bContext *C, ARegion *ar)
{
//	ARegionType *at= ar->type;
	
	/* refresh can be called before window opened */
	region_subwindow(C->wm, C->window, ar);
	
}


ARegion *ED_region_copy(ARegion *ar)
{
	ARegion *newar= MEM_dupallocN(ar);
	
	newar->handlers.first= newar->handlers.last= NULL;
	newar->uiblocks.first= newar->uiblocks.last= NULL;
	newar->swinid= 0;
	
	/* XXX regiondata */
	if(ar->regiondata)
		newar->regiondata= MEM_dupallocN(ar->regiondata);
	
	return newar;
}

/* sa2 to sa1, we swap spaces for fullscreen to keep all allocated data */
/* area vertices were set */
void area_copy_data(ScrArea *sa1, ScrArea *sa2, int swap_space)
{
	Panel *pa1, *pa2, *patab;
	ARegion *ar;
	
	sa1->headertype= sa2->headertype;
	sa1->spacetype= sa2->spacetype;
	
	if(swap_space) {
		SWAP(ListBase, sa1->spacedata, sa2->spacedata);
		/* exception: ensure preview is reset */
//		if(sa1->spacetype==SPACE_VIEW3D)
// XXX			BIF_view3d_previewrender_free(sa1->spacedata.first);
	}
	else {
		BKE_spacedata_freelist(&sa1->spacedata);
		BKE_spacedata_copylist(&sa1->spacedata, &sa2->spacedata);
	}
	
	BLI_freelistN(&sa1->panels);
	BLI_duplicatelist(&sa1->panels, &sa2->panels);
	
	/* copy panel pointers */
	for(pa1= sa1->panels.first; pa1; pa1= pa1->next) {
		
		patab= sa1->panels.first;
		pa2= sa2->panels.first;
		while(patab) {
			if( pa1->paneltab == pa2) {
				pa1->paneltab = patab;
				break;
			}
			patab= patab->next;
			pa2= pa2->next;
		}
	}
	
	/* regions */
	BLI_freelistN(&sa1->regionbase);
	
	for(ar= sa2->regionbase.first; ar; ar= ar->next) {
		ARegion *newar= ED_region_copy(ar);
		BLI_addtail(&sa1->regionbase, newar);
	}
		
#ifndef DISABLE_PYTHON
	/* scripts */
	BPY_free_scriptlink(&sa1->scriptlink);
	sa1->scriptlink= sa2->scriptlink;
	BPY_copy_scriptlink(&sa1->scriptlink);	/* copies internal pointers */
#endif
}


