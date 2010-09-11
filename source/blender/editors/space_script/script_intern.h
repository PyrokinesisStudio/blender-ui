/**
 * $Id$
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
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2008 Blender Foundation.
 * All rights reserved.
 *
 * 
 * Contributor(s): Blender Foundation
 *
 * ***** END GPL LICENSE BLOCK *****
 */
#ifndef ED_SCRIPT_INTERN_H
#define ED_SCRIPT_INTERN_H

/* internal exports only */


/* script_header.c */
void script_header_buttons(const bContext *C, ARegion *ar);

/* script_ops.c */
void script_operatortypes(void);
void script_keymap(struct wmKeyConfig *keyconf);

/* script_edit.c */
void SCRIPT_OT_reload(struct wmOperatorType *ot);
void SCRIPT_OT_python_file_run(struct wmOperatorType *ot);

#endif /* ED_SCRIPT_INTERN_H */

