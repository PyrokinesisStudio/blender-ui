/**
 * $Id: text_ops.c 21549 2009-07-12 12:47:34Z campbellbarton $
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
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h> /* ispunct */
#include <sys/stat.h>

#include "MEM_guardedalloc.h"

#include "DNA_scene_types.h"
#include "DNA_screen_types.h"
#include "DNA_space_types.h"
#include "DNA_windowmanager_types.h"

#include "BLI_blenlib.h"
#include "PIL_time.h"

#include "BKE_utildefines.h"
#include "BKE_context.h"
#include "BKE_depsgraph.h"
#include "BKE_global.h"
#include "BKE_library.h"
#include "BKE_main.h"
#include "BKE_report.h"
// #include "BKE_suggestions.h"
//#include "BKE_text.h"

#include "WM_api.h"
#include "WM_types.h"

#include "ED_screen.h"
#include "UI_interface.h"
#include "UI_resources.h"

#include "RNA_access.h"
#include "RNA_define.h"

#include "console_intern.h"

void console_history_free(SpaceConsole *sc, ConsoleLine *cl)
{
	BLI_remlink(&sc->history, cl);
	MEM_freeN(cl->line);
	MEM_freeN(cl);
}
void console_scrollback_free(SpaceConsole *sc, ConsoleLine *cl)
{
	BLI_remlink(&sc->scrollback, cl);
	MEM_freeN(cl->line);
	MEM_freeN(cl);
}

void console_scrollback_limit(SpaceConsole *sc)
{
	int tot;
	for(tot= BLI_countlist(&sc->scrollback); tot > CONSOLE_SCROLLBACK_LIMIT; tot--)
		console_scrollback_free(sc, sc->scrollback.first);
}

/* return 0 if no change made, clamps the range */
static int console_line_cursor_set(ConsoleLine *cl, int cursor)
{
	int cursor_new;
	
	if(cursor < 0)				cursor_new= 0;
	else if(cursor > cl->len)	cursor_new= cl->len;
	else						cursor_new= cursor;
	
	if(cursor_new == cl->cursor)
		return 0;
	
	cl->cursor= cursor_new;
	return 1;
}

static ConsoleLine *console_lb_add__internal(ListBase *lb, ConsoleLine *from)
{
	ConsoleLine *ci= MEM_callocN(sizeof(ConsoleLine), "ConsoleLine Add");
	
	if(from) {
		ci->line= BLI_strdup(from->line);
		ci->len= strlen(ci->line);
		ci->len_alloc= ci->len;
		
		ci->cursor= from->cursor;
		ci->type= from->type;
	} else {
		ci->line= MEM_callocN(64, "console-in-line");
		ci->len_alloc= 64;
		ci->len= 0;
	}
	
	BLI_addtail(lb, ci);
	return ci;
}

static ConsoleLine *console_history_add(const bContext *C, ConsoleLine *from)
{
	SpaceConsole *sc= CTX_wm_space_console(C);
	
	return console_lb_add__internal(&sc->history, from);
}

static ConsoleLine *console_scrollback_add(const bContext *C, ConsoleLine *from)
{
	SpaceConsole *sc= CTX_wm_space_console(C);
	
	return console_lb_add__internal(&sc->scrollback, from);
}

static ConsoleLine *console_lb_add_str__internal(ListBase *lb, const bContext *C, char *str, int own)
{
	ConsoleLine *ci= MEM_callocN(sizeof(ConsoleLine), "ConsoleLine Add");
	if(own)		ci->line= str;
	else		ci->line= BLI_strdup(str);
	
	ci->len = ci->len_alloc = strlen(str);
	
	BLI_addtail(lb, ci);
	return ci;
}
ConsoleLine *console_history_add_str(const bContext *C, char *str, int own)
{
	return console_lb_add_str__internal(&CTX_wm_space_console(C)->history, C, str, own);
}
ConsoleLine *console_scrollback_add_str(const bContext *C, char *str, int own)
{
	return console_lb_add_str__internal(&CTX_wm_space_console(C)->scrollback, C, str, own);
}

ConsoleLine *console_history_verify(const bContext *C)
{
	SpaceConsole *sc= CTX_wm_space_console(C);
	ConsoleLine *ci= sc->history.last;
	if(ci==NULL)
		ci= console_history_add(C, NULL);
	
	return ci;
}


static void console_line_verify_length(ConsoleLine *ci, int len)
{
	/* resize the buffer if needed */
	if(len > ci->len_alloc) {
		int new_len= len * 2; /* new length */
		char *new_line= MEM_callocN(new_len, "console line");
		memcpy(new_line, ci->line, ci->len);
		MEM_freeN(ci->line);
		
		ci->line= new_line;
		ci->len_alloc= new_len;
	}
}

static int console_line_insert(ConsoleLine *ci, char *str)
{
	int len = strlen(str);
	
	if(len==0)
		return 0;
	
	console_line_verify_length(ci, len + ci->len);
	
	memmove(ci->line+ci->cursor+len, ci->line+ci->cursor, (ci->len - ci->cursor)+1);
	memcpy(ci->line+ci->cursor, str, len);
	
	ci->len += len;
	ci->cursor += len;
	
	return len;
}

static int console_edit_poll(const bContext *C)
{
	SpaceConsole *sc= CTX_wm_space_console(C);

	if(!sc || sc->type != CONSOLE_TYPE_PYTHON)
		return 0;

	return 1;
}

/* static funcs for text editing */


/* similar to the text editor, with some not used. keep compatible */
static EnumPropertyItem move_type_items[]= {
	{LINE_BEGIN, "LINE_BEGIN", 0, "Line Begin", ""},
	{LINE_END, "LINE_END", 0, "Line End", ""},
	{PREV_CHAR, "PREVIOUS_CHARACTER", 0, "Previous Character", ""},
	{NEXT_CHAR, "NEXT_CHARACTER", 0, "Next Character", ""},
	{PREV_WORD, "PREVIOUS_WORD", 0, "Previous Word", ""},
	{NEXT_WORD, "NEXT_WORD", 0, "Next Word", ""},
	{0, NULL, 0, NULL, NULL}};
	
static int move_exec(const bContext *C, wmOperator *op)
{
	ConsoleLine *ci= console_history_verify(C);
	
	int type= RNA_enum_get(op->ptr, "type");
	int done= 0;
	
	switch(type) {
	case LINE_BEGIN:
		done= console_line_cursor_set(ci, 0);
		break;
	case LINE_END:
		done= console_line_cursor_set(ci, INT_MAX);
		break;
	case PREV_CHAR:
		done= console_line_cursor_set(ci, ci->cursor-1);
		break;
	case NEXT_CHAR:
		done= console_line_cursor_set(ci, ci->cursor+1);
		break;
	}
	
	if(done) {
		ED_area_tag_redraw(CTX_wm_area(C));
	}
	
	return OPERATOR_FINISHED;
}

void CONSOLE_OT_move(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Move Cursor";
	ot->idname= "CONSOLE_OT_move";
	
	/* api callbacks */
	ot->exec= move_exec;
	ot->poll= console_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER;

	/* properties */
	RNA_def_enum(ot->srna, "type", move_type_items, LINE_BEGIN, "Type", "Where to move cursor to.");
}


static int insert_exec(const bContext *C, wmOperator *op)
{
	ConsoleLine *ci= console_history_verify(C);
	char *str= RNA_string_get_alloc(op->ptr, "text", NULL, 0);
	
	int len= console_line_insert(ci, str);
	
	MEM_freeN(str);
	
	if(len==0)
		return OPERATOR_CANCELLED;
		
	ED_area_tag_redraw(CTX_wm_area(C));
	
	return OPERATOR_FINISHED;
}

static int insert_invoke(const bContext *C, wmOperator *op, wmEvent *event)
{
	char str[2] = {event->ascii, '\0'};
	RNA_string_set(op->ptr, "text", str);
	return insert_exec(C, op);
}

void CONSOLE_OT_insert(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Insert";
	ot->idname= "CONSOLE_OT_insert";
	
	/* api callbacks */
	ot->exec= insert_exec;
	ot->invoke= insert_invoke;
	ot->poll= console_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER;

	/* properties */
	RNA_def_string(ot->srna, "text", "", 0, "Text", "Text to insert at the cursor position.");
}


static EnumPropertyItem delete_type_items[]= {
	{DEL_NEXT_CHAR, "NEXT_CHARACTER", 0, "Next Character", ""},
	{DEL_PREV_CHAR, "PREVIOUS_CHARACTER", 0, "Previous Character", ""},
//	{DEL_NEXT_WORD, "NEXT_WORD", 0, "Next Word", ""},
//	{DEL_PREV_WORD, "PREVIOUS_WORD", 0, "Previous Word", ""},
	{0, NULL, 0, NULL, NULL}};

static int delete_exec(const bContext *C, wmOperator *op)
{
	
	ConsoleLine *ci= console_history_verify(C);
	
	
	int done = 0;

	int type= RNA_enum_get(op->ptr, "type");
	
	if(ci->len==0) {
		return OPERATOR_CANCELLED;
	}
	
	switch(type) {
	case DEL_NEXT_CHAR:
		if(ci->cursor < ci->len) {
			memmove(ci->line + ci->cursor, ci->line + ci->cursor+1, (ci->len - ci->cursor)+1);
			ci->len--;
			done= 1;
		}
		break;
	case DEL_PREV_CHAR:
		if(ci->cursor > 0) {
			ci->cursor--; /* same as above */
			memmove(ci->line + ci->cursor, ci->line + ci->cursor+1, (ci->len - ci->cursor)+1);
			ci->len--;
			done= 1;
		}
		break;
	}
	
	if(!done)
		return OPERATOR_CANCELLED;
	
	ED_area_tag_redraw(CTX_wm_area(C));
	
	return OPERATOR_FINISHED;
}


void CONSOLE_OT_delete(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Delete";
	ot->idname= "CONSOLE_OT_delete";
	
	/* api callbacks */
	ot->exec= delete_exec;
	ot->poll= console_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER;

	/* properties */
	RNA_def_enum(ot->srna, "type", delete_type_items, DEL_NEXT_CHAR, "Type", "Which part of the text to delete.");
}


/* the python exec operator uses this */
static int clear_exec(const bContext *C, wmOperator *op)
{
	SpaceConsole *sc= CTX_wm_space_console(C);
	
	short scrollback= RNA_boolean_get(op->ptr, "scrollback");
	short history= RNA_boolean_get(op->ptr, "history");
	
	/*ConsoleLine *ci= */ console_history_verify(C);
	
	if(scrollback) { /* last item in mistory */
		while(sc->scrollback.first)
			console_scrollback_free(sc, sc->scrollback.first);
	}
	
	if(history) {
		while(sc->history.first)
			console_history_free(sc, sc->history.first);
	}
	
	ED_area_tag_redraw(CTX_wm_area(C));
	
	return OPERATOR_FINISHED;
}

void CONSOLE_OT_clear(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Clear";
	ot->idname= "CONSOLE_OT_clear";
	
	/* api callbacks */
	ot->exec= clear_exec;
	ot->poll= console_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER;
	
	/* properties */
	RNA_def_boolean(ot->srna, "scrollback", 1, "Scrollback", "Clear the scrollback history");
	RNA_def_boolean(ot->srna, "history", 0, "History", "Clear the command history");
}



/* the python exec operator uses this */
static int history_cycle_exec(const bContext *C, wmOperator *op)
{
	SpaceConsole *sc= CTX_wm_space_console(C);
	ConsoleLine *ci= console_history_verify(C); /* TODO - stupid, just prevernts crashes when no command line */
	
	short reverse= RNA_boolean_get(op->ptr, "reverse"); /* assumes down, reverse is up */
	
	if(reverse) { /* last item in mistory */
		ci= sc->history.last;
		BLI_remlink(&sc->history, ci);
		BLI_addhead(&sc->history, ci);
	}
	else {
		ci= sc->history.first;
		BLI_remlink(&sc->history, ci);
		BLI_addtail(&sc->history, ci);
	}
	
	ED_area_tag_redraw(CTX_wm_area(C));
	
	return OPERATOR_FINISHED;
}

void CONSOLE_OT_history_cycle(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "History Cycle";
	ot->idname= "CONSOLE_OT_history_cycle";
	
	/* api callbacks */
	ot->exec= history_cycle_exec;
	ot->poll= console_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER;
	
	/* properties */
	RNA_def_boolean(ot->srna, "reverse", 0, "Reverse", "reverse cycle history");
}


/* the python exec operator uses this */
static int history_append_exec(const bContext *C, wmOperator *op)
{
	ConsoleLine *ci= console_history_verify(C);
	
	
	char *str= RNA_string_get_alloc(op->ptr, "text", NULL, 0); /* own this text in the new line, dont free */
	int cursor= RNA_int_get(op->ptr, "current_character");
	
	ci= console_history_add_str(C, str, 1); /* own the string */
	console_line_cursor_set(ci, cursor);
	
	ED_area_tag_redraw(CTX_wm_area(C));
	
	return OPERATOR_FINISHED;
}

void CONSOLE_OT_history_append(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "History Append";
	ot->idname= "CONSOLE_OT_history_append";
	
	/* api callbacks */
	ot->exec= history_append_exec;
	ot->poll= console_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER;
	
	/* properties */
	RNA_def_string(ot->srna, "text", "", 0, "Text", "Text to insert at the cursor position.");	
	RNA_def_int(ot->srna, "current_character", 0, 0, INT_MAX, "Cursor", "The index of the cursor.", 0, 10000);
}


/* the python exec operator uses this */
static int scrollback_append_exec(const bContext *C, wmOperator *op)
{
	SpaceConsole *sc= CTX_wm_space_console(C);
	ConsoleLine *ci= console_history_verify(C);
	
	char *str= RNA_string_get_alloc(op->ptr, "text", NULL, 0); /* own this text in the new line, dont free */
	int type= RNA_enum_get(op->ptr, "type");
	
	ci= console_scrollback_add_str(C, str, 1); /* own the string */
	ci->type= type;
	
	console_scrollback_limit(sc);
	
	ED_area_tag_redraw(CTX_wm_area(C));
	
	return OPERATOR_FINISHED;
}

void CONSOLE_OT_scrollback_append(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Scrollback Append";
	ot->idname= "CONSOLE_OT_scrollback_append";
	
	/* api callbacks */
	ot->exec= scrollback_append_exec;
	ot->poll= console_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER;
	
	/* properties */
	RNA_def_string(ot->srna, "text", "", 0, "Text", "Text to insert at the cursor position.");	
	RNA_def_enum(ot->srna, "type", console_line_type_items, CONSOLE_LINE_OUTPUT, "Type", "Console output type.");
}

static int zoom_exec(const bContext *C, wmOperator *op)
{
	SpaceConsole *sc= CTX_wm_space_console(C);
	
	int delta= RNA_int_get(op->ptr, "delta");
	
	sc->lheight += delta;
	CLAMP(sc->lheight, 8, 32);
	
	ED_area_tag_redraw(CTX_wm_area(C));
	
	return OPERATOR_FINISHED;
}


void CONSOLE_OT_zoom(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Console Zoom";
	ot->idname= "CONSOLE_OT_zoom";
	
	/* api callbacks */
	ot->exec= zoom_exec;

	/* flags */
	ot->flag= OPTYPE_REGISTER;
	
	/* properties */
	RNA_def_int(ot->srna, "delta", 0, 0, INT_MAX, "Delta", "Scale the view font.", 0, 1000);
}

