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
 * Contributor(s): Blender Foundation (2008).
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <stdlib.h>

#include "RNA_define.h"
#include "RNA_enum_types.h"

#include "rna_internal.h"

#include "DNA_curve_types.h"
#include "DNA_space_types.h"
#include "DNA_userdef_types.h"
#include "DNA_brush_types.h"

#include "WM_api.h"
#include "WM_types.h"

#include "BKE_utildefines.h"

#include "BKE_sound.h"

#ifdef RNA_RUNTIME

#include "BKE_main.h"
#include "BKE_DerivedMesh.h"
#include "BKE_depsgraph.h"
#include "DNA_object_types.h"
#include "GPU_draw.h"
#include "BKE_global.h"

#include "MEM_guardedalloc.h"
#include "MEM_CacheLimiterC-Api.h"

static void rna_userdef_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	WM_main_add_notifier(NC_WINDOW, NULL);
}

static void rna_userdef_script_autoexec_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	UserDef *userdef = (UserDef*)ptr->data;
	if (userdef->flag & USER_SCRIPT_AUTOEXEC_DISABLE)	G.f &= ~G_SCRIPT_AUTOEXEC;
	else												G.f |=  G_SCRIPT_AUTOEXEC;
}

static void rna_userdef_mipmap_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	GPU_set_mipmap(!(U.gameflags & USER_DISABLE_MIPMAP));
	rna_userdef_update(bmain, scene, ptr);
}

static void rna_userdef_select_mouse_set(PointerRNA *ptr,int value)
{
	UserDef *userdef = (UserDef*)ptr->data;

	if(value) {
		userdef->flag |= USER_LMOUSESELECT;
		userdef->flag &= ~USER_TWOBUTTONMOUSE;
	}
	else
		userdef->flag &= ~USER_LMOUSESELECT;
}

static int rna_userdef_autokeymode_get(PointerRNA *ptr)
{
	UserDef *userdef = (UserDef*)ptr->data;
	short retval = userdef->autokey_mode;
	
	if(!(userdef->autokey_mode & AUTOKEY_ON))
		retval |= AUTOKEY_ON;

	return retval;
}

static void rna_userdef_autokeymode_set(PointerRNA *ptr,int value)
{
	UserDef *userdef = (UserDef*)ptr->data;

	if(value == AUTOKEY_MODE_NORMAL) {
		userdef->autokey_mode |= (AUTOKEY_MODE_NORMAL - AUTOKEY_ON);
		userdef->autokey_mode &= ~(AUTOKEY_MODE_EDITKEYS - AUTOKEY_ON);
	}
	else if(value == AUTOKEY_MODE_EDITKEYS) {
		userdef->autokey_mode |= (AUTOKEY_MODE_EDITKEYS - AUTOKEY_ON);
		userdef->autokey_mode &= ~(AUTOKEY_MODE_NORMAL - AUTOKEY_ON);
	}
}

static void rna_userdef_timecode_style_set(PointerRNA *ptr, int value)
{
	UserDef *userdef = (UserDef*)ptr->data;
	int required_size = userdef->v2d_min_gridsize;
	
	/* set the timecode style */
	userdef->timecode_style= value;
	
	/* adjust the v2d gridsize if needed so that timecodes don't overlap 
	 * NOTE: most of these have been hand-picked to avoid overlaps while still keeping
	 * things from getting too blown out
	 */
	switch (value) {
		case USER_TIMECODE_MINIMAL:
		case USER_TIMECODE_SECONDS_ONLY:
			/* 35 is great most of the time, but not that great for full-blown */
			required_size= 35;
			break;
		case USER_TIMECODE_SMPTE_MSF:
			required_size= 50;
			break;
		case USER_TIMECODE_SMPTE_FULL:
			/* the granddaddy! */
			required_size= 65;
			break;
		case USER_TIMECODE_MILLISECONDS:
			required_size= 45;
			break;
	}
	
	if (U.v2d_min_gridsize < required_size)
		U.v2d_min_gridsize= required_size;
}

static PointerRNA rna_UserDef_view_get(PointerRNA *ptr)
{
	return rna_pointer_inherit_refine(ptr, &RNA_UserPreferencesView, ptr->data);
}

static PointerRNA rna_UserDef_edit_get(PointerRNA *ptr)
{
	return rna_pointer_inherit_refine(ptr, &RNA_UserPreferencesEdit, ptr->data);
}

static PointerRNA rna_UserDef_input_get(PointerRNA *ptr)
{
	return rna_pointer_inherit_refine(ptr, &RNA_UserPreferencesInput, ptr->data);
}

static PointerRNA rna_UserDef_filepaths_get(PointerRNA *ptr)
{
	return rna_pointer_inherit_refine(ptr, &RNA_UserPreferencesFilePaths, ptr->data);
}

static PointerRNA rna_UserDef_system_get(PointerRNA *ptr)
{
	return rna_pointer_inherit_refine(ptr, &RNA_UserPreferencesSystem, ptr->data);
}

static void rna_UserDef_audio_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	sound_init(bmain);
}

static void rna_Userdef_memcache_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	MEM_CacheLimiter_set_maximum(U.memcachelimit * 1024 * 1024);
}

static void rna_UserDef_weight_color_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	Object *ob;

	vDM_ColorBand_store((U.flag & USER_CUSTOM_RANGE) ? (&U.coba_weight):NULL);

	for(ob= bmain->object.first; ob; ob= ob->id.next) {
		if(ob->mode & OB_MODE_WEIGHT_PAINT)
			DAG_id_flush_update(&ob->id, OB_RECALC_DATA);
	}

	rna_userdef_update(bmain, scene, ptr);
}

static void rna_UserDef_viewport_lights_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	WM_main_add_notifier(NC_SPACE|ND_SPACE_VIEW3D|NS_VIEW3D_GPU, NULL);
	rna_userdef_update(bmain, scene, ptr);
}

static void rna_userdef_autosave_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	wmWindowManager *wm= bmain->wm.first;

	if(wm)
		WM_autosave_init(wm);
	rna_userdef_update(bmain, scene, ptr);
}

static bAddon *rna_userdef_addon_new(void)
{
	bAddon *bext= MEM_callocN(sizeof(bAddon), "bAddon");
	BLI_addtail(&U.addons, bext);
	return bext;
}

static void rna_userdef_addon_remove(bAddon *bext)
{
	BLI_freelinkN(&U.addons, bext);
}

static void rna_userdef_temp_update(Main *bmain, Scene *scene, PointerRNA *ptr)
{
	extern char btempdir[];
	UserDef *userdef = (UserDef*)ptr->data;
	strncpy(btempdir, userdef->tempdir, FILE_MAXDIR+FILE_MAXFILE);
}

#else

static void rna_def_userdef_theme_ui_font_style(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;
	
	static EnumPropertyItem font_kerning_style[] = {
		{0, "UNFITTED", 0, "Unfitted", "Use scaled but un-grid-fitted kerning distances"},
		{1, "DEFAULT", 0, "Default", "Use scaled and grid-fitted kerning distances"},
		{0, NULL, 0, NULL, NULL}};

	srna= RNA_def_struct(brna, "ThemeFontStyle", NULL);
	RNA_def_struct_sdna(srna, "uiFontStyle");
	RNA_def_struct_ui_text(srna, "Font Style", "Theme settings for Font");
	
	prop= RNA_def_property(srna, "points", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, 6, 48);
	RNA_def_property_ui_text(prop, "Points", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "font_kerning_style", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "kerning");
	RNA_def_property_enum_items(prop, font_kerning_style);
	RNA_def_property_ui_text(prop, "Kerning Style", "Which style to use for font kerning");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "shadow", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, 0, 5);
	RNA_def_property_ui_text(prop, "Shadow Size", "Shadow size in pixels (0, 3 and 5 supported)");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "shadow_offset_x", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "shadx");
	RNA_def_property_range(prop, -10, 10);
	RNA_def_property_ui_text(prop, "Shadow X Offset", "Shadow offset in pixels");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "shadow_offset_y", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "shady");
	RNA_def_property_range(prop, -10, 10);
	RNA_def_property_ui_text(prop, "Shadow Y Offset", "Shadow offset in pixels");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "shadowalpha", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.0f, 1.0f);
	RNA_def_property_ui_text(prop, "Shadow Alpha", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "shadowcolor", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.0f, 1.0f);
	RNA_def_property_ui_text(prop, "Shadow Brightness", "Shadow color in grey value");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}	

static void rna_def_userdef_theme_ui_style(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;
	
	rna_def_userdef_theme_ui_font_style(brna);
	
	srna= RNA_def_struct(brna, "ThemeStyle", NULL);
	RNA_def_struct_sdna(srna, "uiStyle");
	RNA_def_struct_ui_text(srna, "Style", "Theme settings for style sets");
	
	prop= RNA_def_property(srna, "panelzoom", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.5, 2.0);
	RNA_def_property_ui_text(prop, "Panel Zoom", "Default zoom level for panel areas");
	
	prop= RNA_def_property(srna, "panel_title", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "paneltitle");
	RNA_def_property_struct_type(prop, "ThemeFontStyle");
	RNA_def_property_ui_text(prop, "Panel Font", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "group_label", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "grouplabel");
	RNA_def_property_struct_type(prop, "ThemeFontStyle");
	RNA_def_property_ui_text(prop, "Group Label Font", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "widget_label", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "widgetlabel");
	RNA_def_property_struct_type(prop, "ThemeFontStyle");
	RNA_def_property_ui_text(prop, "Widget Label Font", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "widget", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "widget");
	RNA_def_property_struct_type(prop, "ThemeFontStyle");
	RNA_def_property_ui_text(prop, "Widget Font", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
}	

static void rna_def_userdef_theme_ui_wcol(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;
	
	srna= RNA_def_struct(brna, "ThemeWidgetColors", NULL);
	RNA_def_struct_sdna(srna, "uiWidgetColors");
	RNA_def_struct_ui_text(srna, "Theme Widget Color Set", "Theme settings for widget color sets");
		
	prop= RNA_def_property(srna, "outline", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Outline", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "inner", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Inner", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "inner_sel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Inner Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "item", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Item", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "text", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Text", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "text_sel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Text Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "show_shaded", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "shaded", 1);
	RNA_def_property_ui_text(prop, "Shaded", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "shadetop", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, -100, 100);
	RNA_def_property_ui_text(prop, "Shade Top", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "shadedown", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, -100, 100);
	RNA_def_property_ui_text(prop, "Shade Down", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_ui_wcol_state(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;
	
	srna= RNA_def_struct(brna, "ThemeWidgetStateColors", NULL);
	RNA_def_struct_sdna(srna, "uiWidgetStateColors");
	RNA_def_struct_ui_text(srna, "Theme Widget State Color", "Theme settings for widget state colors");
		
	prop= RNA_def_property(srna, "inner_anim", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Animated", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "inner_anim_sel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Animated Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "inner_key", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Keyframe", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "inner_key_sel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Keyframe Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "inner_driven", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Driven", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "inner_driven_sel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Driven Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "blend", PROP_FLOAT, PROP_FACTOR);
	RNA_def_property_ui_text(prop, "Blend", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_ui(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	rna_def_userdef_theme_ui_wcol(brna);
	rna_def_userdef_theme_ui_wcol_state(brna);
	
	srna= RNA_def_struct(brna, "ThemeUserInterface", NULL);
	RNA_def_struct_sdna(srna, "ThemeUI");
	RNA_def_struct_ui_text(srna, "Theme User Interface", "Theme settings for user interface elements");

	prop= RNA_def_property(srna, "wcol_regular", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_regular");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Regular Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_tool", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_tool");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Tool Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_radio", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_radio");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Radio Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_text", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_text");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Text Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_option", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_option");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Option Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "wcol_toggle", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_toggle");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Toggle Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_num", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_num");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Number Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_numslider", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_numslider");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Slider Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_box", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_box");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Box Backdrop Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_menu", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_menu");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Menu Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_pulldown", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_pulldown");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Pulldown Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_menu_back", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_menu_back");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Menu Backdrop Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_menu_item", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_menu_item");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Menu Item Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "wcol_scroll", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_scroll");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Scroll Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "wcol_progress", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_progress");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "Progress Bar Widget Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "wcol_list_item", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_list_item");
	RNA_def_property_struct_type(prop, "ThemeWidgetColors");
	RNA_def_property_ui_text(prop, "List Item Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "wcol_state", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "wcol_state");
	RNA_def_property_struct_type(prop, "ThemeWidgetStateColors");
	RNA_def_property_ui_text(prop, "State Colors", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "icon_file", PROP_STRING, PROP_FILEPATH);
	RNA_def_property_string_sdna(prop, NULL, "iconfile");
	RNA_def_property_ui_text(prop, "Icon File", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_spaces_main(StructRNA *srna, int spacetype)
{
	PropertyRNA *prop;

	/* window */
	prop= RNA_def_property(srna, "back", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Window Background", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "title", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Title", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "text", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Text", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "text_hi", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Text Highlight", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	/* header */
	prop= RNA_def_property(srna, "header", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Header", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "header_text", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Header Text", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "header_text_hi", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Header Text Highlight", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	/* buttons */
//	if(! ELEM(spacetype, SPACE_BUTS, SPACE_OUTLINER)) {
	prop= RNA_def_property(srna, "button", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Region Background", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "button_title", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Region Text Titles", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "button_text", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Region Text", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "button_text_hi", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Region Text Highlight", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
//	}
	
	/* list/channels */
	if(ELEM5(spacetype, SPACE_IPO, SPACE_ACTION, SPACE_NLA, SPACE_NODE, SPACE_FILE)) {
		prop= RNA_def_property(srna, "list", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Source List", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");
		
		prop= RNA_def_property(srna, "list_title", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Source List Title", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");
		
		prop= RNA_def_property(srna, "list_text", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Source List Text", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");
		
		prop= RNA_def_property(srna, "list_text_hi", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Source List Text Highlight", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");
	}	
}

static void rna_def_userdef_theme_spaces_vertex(StructRNA *srna)
{
	PropertyRNA *prop;

	prop= RNA_def_property(srna, "vertex", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Vertex", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "vertex_select", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Vertex Select", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "vertex_size", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, 1, 10);
	RNA_def_property_ui_text(prop, "Vertex Size", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_spaces_edge(StructRNA *srna)
{
	PropertyRNA *prop;

	prop= RNA_def_property(srna, "edge_select", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Edge Select", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "edge_seam", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Edge Seam", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "edge_sharp", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Edge Sharp", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "edge_crease", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Edge Crease", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "edge_facesel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Edge UV Face Select", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_spaces_face(StructRNA *srna)
{
	PropertyRNA *prop;

	prop= RNA_def_property(srna, "face", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Face", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "face_select", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Face Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "face_dot", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Face Dot Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "facedot_size", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, 1, 10);
	RNA_def_property_ui_text(prop, "Face Dot Size", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_spaces_curves(StructRNA *srna, short incl_nurbs)
{
	PropertyRNA *prop;
	
	if (incl_nurbs) {
		prop= RNA_def_property(srna, "nurb_uline", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_float_sdna(prop, NULL, "nurb_uline");
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Nurb U-lines", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");

		prop= RNA_def_property(srna, "nurb_vline", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_float_sdna(prop, NULL, "nurb_vline");
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Nurb V-lines", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");

		prop= RNA_def_property(srna, "nurb_sel_uline", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_float_sdna(prop, NULL, "nurb_sel_uline");
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Nurb active U-lines", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");

		prop= RNA_def_property(srna, "nurb_sel_vline", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_float_sdna(prop, NULL, "nurb_sel_vline");
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Nurb active V-lines", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");

		prop= RNA_def_property(srna, "act_spline", PROP_FLOAT, PROP_COLOR);
		RNA_def_property_float_sdna(prop, NULL, "act_spline");
		RNA_def_property_array(prop, 3);
		RNA_def_property_ui_text(prop, "Active spline", "");
		RNA_def_property_update(prop, 0, "rna_userdef_update");
	}

	prop= RNA_def_property(srna, "handle_free", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_free");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Free handle color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_auto", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_auto");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Auto handle color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_vect", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_vect");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Vector handle color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_align", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_align");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Align handle color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_sel_free", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_sel_free");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Free handle selected color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_sel_auto", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_sel_auto");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Auto handle selected color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_sel_vect", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_sel_vect");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Vector handle selected color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_sel_align", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "handle_sel_align");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Align handle selected color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "lastsel_point", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "lastsel_point");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Last selected point", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_view3d(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_view3d */

	srna= RNA_def_struct(brna, "ThemeView3D", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme 3D View", "Theme settings for the 3D View");

	rna_def_userdef_theme_spaces_main(srna, SPACE_VIEW3D);

	prop= RNA_def_property(srna, "grid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Grid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "panel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Panel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "wire", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Wire", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "lamp", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Lamp", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "object_selected", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "select");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Object Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "object_active", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "active");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Active Object", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "object_grouped", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "group");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Object Grouped", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "object_grouped_active", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "group_active");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Object Grouped Active", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "transform", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Transform", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	rna_def_userdef_theme_spaces_vertex(srna);
	rna_def_userdef_theme_spaces_edge(srna);
	rna_def_userdef_theme_spaces_face(srna);
	rna_def_userdef_theme_spaces_curves(srna, 1);

	prop= RNA_def_property(srna, "editmesh_active", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Active Vert/Edge/Face", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "normal", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Face Normal", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "vertex_normal", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Vertex Normal", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "bone_solid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Bone Solid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "bone_pose", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Bone Pose", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "frame_current", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "cframe");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Current Frame", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_graph(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_graph */

	srna= RNA_def_struct(brna, "ThemeGraphEditor", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Graph Editor", "Theme settings for the graph editor");

	rna_def_userdef_theme_spaces_main(srna, SPACE_IPO);

	prop= RNA_def_property(srna, "grid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Grid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "panel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Panel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "window_sliders", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade1");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Window Sliders", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "channels_region", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade2");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Channels Region", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	rna_def_userdef_theme_spaces_vertex(srna);
	rna_def_userdef_theme_spaces_curves(srna, 0);

	prop= RNA_def_property(srna, "frame_current", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "cframe");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Current Frame", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_vertex", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Handle Vertex", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_vertex_select", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Handle Vertex Select", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "handle_vertex_size", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, 0, 255);
	RNA_def_property_ui_text(prop, "Handle Vertex Size", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "channel_group", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "group");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Channel Group", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "active_channels_group", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "group_active");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Active Channel Group", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "dopesheet_channel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "ds_channel");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "DopeSheet Channel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "dopesheet_subchannel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "ds_subchannel");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "DopeSheet Sub-Channel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_file(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_file  */

	srna= RNA_def_struct(brna, "ThemeFileBrowser", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme File Browser", "Theme settings for the File Browser");

	rna_def_userdef_theme_spaces_main(srna, SPACE_FILE);

	prop= RNA_def_property(srna, "selected_file", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "hilite");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Selected File", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "tiles", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "panel");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Tiles", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "scrollbar", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade1");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Scrollbar", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "scroll_handle", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade2");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Scroll Handle", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "active_file", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "active");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Active File", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "active_file_text", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "grid");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Active File Text", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_outliner(BlenderRNA *brna)
{
	StructRNA *srna;

	/* space_outliner */

	srna= RNA_def_struct(brna, "ThemeOutliner", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Outliner", "Theme settings for the Outliner");

	rna_def_userdef_theme_spaces_main(srna, SPACE_OUTLINER);
}

static void rna_def_userdef_theme_space_userpref(BlenderRNA *brna)
{
	StructRNA *srna;

	/* space_userpref */

	srna= RNA_def_struct(brna, "ThemeUserPreferences", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme User Preferences", "Theme settings for the User Preferences");

	rna_def_userdef_theme_spaces_main(srna, SPACE_USERPREF);
}

static void rna_def_userdef_theme_space_console(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_console */

	srna= RNA_def_struct(brna, "ThemeConsole", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Console", "Theme settings for the Console");
	
	rna_def_userdef_theme_spaces_main(srna, SPACE_CONSOLE);
	
	prop= RNA_def_property(srna, "line_output", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "console_output");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Line Output", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "line_input", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "console_input");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Line Input", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "line_info", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "console_info");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Line Info", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "line_error", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "console_error");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Line Error", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "cursor", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "console_cursor");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Cursor", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_info(BlenderRNA *brna)
{
	StructRNA *srna;

	/* space_info */

	srna= RNA_def_struct(brna, "ThemeInfo", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Info", "Theme settings for Info");

	rna_def_userdef_theme_spaces_main(srna, SPACE_INFO);
}


static void rna_def_userdef_theme_space_text(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_text */

	srna= RNA_def_struct(brna, "ThemeTextEditor", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Text Editor", "Theme settings for the Text Editor");

	rna_def_userdef_theme_spaces_main(srna, SPACE_TEXT);

	prop= RNA_def_property(srna, "line_numbers_background", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "grid");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Line Numbers Background", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "scroll_bar", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade1");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Scroll Bar", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "selected_text", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade2");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Selected Text", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "cursor", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "hilite");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Cursor", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "syntax_builtin", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxb");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Syntax Built-in", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "syntax_special", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxv");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Syntax Special", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "syntax_comment", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxc");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Syntax Comment", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "syntax_string", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxl");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Syntax String", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "syntax_numbers", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxn");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Syntax Numbers", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_node(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_node */

	srna= RNA_def_struct(brna, "ThemeNodeEditor", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Node Editor", "Theme settings for the Node Editor");

	rna_def_userdef_theme_spaces_main(srna, SPACE_NODE);

	prop= RNA_def_property(srna, "wire", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "wire");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Wires", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "wire_select", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "edge_select");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Wire Select", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "selected_text", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade2");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Selected Text", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "node_backdrop", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxl");
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Node Backdrop", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "in_out_node", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxn");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "In/Out Node", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "converter_node", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxv");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Converter Node", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "operator_node", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxb");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Operator Node", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "group_node", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "syntaxc");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Group Node", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_logic(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;
	
	/* space_buts */
	
	srna= RNA_def_struct(brna, "ThemeLogicEditor", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Logic Editor", "Theme settings for the Logic Editor");
	
	rna_def_userdef_theme_spaces_main(srna, SPACE_LOGIC);
	
	prop= RNA_def_property(srna, "panel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Panel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}


static void rna_def_userdef_theme_space_buts(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_buts */

	srna= RNA_def_struct(brna, "ThemeProperties", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Properties", "Theme settings for the Properties");

	rna_def_userdef_theme_spaces_main(srna, SPACE_BUTS);

	prop= RNA_def_property(srna, "panel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Panel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_time(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_time */

	srna= RNA_def_struct(brna, "ThemeTimeline", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Timeline", "Theme settings for the Timeline");

	rna_def_userdef_theme_spaces_main(srna, SPACE_TIME);

	prop= RNA_def_property(srna, "grid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Grid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "frame_current", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "cframe");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Current Frame", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_sound(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_sound */

	srna= RNA_def_struct(brna, "ThemeAudioWindow", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Audio Window", "Theme settings for the Audio Window");

	rna_def_userdef_theme_spaces_main(srna, SPACE_SOUND);

	prop= RNA_def_property(srna, "grid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Grid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "window_sliders", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade1");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Window Sliders", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "frame_current", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "cframe");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Current Frame", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_image(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_image */

	srna= RNA_def_struct(brna, "ThemeImageEditor", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Image Editor", "Theme settings for the Image Editor");

	rna_def_userdef_theme_spaces_main(srna, SPACE_IMAGE);
	rna_def_userdef_theme_spaces_vertex(srna);
	rna_def_userdef_theme_spaces_face(srna);

	prop= RNA_def_property(srna, "editmesh_active", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Active Vert/Edge/Face", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "scope_back", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "preview_back");
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Scope region background color", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_seq(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_seq */

	srna= RNA_def_struct(brna, "ThemeSequenceEditor", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme Sequence Editor", "Theme settings for the Sequence Editor");

	rna_def_userdef_theme_spaces_main(srna, SPACE_IMAGE);

	prop= RNA_def_property(srna, "grid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Grid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "window_sliders", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade1");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Window Sliders", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "movie_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "movie");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Movie Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "image_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "image");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Image Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "scene_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "scene");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Scene Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "audio_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "audio");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Audio Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "effect_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "effect");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Effect Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "plugin_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "plugin");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Plugin Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "transition_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "transition");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Transition Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "meta_strip", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "meta");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Meta Strip", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "frame_current", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "cframe");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Current Frame", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "keyframe", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "vertex_select");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Keyframe", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "draw_action", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "bone_pose");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Draw Action", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_action(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_action */

	srna= RNA_def_struct(brna, "ThemeDopeSheet", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme DopeSheet", "Theme settings for the DopeSheet");

	rna_def_userdef_theme_spaces_main(srna, SPACE_ACTION);

	prop= RNA_def_property(srna, "grid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Grid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "value_sliders", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "face");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Value Sliders", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "view_sliders", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade1");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "View Sliders", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "channels", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade2");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Channels", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "channels_selected", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "hilite");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Channels Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "channel_group", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "group");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Channel Group", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "active_channels_group", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "group_active");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Active Channel Group", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "long_key", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "strip");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Long Key", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "long_key_selected", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "strip_select");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Long Key Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "frame_current", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "cframe");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Current Frame", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "dopesheet_channel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "ds_channel");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "DopeSheet Channel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "dopesheet_subchannel", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "ds_subchannel");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "DopeSheet Sub-Channel", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_space_nla(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	/* space_nla */

	srna= RNA_def_struct(brna, "ThemeNLAEditor", NULL);
	RNA_def_struct_sdna(srna, "ThemeSpace");
	RNA_def_struct_ui_text(srna, "Theme NLA Editor", "Theme settings for the NLA Editor");

	rna_def_userdef_theme_spaces_main(srna, SPACE_NLA);

	prop= RNA_def_property(srna, "grid", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Grid", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "view_sliders", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade1");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "View Sliders", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "bars", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "shade2");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Bars", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "bars_selected", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "hilite");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Bars Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "strips", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "strip");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Strips", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "strips_selected", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "strip_select");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Strips Selected", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "frame_current", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "cframe");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Current Frame", "");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_theme_colorset(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	srna= RNA_def_struct(brna, "ThemeBoneColorSet", NULL);
	RNA_def_struct_sdna(srna, "ThemeWireColor");
	RNA_def_struct_ui_text(srna, "Theme Bone Color Set", "Theme settings for bone color sets");

	prop= RNA_def_property(srna, "normal", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "solid");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Normal", "Color used for the surface of bones");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "select", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "select");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Select", "Color used for selected bones");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "active", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Active", "Color used for active bones");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "show_colored_constraints", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TH_WIRECOLOR_CONSTCOLS);
	RNA_def_property_ui_text(prop, "Colored Constraints", "Allow the use of colors indicating constraints/keyed status");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_themes(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;
	
	static EnumPropertyItem active_theme_area[] = {
		{0, "USER_INTERFACE", ICON_UI, "User Interface", ""},
		{1, "VIEW_3D", ICON_VIEW3D, "3D View", ""},
		{2, "TIMELINE", ICON_TIME, "Timeline", ""},
		{3, "GRAPH_EDITOR", ICON_IPO, "Graph Editor", ""},
		{4, "DOPESHEET_EDITOR", ICON_ACTION, "Dopesheet", ""},
		{5, "NLA_EDITOR", ICON_NLA, "NLA Editor", ""},
		{6, "IMAGE_EDITOR", ICON_IMAGE_COL, "UV/Image Editor", ""},
		{7, "SEQUENCE_EDITOR", ICON_SEQUENCE, "Video Sequence Editor", ""},
		{8, "TEXT_EDITOR", ICON_TEXT, "Text Editor", ""},
		{9, "NODE_EDITOR", ICON_NODETREE, "Node Editor", ""},
		{10, "LOGIC_EDITOR", ICON_LOGIC, "Logic Editor", ""},
		{11, "PROPERTIES", ICON_BUTS, "Properties", ""},
		{12, "OUTLINER", ICON_OOPS, "Outliner", ""},
		{14, "USER_PREFERENCES", ICON_PREFERENCES, "User Preferences", ""},
		{15, "INFO", ICON_INFO, "Info", ""},
		{16, "FILE_BROWSER", ICON_FILESEL, "File Browser", ""},
		{17, "CONSOLE", ICON_CONSOLE, "Console", ""},
		{0, NULL, 0, NULL, NULL}};

	srna= RNA_def_struct(brna, "Theme", NULL);
	RNA_def_struct_sdna(srna, "bTheme");
	RNA_def_struct_ui_text(srna, "Theme", "Theme settings defining draw style and colors in the user interface");

	prop= RNA_def_property(srna, "name", PROP_STRING, PROP_NONE);
	RNA_def_property_ui_text(prop, "Name", "Name of the theme");
	RNA_def_struct_name_property(srna, prop);

	prop= RNA_def_property(srna, "theme_area", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "active_theme_area");
	RNA_def_property_enum_items(prop, active_theme_area);
	RNA_def_property_ui_text(prop, "Active Theme Area", "");

	prop= RNA_def_property(srna, "user_interface", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tui");
	RNA_def_property_struct_type(prop, "ThemeUserInterface");
	RNA_def_property_ui_text(prop, "User Interface", "");

	prop= RNA_def_property(srna, "view_3d", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tv3d");
	RNA_def_property_struct_type(prop, "ThemeView3D");
	RNA_def_property_ui_text(prop, "3D View", "");

	prop= RNA_def_property(srna, "graph_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tipo");
	RNA_def_property_struct_type(prop, "ThemeGraphEditor");
	RNA_def_property_ui_text(prop, "Graph Editor", "");

	prop= RNA_def_property(srna, "file_browser", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tfile");
	RNA_def_property_struct_type(prop, "ThemeFileBrowser");
	RNA_def_property_ui_text(prop, "File Browser", "");

	prop= RNA_def_property(srna, "nla_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tnla");
	RNA_def_property_struct_type(prop, "ThemeNLAEditor");
	RNA_def_property_ui_text(prop, "NLA Editor", "");

	prop= RNA_def_property(srna, "dopesheet_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tact");
	RNA_def_property_struct_type(prop, "ThemeDopeSheet");
	RNA_def_property_ui_text(prop, "DopeSheet", "");

	prop= RNA_def_property(srna, "image_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tima");
	RNA_def_property_struct_type(prop, "ThemeImageEditor");
	RNA_def_property_ui_text(prop, "Image Editor", "");

	prop= RNA_def_property(srna, "sequence_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tseq");
	RNA_def_property_struct_type(prop, "ThemeSequenceEditor");
	RNA_def_property_ui_text(prop, "Sequence Editor", "");

	prop= RNA_def_property(srna, "properties", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tbuts");
	RNA_def_property_struct_type(prop, "ThemeProperties");
	RNA_def_property_ui_text(prop, "Properties", "");

	prop= RNA_def_property(srna, "text_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "text");
	RNA_def_property_struct_type(prop, "ThemeTextEditor");
	RNA_def_property_ui_text(prop, "Text Editor", "");

	prop= RNA_def_property(srna, "timeline", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "ttime");
	RNA_def_property_struct_type(prop, "ThemeTimeline");
	RNA_def_property_ui_text(prop, "Timeline", "");

	prop= RNA_def_property(srna, "node_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tnode");
	RNA_def_property_struct_type(prop, "ThemeNodeEditor");
	RNA_def_property_ui_text(prop, "Node Editor", "");

	prop= RNA_def_property(srna, "logic_editor", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tlogic");
	RNA_def_property_struct_type(prop, "ThemeLogicEditor");
	RNA_def_property_ui_text(prop, "Logic Editor", "");
	
	prop= RNA_def_property(srna, "outliner", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "toops");
	RNA_def_property_struct_type(prop, "ThemeOutliner");
	RNA_def_property_ui_text(prop, "Outliner", "");

	prop= RNA_def_property(srna, "info", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tinfo");
	RNA_def_property_struct_type(prop, "ThemeInfo");
	RNA_def_property_ui_text(prop, "Info", "");

	prop= RNA_def_property(srna, "user_preferences", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tuserpref");
	RNA_def_property_struct_type(prop, "ThemeUserPreferences");
	RNA_def_property_ui_text(prop, "User Preferences", "");
	
	prop= RNA_def_property(srna, "console", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "tconsole");
	RNA_def_property_struct_type(prop, "ThemeConsole");
	RNA_def_property_ui_text(prop, "Console", "");

	prop= RNA_def_property(srna, "bone_color_sets", PROP_COLLECTION, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_collection_sdna(prop, NULL, "tarm", "");
	RNA_def_property_struct_type(prop, "ThemeBoneColorSet");
	RNA_def_property_ui_text(prop, "Bone Color Sets", "");
}

static void rna_def_userdef_addon(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	srna= RNA_def_struct(brna, "Addon", NULL);
	RNA_def_struct_sdna(srna, "bAddon");
	RNA_def_struct_ui_text(srna, "Addon", "Python addons to be loaded automatically");

	prop= RNA_def_property(srna, "module", PROP_STRING, PROP_NONE);
	RNA_def_property_ui_text(prop, "Module", "Module name");
	RNA_def_struct_name_property(srna, prop);
}


static void rna_def_userdef_dothemes(BlenderRNA *brna)
{
	
	rna_def_userdef_theme_ui_style(brna);
	rna_def_userdef_theme_ui(brna);
	
	rna_def_userdef_theme_space_view3d(brna);
	rna_def_userdef_theme_space_graph(brna);
	rna_def_userdef_theme_space_file(brna);
	rna_def_userdef_theme_space_nla(brna);
	rna_def_userdef_theme_space_action(brna);
	rna_def_userdef_theme_space_image(brna);
	rna_def_userdef_theme_space_seq(brna);
	rna_def_userdef_theme_space_buts(brna);
	rna_def_userdef_theme_space_text(brna);
	rna_def_userdef_theme_space_time(brna);
	rna_def_userdef_theme_space_node(brna);
	rna_def_userdef_theme_space_outliner(brna);
	rna_def_userdef_theme_space_info(brna);
	rna_def_userdef_theme_space_userpref(brna);
	rna_def_userdef_theme_space_console(brna);
	rna_def_userdef_theme_space_sound(brna);
	rna_def_userdef_theme_space_logic(brna);
	rna_def_userdef_theme_colorset(brna);
	rna_def_userdef_themes(brna);
}

static void rna_def_userdef_solidlight(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	srna= RNA_def_struct(brna, "UserSolidLight", NULL);
	RNA_def_struct_sdna(srna, "SolidLight");
	RNA_def_struct_ui_text(srna, "Solid Light", "Light used for OpenGL lighting in solid draw mode");
	
	prop= RNA_def_property(srna, "use", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", 1);
	RNA_def_property_ui_text(prop, "Enabled", "Enable this OpenGL light in solid draw mode");
	RNA_def_property_update(prop, 0, "rna_UserDef_viewport_lights_update");

	prop= RNA_def_property(srna, "direction", PROP_FLOAT, PROP_DIRECTION);
	RNA_def_property_float_sdna(prop, NULL, "vec");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Direction", "The direction that the OpenGL light is shining");
	RNA_def_property_update(prop, 0, "rna_UserDef_viewport_lights_update");

	prop= RNA_def_property(srna, "diffuse_color", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "col");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Diffuse Color", "The diffuse color of the OpenGL light");
	RNA_def_property_update(prop, 0, "rna_UserDef_viewport_lights_update");

	prop= RNA_def_property(srna, "specular_color", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "spec");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Specular Color", "The color of the lights specular highlight");
	RNA_def_property_update(prop, 0, "rna_UserDef_viewport_lights_update");
}

static void rna_def_userdef_view(BlenderRNA *brna)
{
	static EnumPropertyItem timecode_styles[] = {
		{USER_TIMECODE_MINIMAL, "MINIMAL", 0, "Minimal Info", "Most compact representation. Uses '+' as separator for sub-second frame numbers, with left and right truncation of the timecode as necessary"},
		{USER_TIMECODE_SMPTE_FULL, "SMPTE", 0, "SMPTE (Full)", "Full SMPTE timecode. Format is HH:MM:SS:FF"},
		{USER_TIMECODE_SMPTE_MSF, "SMPTE_COMPACT", 0, "SMPTE (Compact)", "SMPTE timecode showing minutes, seconds, and frames only. Hours are also shown if necessary, but not by default"},
		{USER_TIMECODE_MILLISECONDS, "MILLISECONDS", 0, "Compact with Milliseconds", "Similar to SMPTE (Compact), except that instead of frames, milliseconds are shown instead"},
		{USER_TIMECODE_SECONDS_ONLY, "SECONDS_ONLY", 0, "Only Seconds", "Direct conversion of frame numbers to seconds"},
		{0, NULL, 0, NULL, NULL}};
	
	PropertyRNA *prop;
	StructRNA *srna;
	
	srna= RNA_def_struct(brna, "UserPreferencesView", NULL);
	RNA_def_struct_sdna(srna, "UserDef");
	RNA_def_struct_nested(brna, srna, "UserPreferences");
	RNA_def_struct_ui_text(srna, "View & Controls", "Preferences related to viewing data");

	/* View  */

	/* display */
	prop= RNA_def_property(srna, "show_tooltips", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_TOOLTIPS);
	RNA_def_property_ui_text(prop, "Tooltips", "Display tooltips");

	prop= RNA_def_property(srna, "show_object_info", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_DRAWVIEWINFO);
	RNA_def_property_ui_text(prop, "Display Object Info", "Display objects name and frame number in 3D view");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "use_global_scene", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_SCENEGLOBAL);
	RNA_def_property_ui_text(prop, "Global Scene", "Forces the current Scene to be displayed in all Screens");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "show_large_cursors", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "curssize", 0);
	RNA_def_property_ui_text(prop, "Large Cursors", "Use large mouse cursors when available");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "show_view_name", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_SHOW_VIEWPORTNAME);
	RNA_def_property_ui_text(prop, "Show View Name", "Show the name of the view's direction in each 3D View");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
    
	prop= RNA_def_property(srna, "show_splash", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "uiflag", USER_SPLASH_DISABLE);
	RNA_def_property_ui_text(prop, "Show Splash", "Display splash screen on startup");

	prop= RNA_def_property(srna, "show_playback_fps", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_SHOW_FPS);
	RNA_def_property_ui_text(prop, "Show Playback FPS", "Show the frames per second screen refresh rate, while animation is played back");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	/* menus */
	prop= RNA_def_property(srna, "use_mouse_over_open", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_MENUOPENAUTO);
	RNA_def_property_ui_text(prop, "Open On Mouse Over", "Open menu buttons and pulldowns automatically when the mouse is hovering");
	
	prop= RNA_def_property(srna, "open_toplevel_delay", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "menuthreshold1");
	RNA_def_property_range(prop, 1, 40);
	RNA_def_property_ui_text(prop, "Top Level Menu Open Delay", "Time delay in 1/10 seconds before automatically opening top level menus");

	prop= RNA_def_property(srna, "open_sublevel_delay", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "menuthreshold2");
	RNA_def_property_range(prop, 1, 40);
	RNA_def_property_ui_text(prop, "Sub Level Menu Open Delay", "Time delay in 1/10 seconds before automatically opening sub level menus");

	/* Toolbox click-hold delay */
	prop= RNA_def_property(srna, "open_left_mouse_delay", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "tb_leftmouse");
	RNA_def_property_range(prop, 1, 40);
	RNA_def_property_ui_text(prop, "Hold LMB Open Toolbox Delay", "Time in 1/10 seconds to hold the Left Mouse Button before opening the toolbox");

	prop= RNA_def_property(srna, "open_right_mouse_delay", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "tb_rightmouse");
	RNA_def_property_range(prop, 1, 40);
	RNA_def_property_ui_text(prop, "Hold RMB Open Toolbox Delay", "Time in 1/10 seconds to hold the Right Mouse Button before opening the toolbox");

	prop= RNA_def_property(srna, "show_column_layout", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_PLAINMENUS);
	RNA_def_property_ui_text(prop, "Toolbox Column Layout", "Use a column layout for toolbox");

	prop= RNA_def_property(srna, "use_directional_menus", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "uiflag", USER_MENUFIXEDORDER);
	RNA_def_property_ui_text(prop, "Contents Follow Opening Direction", "Otherwise menus, etc will always be top to bottom, left to right, no matter opening direction");

	prop= RNA_def_property(srna, "use_global_pivot", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_LOCKAROUND);
	RNA_def_property_ui_text(prop, "Global Pivot", "Lock the same rotation/scaling pivot in all 3D Views");

	prop= RNA_def_property(srna, "use_mouse_auto_depth", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_ORBIT_ZBUF);
	RNA_def_property_ui_text(prop, "Auto Depth", "Use the depth under the mouse to improve view pan/rotate/zoom functionality");

	/* view zoom */
	prop= RNA_def_property(srna, "use_zoom_to_mouse", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_ZOOM_TO_MOUSEPOS);
	RNA_def_property_ui_text(prop, "Zoom To Mouse Position", "Zoom in towards the mouse pointer's position in the 3D view, rather than the 2D window center");

	/* view rotation */
	prop= RNA_def_property(srna, "use_auto_perspective", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_AUTOPERSP);
	RNA_def_property_ui_text(prop, "Auto Perspective", "Automatically switch between orthographic and perspective when changing from top/front/side views");

	prop= RNA_def_property(srna, "use_rotate_around_active", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_ORBIT_SELECTION);
	RNA_def_property_ui_text(prop, "Rotate Around Selection", "Use selection as the pivot point");
	
	/* mini axis */
	prop= RNA_def_property(srna, "show_mini_axis", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_SHOW_ROTVIEWICON);
	RNA_def_property_ui_text(prop, "Show Mini Axis", "Show a small rotating 3D axis in the bottom left corner of the 3D View");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "mini_axis_size", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "rvisize");
	RNA_def_property_range(prop, 10, 64);
	RNA_def_property_ui_text(prop, "Mini Axis Size", "The axis icon's size");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "mini_axis_brightness", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "rvibright");
	RNA_def_property_range(prop, 0, 10);
	RNA_def_property_ui_text(prop, "Mini Axis Brightness", "The brightness of the icon");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "smooth_view", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "smooth_viewtx");
	RNA_def_property_range(prop, 0, 1000);
	RNA_def_property_ui_text(prop, "Smooth View", "The time to animate the view in milliseconds, zero to disable");

	prop= RNA_def_property(srna, "rotation_angle", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "pad_rot_angle");
	RNA_def_property_range(prop, 0, 90);
	RNA_def_property_ui_text(prop, "Rotation Angle", "The rotation step for numerical pad keys (2 4 6 8)");

	/* 3D transform widget */
	prop= RNA_def_property(srna, "show_manipulator", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "tw_flag", 1);
	RNA_def_property_ui_text(prop, "Manipulator", "Use 3D transform manipulator");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "manipulator_size", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "tw_size");
	RNA_def_property_range(prop, 2, 40);
	RNA_def_property_ui_text(prop, "Manipulator Size", "Diameter of widget, in 10 pixel units");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "manipulator_handle_size", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "tw_handlesize");
	RNA_def_property_range(prop, 2, 40);
	RNA_def_property_ui_text(prop, "Manipulator Handle Size", "Size of widget handles as percentage of widget radius");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "manipulator_hotspot", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "tw_hotspot");
	RNA_def_property_range(prop, 4, 40);
	RNA_def_property_ui_text(prop, "Manipulator Hotspot", "Hotspot in pixels for clicking widget handles");

	prop= RNA_def_property(srna, "object_origin_size", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "obcenter_dia");
	RNA_def_property_range(prop, 4, 10);
	RNA_def_property_ui_text(prop, "Object Origin Size", "Diameter in Pixels for Object/Lamp origin display");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	/* View2D Grid Displays */
	prop= RNA_def_property(srna, "view2d_grid_spacing_min", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "v2d_min_gridsize");
	RNA_def_property_range(prop, 1, 500); // XXX: perhaps the lower range should only go down to 5?
	RNA_def_property_ui_text(prop, "2D View Minimum Grid Spacing", "Minimum number of pixels between each gridline in 2D Viewports");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
		// TODO: add a setter for this, so that we can bump up the minimum size as necessary...
	prop= RNA_def_property(srna, "timecode_style", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, timecode_styles);
	RNA_def_property_enum_sdna(prop, NULL, "timecode_style");
	RNA_def_property_enum_funcs(prop, NULL, "rna_userdef_timecode_style_set", NULL);
	RNA_def_property_ui_text(prop, "TimeCode Style", "Format of Time Codes displayed when not displaying timing in terms of frames");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
}

static void rna_def_userdef_edit(BlenderRNA *brna)
{
	PropertyRNA *prop;
	StructRNA *srna;

	static EnumPropertyItem auto_key_modes[] = {
		{AUTOKEY_MODE_NORMAL, "ADD_REPLACE_KEYS", 0, "Add/Replace", ""},
		{AUTOKEY_MODE_EDITKEYS, "REPLACE_KEYS", 0, "Replace", ""},
		{0, NULL, 0, NULL, NULL}};
		
	static const EnumPropertyItem material_link_items[]= {
		{0, "OBDATA", 0, "ObData", "Toggle whether the material is linked to object data or the object block"},
		{USER_MAT_ON_OB, "OBJECT", 0, "Object", "Toggle whether the material is linked to object data or the object block"},
		{0, NULL, 0, NULL, NULL}};
		
	static const EnumPropertyItem object_align_items[]= {
		{0, "WORLD", 0, "World", "Align newly added objects to the world coordinates"},
		{USER_ADD_VIEWALIGNED, "VIEW", 0, "View", "Align newly added objects facing the active 3D View direction"},
		{0, NULL, 0, NULL, NULL}};

	srna= RNA_def_struct(brna, "UserPreferencesEdit", NULL);
	RNA_def_struct_sdna(srna, "UserDef");
	RNA_def_struct_nested(brna, srna, "UserPreferences");
	RNA_def_struct_ui_text(srna, "Edit Methods", "Settings for interacting with Blender data");
	
	/* Edit Methods */
	
	prop= RNA_def_property(srna, "material_link", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_bitflag_sdna(prop, NULL, "flag");
	RNA_def_property_enum_items(prop, material_link_items);
	RNA_def_property_ui_text(prop, "Material Link To", "Toggle whether the material is linked to object data or the object block");
	
	prop= RNA_def_property(srna, "object_align", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_bitflag_sdna(prop, NULL, "flag");
	RNA_def_property_enum_items(prop, object_align_items);
	RNA_def_property_ui_text(prop, "Align Object To", "When adding objects from a 3D View menu, either align them to that view's direction or the world coordinates");

	prop= RNA_def_property(srna, "use_enter_edit_mode", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_ADD_EDITMODE);
	RNA_def_property_ui_text(prop, "Enter Edit Mode", "Enter Edit Mode automatically after adding a new object");

	prop= RNA_def_property(srna, "use_drag_immediately", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_RELEASECONFIRM);
	RNA_def_property_ui_text(prop, "Release confirm", "Moving things with a mouse drag confirms when releasing the button");
	
	/* Undo */
	prop= RNA_def_property(srna, "undo_steps", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "undosteps");
	RNA_def_property_range(prop, 0, 64);
	RNA_def_property_ui_text(prop, "Undo Steps", "Number of undo steps available (smaller values conserve memory)");

	prop= RNA_def_property(srna, "undo_memory_limit", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "undomemory");
	RNA_def_property_range(prop, 0, 32767);
	RNA_def_property_ui_text(prop, "Undo Memory Size", "Maximum memory usage in megabytes (0 means unlimited)");

	prop= RNA_def_property(srna, "use_global_undo", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_GLOBALUNDO);
	RNA_def_property_ui_text(prop, "Global Undo", "Global undo works by keeping a full copy of the file itself in memory, so takes extra memory");

	/* auto keyframing */	
	prop= RNA_def_property(srna, "use_auto_keying", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "autokey_mode", AUTOKEY_ON);
	RNA_def_property_ui_text(prop, "Auto Keying Enable", "Automatic keyframe insertion for Objects and Bones");
	RNA_def_property_ui_icon(prop, ICON_REC, 0);

	prop= RNA_def_property(srna, "auto_keying_mode", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, auto_key_modes);
	RNA_def_property_enum_funcs(prop, "rna_userdef_autokeymode_get", "rna_userdef_autokeymode_set", NULL);
	RNA_def_property_ui_text(prop, "Auto Keying Mode", "Mode of automatic keyframe insertion for Objects and Bones");

	prop= RNA_def_property(srna, "use_keyframe_insert_available", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "autokey_flag", AUTOKEY_FLAG_INSERTAVAIL);
	RNA_def_property_ui_text(prop, "Auto Keyframe Insert Available", "Automatic keyframe insertion in available curves");
	
	prop= RNA_def_property(srna, "use_keyframe_insert_keyingset", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "autokey_flag", AUTOKEY_FLAG_ONLYKEYINGSET);
	RNA_def_property_ui_text(prop, "Auto Keyframe Insert Keying Set", "Automatic keyframe insertion using active Keying Set");
	
	/* keyframing settings */
	prop= RNA_def_property(srna, "use_keyframe_insert_needed", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "autokey_flag", AUTOKEY_FLAG_INSERTNEEDED);
	RNA_def_property_ui_text(prop, "Keyframe Insert Needed", "Keyframe insertion only when keyframe needed");

	prop= RNA_def_property(srna, "use_visual_keying", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "autokey_flag", AUTOKEY_FLAG_AUTOMATKEY);
	RNA_def_property_ui_text(prop, "Visual Keying", "Use Visual keying automatically for constrained objects");
	
	prop= RNA_def_property(srna, "use_insertkey_xyz_to_rgb", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "autokey_flag", AUTOKEY_FLAG_XYZ2RGB);
	RNA_def_property_ui_text(prop, "New F-Curve Colors - XYZ to RGB", "Color for newly added transformation F-Curves (Location, Rotation, Scale) and also Color is based on the transform axis");
	
	prop= RNA_def_property(srna, "keyframe_new_interpolation_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, beztriple_interpolation_mode_items);
	RNA_def_property_enum_sdna(prop, NULL, "ipo_new");
	RNA_def_property_ui_text(prop, "New Interpolation Type", "");
	
	prop= RNA_def_property(srna, "keyframe_new_handle_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, beztriple_handle_type_items);
	RNA_def_property_enum_sdna(prop, NULL, "keyhandles_new");
	RNA_def_property_ui_text(prop, "New Handles Type", "");
	
	/* frame numbers */
	prop= RNA_def_property(srna, "use_negative_frames", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "flag", USER_NONEGFRAMES);
	RNA_def_property_ui_text(prop, "Allow Negative Frames", "Current frame number can be manually set to a negative value");
	
	/* grease pencil */
	prop= RNA_def_property(srna, "grease_pencil_manhattan_distance", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "gp_manhattendist");
	RNA_def_property_range(prop, 0, 100);
	RNA_def_property_ui_text(prop, "Grease Pencil Manhattan Distance", "Pixels moved by mouse per axis when drawing stroke");

	prop= RNA_def_property(srna, "grease_pencil_euclidean_distance", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "gp_euclideandist");
	RNA_def_property_range(prop, 0, 100);
	RNA_def_property_ui_text(prop, "Grease Pencil Euclidean Distance", "Distance moved by mouse when drawing stroke (in pixels) to include");

	prop= RNA_def_property(srna, "use_grease_pencil_smooth_stroke", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "gp_settings", GP_PAINT_DOSMOOTH);
	RNA_def_property_ui_text(prop, "Grease Pencil Smooth Stroke", "Smooth the final stroke");

	prop= RNA_def_property(srna, "use_grease_pencil_simplify_stroke", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "gp_settings", GP_PAINT_DOSIMPLIFY);
	RNA_def_property_ui_text(prop, "Grease Pencil Simplify Stroke", "Simplify the final stroke");

	prop= RNA_def_property(srna, "grease_pencil_eraser_radius", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "gp_eraser");
	RNA_def_property_range(prop, 0, 100);
	RNA_def_property_ui_text(prop, "Grease Pencil Eraser Radius", "Radius of eraser 'brush'");

	/* sculpt and paint */

	prop= RNA_def_property(srna, "sculpt_paint_overlay_color", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "sculpt_paint_overlay_col");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Sculpt/Paint Overlay Color", "Color of texture overlay");

	/* duplication linking */
	prop= RNA_def_property(srna, "use_duplicate_mesh", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_MESH);
	RNA_def_property_ui_text(prop, "Duplicate Mesh", "Causes mesh data to be duplicated with the object");

	prop= RNA_def_property(srna, "use_duplicate_surface", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_SURF);
	RNA_def_property_ui_text(prop, "Duplicate Surface", "Causes surface data to be duplicated with the object");
	
	prop= RNA_def_property(srna, "use_duplicate_curve", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_CURVE);
	RNA_def_property_ui_text(prop, "Duplicate Curve", "Causes curve data to be duplicated with the object");

	prop= RNA_def_property(srna, "use_duplicate_text", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_FONT);
	RNA_def_property_ui_text(prop, "Duplicate Text", "Causes text data to be duplicated with the object");

	prop= RNA_def_property(srna, "use_duplicate_metaball", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_MBALL);
	RNA_def_property_ui_text(prop, "Duplicate Metaball", "Causes metaball data to be duplicated with the object");
	
	prop= RNA_def_property(srna, "use_duplicate_armature", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_ARM);
	RNA_def_property_ui_text(prop, "Duplicate Armature", "Causes armature data to be duplicated with the object");

	prop= RNA_def_property(srna, "use_duplicate_lamp", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_LAMP);
	RNA_def_property_ui_text(prop, "Duplicate Lamp", "Causes lamp data to be duplicated with the object");

	prop= RNA_def_property(srna, "use_duplicate_material", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_MAT);
	RNA_def_property_ui_text(prop, "Duplicate Material", "Causes material data to be duplicated with the object");

	prop= RNA_def_property(srna, "use_duplicate_texture", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_TEX);
	RNA_def_property_ui_text(prop, "Duplicate Texture", "Causes texture data to be duplicated with the object");
		
		// xxx
	prop= RNA_def_property(srna, "use_duplicate_fcurve", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_IPO);
	RNA_def_property_ui_text(prop, "Duplicate F-Curve", "Causes F-curve data to be duplicated with the object");
		// xxx
	prop= RNA_def_property(srna, "use_duplicate_action", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_ACT);
	RNA_def_property_ui_text(prop, "Duplicate Action", "Causes actions to be duplicated with the object");
	
	prop= RNA_def_property(srna, "use_duplicate_particle", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "dupflag", USER_DUP_PSYS);
	RNA_def_property_ui_text(prop, "Duplicate Particle", "Causes particle systems to be duplicated with the object");
}

static void rna_def_userdef_system(BlenderRNA *brna)
{
	PropertyRNA *prop;
	StructRNA *srna;

	static EnumPropertyItem gl_texture_clamp_items[] = {
		{0, "CLAMP_OFF", 0, "Off", ""},
		{8192, "CLAMP_8192", 0, "8192", ""},
		{4096, "CLAMP_4096", 0, "4096", ""},
		{2048, "CLAMP_2048", 0, "2048", ""},
		{1024, "CLAMP_1024", 0, "1024", ""},
		{512, "CLAMP_512", 0, "512", ""},
		{256, "CLAMP_256", 0, "256", ""},
		{128, "CLAMP_128", 0, "128", ""},
		{0, NULL, 0, NULL, NULL}};

	static EnumPropertyItem audio_mixing_samples_items[] = {
		{256, "SAMPLES_256", 0, "256", "Set audio mixing buffer size to 256 samples"},
		{512, "SAMPLES_512", 0, "512", "Set audio mixing buffer size to 512 samples"},
		{1024, "SAMPLES_1024", 0, "1024", "Set audio mixing buffer size to 1024 samples"},
		{2048, "SAMPLES_2048", 0, "2048", "Set audio mixing buffer size to 2048 samples"},
		{4096, "SAMPLES_4096", 0, "4096", "Set audio mixing buffer size to 4096 samples"},
		{8192, "SAMPLES_8192", 0, "8192", "Set audio mixing buffer size to 8192 samples"},
		{16384, "SAMPLES_16384", 0, "16384", "Set audio mixing buffer size to 16384 samples"},
		{32768, "SAMPLES_32768", 0, "32768", "Set audio mixing buffer size to 32768 samples"},
		{0, NULL, 0, NULL, NULL}};

	static EnumPropertyItem audio_device_items[] = {
		{0, "NONE", 0, "None", "Null device - there will be no audio output"},
#ifdef WITH_SDL
		{1, "SDL", 0, "SDL", "SDL device - simple direct media layer, recommended for sequencer usage"},
#endif
#ifdef WITH_OPENAL
		{2, "OPENAL", 0, "OpenAL", "OpenAL device - supports 3D audio, recommended for game engine usage"},
#endif
#ifdef WITH_JACK
		{3, "JACK", 0, "Jack", "Jack device - open source pro audio, recommended for pro audio users"},
#endif
		{0, NULL, 0, NULL, NULL}};

	static EnumPropertyItem audio_rate_items[] = {
//		{8000, "RATE_8000", 0, "8 kHz", "Set audio sampling rate to 8000 samples per second"},
//		{11025, "RATE_11025", 0, "11.025 kHz", "Set audio sampling rate to 11025 samples per second"},
//		{16000, "RATE_16000", 0, "16 kHz", "Set audio sampling rate to 16000 samples per second"},
//		{22050, "RATE_22050", 0, "22.05 kHz", "Set audio sampling rate to 22050 samples per second"},
//		{32000, "RATE_32000", 0, "32 kHz", "Set audio sampling rate to 32000 samples per second"},
		{44100, "RATE_44100", 0, "44.1 kHz", "Set audio sampling rate to 44100 samples per second"},
		{48000, "RATE_48000", 0, "48 kHz", "Set audio sampling rate to 48000 samples per second"},
//		{88200, "RATE_88200", 0, "88.2 kHz", "Set audio sampling rate to 88200 samples per second"},
		{96000, "RATE_96000", 0, "96 kHz", "Set audio sampling rate to 96000 samples per second"},
		{192000, "RATE_192000", 0, "192 kHz", "Set audio sampling rate to 192000 samples per second"},
		{0, NULL, 0, NULL, NULL}};

	static EnumPropertyItem audio_format_items[] = {
		{0x01, "U8", 0, "8-bit Unsigned", "Set audio sample format to 8 bit unsigned integer"},
		{0x12, "S16", 0, "16-bit Signed", "Set audio sample format to 16 bit signed integer"},
		{0x13, "S24", 0, "24-bit Signed", "Set audio sample format to 24 bit signed integer"},
		{0x14, "S32", 0, "32-bit Signed", "Set audio sample format to 32 bit signed integer"},
		{0x24, "FLOAT", 0, "32-bit Float", "Set audio sample format to 32 bit float"},
		{0x28, "DOUBLE", 0, "64-bit Float", "Set audio sample format to 64 bit float"},
		{0, NULL, 0, NULL, NULL}};

	static EnumPropertyItem audio_channel_items[] = {
		{1, "MONO", 0, "Mono", "Set audio channels to mono"},
		{2, "STEREO", 0, "Stereo", "Set audio channels to stereo"},
		{4, "SURROUND4", 0, "4 Channels", "Set audio channels to 4 channels"},
		{6, "SURROUND51", 0, "5.1 Surround", "Set audio channels to 5.1 surround sound"},
		{8, "SURROUND71", 0, "7.1 Surround", "Set audio channels to 7.1 surround sound"},
		{0, NULL, 0, NULL, NULL}};

	static EnumPropertyItem draw_method_items[] = {
		{USER_DRAW_AUTOMATIC, "AUTOMATIC", 0, "Automatic", "Automatically set based on graphics card and driver"},
		{USER_DRAW_TRIPLE, "TRIPLE_BUFFER", 0, "Triple Buffer", "Use a third buffer for minimal redraws at the cost of more memory"},
		{USER_DRAW_OVERLAP, "OVERLAP", 0, "Overlap", "Redraw all overlapping regions, minimal memory usage but more redraws"},
		{USER_DRAW_OVERLAP_FLIP, "OVERLAP_FLIP", 0, "Overlap Flip", "Redraw all overlapping regions, minimal memory usage but more redraws (for graphics drivers that do flipping)"},
		{USER_DRAW_FULL, "FULL", 0, "Full", "Do a full redraw each time, slow, only use for reference or when all else fails"},
		{0, NULL, 0, NULL, NULL}};
	
	static EnumPropertyItem color_picker_types[] = {
		{USER_CP_CIRCLE, "CIRCLE", 0, "Circle", "A circular Hue/Saturation color wheel, with Value slider"},
		{USER_CP_SQUARE_SV, "SQUARE_SV", 0, "Square (SV + H)", "A square showing Saturation/Value, with Hue slider"},
		{USER_CP_SQUARE_HS, "SQUARE_HS", 0, "Square (HS + V)", "A square showing Hue/Saturation, with Value slider"},
		{USER_CP_SQUARE_HV, "SQUARE_HV", 0, "Square (HV + S)", "A square showing Hue/Value, with Saturation slider"},
		{0, NULL, 0, NULL, NULL}};
	
		/* hardcoded here, could become dynamic somehow */
	static EnumPropertyItem language_items[] = {
		{0, "ENGLISH", 0, "English", ""},
		{1, "JAPANESE", 0, "Japanese", ""},
		{2, "DUTCH", 0, "Dutch", ""},
		{3, "ITALIAN", 0, "Italian", ""},
		{4, "GERMAN", 0, "German", ""},
		{5, "FINNISH", 0, "Finnish", ""},
		{6, "SWEDISH", 0, "Swedish", ""},
		{7, "FRENCH", 0, "French", ""},
		{8, "SPANISH", 0, "Spanish", ""},
		{9, "CATALAN", 0, "Catalan", ""},
		{10, "CZECH", 0, "Czech", ""},
		{11, "BRAZILIAN_PORTUGUESE", 0, "Brazilian Portuguese", ""},
		{12, "SIMPLIFIED_CHINESE", 0, "Simplified Chinese", ""},
		{13, "RUSSIAN", 0, "Russian", ""},
		{14, "CROATIAN", 0, "Croatian", ""},
		{15, "SERBIAN", 0, "Serbian", ""},
		{16, "UKRAINIAN", 0, "Ukrainian", ""},
		{17, "POLISH", 0, "Polish", ""},
		{18, "ROMANIAN", 0, "Romanian", ""},
		{19, "ARABIC", 0, "Arabic", ""},
		{20, "BULGARIAN", 0, "Bulgarian", ""},
		{21, "GREEK", 0, "Greek", ""},
		{22, "KOREAN", 0, "Korean", ""},
		{0, NULL, 0, NULL, NULL}};

	srna= RNA_def_struct(brna, "UserPreferencesSystem", NULL);
	RNA_def_struct_sdna(srna, "UserDef");
	RNA_def_struct_nested(brna, srna, "UserPreferences");
	RNA_def_struct_ui_text(srna, "System & OpenGL", "Graphics driver and operating system settings");

	/* Language */
	
	prop= RNA_def_property(srna, "use_international_fonts", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "transopts", USER_DOTRANSLATE);
	RNA_def_property_ui_text(prop, "International Fonts", "Use international fonts");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "dpi", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "dpi");
	RNA_def_property_range(prop, 48, 128);
	RNA_def_property_ui_text(prop, "DPI", "Font size and resolution for display");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "scrollback", PROP_INT, PROP_UNSIGNED);
	RNA_def_property_int_sdna(prop, NULL, "scrollback");
	RNA_def_property_range(prop, 32, 32768);
	RNA_def_property_ui_text(prop, "Scrollback", "Maximum number of lines to store for the console buffer");

	/* Language Selection */

	prop= RNA_def_property(srna, "language", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, language_items);
	RNA_def_property_ui_text(prop, "Language", "Language use for translation");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "use_translate_tooltips", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "transopts", USER_TR_TOOLTIPS);
	RNA_def_property_ui_text(prop, "Translate Tooltips", "Translate Tooltips");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "use_translate_buttons", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "transopts", USER_TR_BUTTONS);
	RNA_def_property_ui_text(prop, "Translate Buttons", "Translate button labels");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "use_translate_toolbox", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "transopts", USER_TR_MENUS);
	RNA_def_property_ui_text(prop, "Translate Toolbox", "Translate toolbox menu");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "use_textured_fonts", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "transopts", USER_USETEXTUREFONT);
	RNA_def_property_ui_text(prop, "Textured Fonts", "Use textures for drawing international fonts");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	/* System & OpenGL */

	prop= RNA_def_property(srna, "solid_lights", PROP_COLLECTION, PROP_NONE);
	RNA_def_property_collection_sdna(prop, NULL, "light", "");
	RNA_def_property_struct_type(prop, "UserSolidLight");
	RNA_def_property_ui_text(prop, "Solid Lights", "Lights user to display objects in solid draw mode");

	prop= RNA_def_property(srna, "use_weight_color_range", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_CUSTOM_RANGE);
	RNA_def_property_ui_text(prop, "Use Weight Color Range", "Enable color range used for weight visualization in weight painting mode");
	RNA_def_property_update(prop, 0, "rna_UserDef_weight_color_update");

	prop= RNA_def_property(srna, "weight_color_range", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "coba_weight");
	RNA_def_property_struct_type(prop, "ColorRamp");
	RNA_def_property_ui_text(prop, "Weight Color Range", "Color range used for weight visualization in weight painting mode");
	RNA_def_property_update(prop, 0, "rna_UserDef_weight_color_update");

	prop= RNA_def_property(srna, "color_picker_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, color_picker_types);
	RNA_def_property_enum_sdna(prop, NULL, "color_picker_type");
	RNA_def_property_ui_text(prop, "Color Picker Type", "Different styles of displaying the color picker widget");
	
	prop= RNA_def_property(srna, "use_preview_images", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_ALLWINCODECS);
	RNA_def_property_ui_text(prop, "Enable All Codecs", "Enables automatic saving of preview images in the .blend file (Windows only)");

	prop= RNA_def_property(srna, "use_scripts_auto_execute", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "flag", USER_SCRIPT_AUTOEXEC_DISABLE);
	RNA_def_property_ui_text(prop, "Auto Run Python Scripts", "Allow any .blend file to run scripts automatically (unsafe with blend files from an untrusted source)");
	RNA_def_property_update(prop, 0, "rna_userdef_script_autoexec_update");

	prop= RNA_def_property(srna, "use_tabs_as_spaces", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "flag", USER_TXT_TABSTOSPACES_DISABLE);
	RNA_def_property_ui_text(prop, "Tabs as Spaces", "Automatically converts all new tabs into spaces for new and loaded text files");

	prop= RNA_def_property(srna, "prefetch_frames", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "prefetchframes");
	RNA_def_property_range(prop, 0, 500);
	RNA_def_property_ui_text(prop, "Prefetch Frames", "Number of frames to render ahead during playback");

	prop= RNA_def_property(srna, "memory_cache_limit", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "memcachelimit");
	RNA_def_property_range(prop, 0, (sizeof(void *) ==8)? 1024*16: 1024); /* 32 bit 2 GB, 64 bit 16 GB */
	RNA_def_property_ui_text(prop, "Memory Cache Limit", "Memory cache limit in sequencer (megabytes)");
	RNA_def_property_update(prop, 0, "rna_Userdef_memcache_update");

	prop= RNA_def_property(srna, "frame_server_port", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "frameserverport");
	RNA_def_property_range(prop, 0, 32727);
	RNA_def_property_ui_text(prop, "Frame Server Port", "Frameserver Port for Frameserver Rendering");

	prop= RNA_def_property(srna, "gl_clip_alpha", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "glalphaclip");
	RNA_def_property_range(prop, 0.0f, 1.0f);
	RNA_def_property_ui_text(prop, "Clip Alpha", "Clip alpha below this threshold in the 3D textured view");
	RNA_def_property_update(prop, 0, "rna_userdef_update");
	
	prop= RNA_def_property(srna, "use_mipmaps", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "gameflags", USER_DISABLE_MIPMAP);
	RNA_def_property_ui_text(prop, "Mipmaps", "Scale textures for the 3D View (looks nicer but uses more memory and slows image reloading)");
	RNA_def_property_update(prop, 0, "rna_userdef_mipmap_update");

	prop= RNA_def_property(srna, "use_vertex_buffer_objects", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "gameflags", USER_DISABLE_VBO);
	RNA_def_property_ui_text(prop, "VBOs", "Use Vertex Buffer Objects (or Vertex Arrays, if unsupported) for viewport rendering");

	prop= RNA_def_property(srna, "use_antialiasing", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "gameflags", USER_DISABLE_AA);
	RNA_def_property_ui_text(prop, "Anti-aliasing", "Use anti-aliasing for the 3D view (may impact redraw performance)");
	
	prop= RNA_def_property(srna, "gl_texture_limit", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "glreslimit");
	RNA_def_property_enum_items(prop, gl_texture_clamp_items);
	RNA_def_property_ui_text(prop, "GL Texture Limit", "Limit the texture size to save graphics memory");
	RNA_def_property_update(prop, 0, "rna_userdef_mipmap_update");

	prop= RNA_def_property(srna, "texture_time_out", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "textimeout");
	RNA_def_property_range(prop, 0, 3600);
	RNA_def_property_ui_text(prop, "Texture Time Out", "Time since last access of a GL texture in seconds after which it is freed. (Set to 0 to keep textures allocated.)");

	prop= RNA_def_property(srna, "texture_collection_rate", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "texcollectrate");
	RNA_def_property_range(prop, 1, 3600);
	RNA_def_property_ui_text(prop, "Texture Collection Rate", "Number of seconds between each run of the GL texture garbage collector");

	prop= RNA_def_property(srna, "window_draw_method", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "wmdrawmethod");
	RNA_def_property_enum_items(prop, draw_method_items);
	RNA_def_property_ui_text(prop, "Window Draw Method", "Drawing method used by the window manager");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "audio_mixing_buffer", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "mixbufsize");
	RNA_def_property_enum_items(prop, audio_mixing_samples_items);
	RNA_def_property_ui_text(prop, "Audio Mixing Buffer", "Sets the number of samples used by the audio mixing buffer");
	RNA_def_property_update(prop, 0, "rna_UserDef_audio_update");

	prop= RNA_def_property(srna, "audio_device", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "audiodevice");
	RNA_def_property_enum_items(prop, audio_device_items);
	RNA_def_property_ui_text(prop, "Audio Device", "Sets the audio output device");
	RNA_def_property_update(prop, 0, "rna_UserDef_audio_update");

	prop= RNA_def_property(srna, "audio_sample_rate", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "audiorate");
	RNA_def_property_enum_items(prop, audio_rate_items);
	RNA_def_property_ui_text(prop, "Audio Sample Rate", "Sets the audio sample rate");
	RNA_def_property_update(prop, 0, "rna_UserDef_audio_update");

	prop= RNA_def_property(srna, "audio_sample_format", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "audioformat");
	RNA_def_property_enum_items(prop, audio_format_items);
	RNA_def_property_ui_text(prop, "Audio Sample Format", "Sets the audio sample format");
	RNA_def_property_update(prop, 0, "rna_UserDef_audio_update");

	prop= RNA_def_property(srna, "audio_channels", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "audiochannels");
	RNA_def_property_enum_items(prop, audio_channel_items);
	RNA_def_property_ui_text(prop, "Audio Channels", "Sets the audio channel count");
	RNA_def_property_update(prop, 0, "rna_UserDef_audio_update");

	prop= RNA_def_property(srna, "screencast_fps", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "scrcastfps");
	RNA_def_property_range(prop, 10, 50);
	RNA_def_property_ui_text(prop, "FPS", "Frame rate for the screencast to be played back");

	prop= RNA_def_property(srna, "screencast_wait_time", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "scrcastwait");
	RNA_def_property_range(prop, 50, 1000);
	RNA_def_property_ui_text(prop, "Wait Timer (ms)", "Time in milliseconds between each frame recorded for screencast");

#if 0
	prop= RNA_def_property(srna, "verse_master", PROP_STRING, PROP_NONE);
	RNA_def_property_string_sdna(prop, NULL, "versemaster");
	RNA_def_property_ui_text(prop, "Verse Master", "The Verse Master-server IP");

	prop= RNA_def_property(srna, "verse_username", PROP_STRING, PROP_NONE);
	RNA_def_property_string_sdna(prop, NULL, "verseuser");
	RNA_def_property_ui_text(prop, "Verse Username", "The Verse user name");
#endif
}

static void rna_def_userdef_input(BlenderRNA *brna)
{
	PropertyRNA *prop;
	StructRNA *srna;

	static EnumPropertyItem select_mouse_items[] = {
		{USER_LMOUSESELECT, "LEFT", 0, "Left", "Use left Mouse Button for selection"},
		{0, "RIGHT", 0, "Right", "Use Right Mouse Button for selection"},
		{0, NULL, 0, NULL, NULL}};
		
	static EnumPropertyItem view_rotation_items[] = {
		{0, "TURNTABLE", 0, "Turntable", "Use turntable style rotation in the viewport"},
		{USER_TRACKBALL, "TRACKBALL", 0, "Trackball", "Use trackball style rotation in the viewport"},
		{0, NULL, 0, NULL, NULL}};
		
	static EnumPropertyItem view_zoom_styles[] = {
		{USER_ZOOM_CONT, "CONTINUE", 0, "Continue", "Old style zoom, continues while moving mouse up or down"},
		{USER_ZOOM_DOLLY, "DOLLY", 0, "Dolly", "Zooms in and out based on vertical mouse movement"},
		{USER_ZOOM_SCALE, "SCALE", 0, "Scale", "Zooms in and out like scaling the view, mouse movements relative to center"},
		{0, NULL, 0, NULL, NULL}};
	
	static EnumPropertyItem view_zoom_axes[] = {
		{0,						"VERTICAL", 0, "Vertical", "Zooms in and out based on vertical mouse movement"},
		{USER_ZOOM_DOLLY_HORIZ, "HORIZONTAL", 0, "Horizontal", "Zooms in and out based on horizontal mouse movement"},
		{0, NULL, 0, NULL, NULL}};
		
	srna= RNA_def_struct(brna, "UserPreferencesInput", NULL);
	RNA_def_struct_sdna(srna, "UserDef");
	RNA_def_struct_nested(brna, srna, "UserPreferences");
	RNA_def_struct_ui_text(srna, "Input", "Settings for input devices");
	
	prop= RNA_def_property(srna, "select_mouse", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_bitflag_sdna(prop, NULL, "flag");
	RNA_def_property_enum_items(prop, select_mouse_items);
	RNA_def_property_enum_funcs(prop, NULL, "rna_userdef_select_mouse_set", NULL);
	RNA_def_property_ui_text(prop, "Select Mouse", "The mouse button used for selection");
	
	prop= RNA_def_property(srna, "view_zoom_method", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "viewzoom");
	RNA_def_property_enum_items(prop, view_zoom_styles);
	RNA_def_property_ui_text(prop, "Zoom Style", "Which style to use for viewport scaling");
	
	prop= RNA_def_property(srna, "view_zoom_axis", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_bitflag_sdna(prop, NULL, "uiflag");
	RNA_def_property_enum_items(prop, view_zoom_axes);
	RNA_def_property_ui_text(prop, "Zoom Axis", "Axis of mouse movement to zoom in or out on");
	
	prop= RNA_def_property(srna, "invert_mouse_wheel_zoom", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_ZOOM_INVERT);
	RNA_def_property_ui_text(prop, "Invert Zoom Direction", "Invert the axis of mouse movement for zooming");
	
	prop= RNA_def_property(srna, "view_rotate_method", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_bitflag_sdna(prop, NULL, "flag");
	RNA_def_property_enum_items(prop, view_rotation_items);
	RNA_def_property_ui_text(prop, "View Rotation", "Rotation style in the viewport");
	
	prop= RNA_def_property(srna, "use_mouse_continuous", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_CONTINUOUS_MOUSE);
	RNA_def_property_ui_text(prop, "Continuous Grab", "Allow moving the mouse outside the view on some manipulations (transform, ui control drag)");
	
	prop= RNA_def_property(srna, "ndof_pan_speed", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "ndof_pan");
	RNA_def_property_range(prop, 0, 200);
	RNA_def_property_ui_text(prop, "NDof Pan Speed", "The overall panning speed of an NDOF device, as percent of standard");

	prop= RNA_def_property(srna, "ndof_rotate_speed", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "ndof_rotate");
	RNA_def_property_range(prop, 0, 200);
	RNA_def_property_ui_text(prop, "NDof Rotation Speed", "The overall rotation speed of an NDOF device, as percent of standard");
	
	prop= RNA_def_property(srna, "mouse_double_click_time", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "dbl_click_time");
	RNA_def_property_range(prop, 1, 1000);
	RNA_def_property_ui_text(prop, "Double Click Timeout", "The time (in ms) for a double click");

	prop= RNA_def_property(srna, "use_mouse_emulate_3_button", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_TWOBUTTONMOUSE);
	RNA_def_property_ui_text(prop, "Emulate 3 Button Mouse", "Emulates Middle Mouse with Alt+LeftMouse (doesn't work with Left Mouse Select option)");

	prop= RNA_def_property(srna, "use_emulate_numpad", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_NONUMPAD);
	RNA_def_property_ui_text(prop, "Emulate Numpad", "Causes the 1 to 0 keys to act as the numpad (useful for laptops)");
	
	/* middle mouse button */
	prop= RNA_def_property(srna, "use_mouse_mmb_paste", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_MMB_PASTE);
	RNA_def_property_ui_text(prop, "Middle Mouse Paste", "In text window, paste with middle mouse button instead of panning");
	
	prop= RNA_def_property(srna, "invert_zoom_wheel", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_WHEELZOOMDIR);
	RNA_def_property_ui_text(prop, "Wheel Invert Zoom", "Swap the Mouse Wheel zoom direction");

	prop= RNA_def_property(srna, "wheel_scroll_lines", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "wheellinescroll");
	RNA_def_property_range(prop, 0, 32);
	RNA_def_property_ui_text(prop, "Wheel Scroll Lines", "The number of lines scrolled at a time with the mouse wheel");
	
	/* U.keymaps - custom keymaps that have been edited from default configs */
	prop= RNA_def_property(srna, "edited_keymaps", PROP_COLLECTION, PROP_NONE);
	RNA_def_property_collection_sdna(prop, NULL, "keymaps", NULL);
	RNA_def_property_struct_type(prop, "KeyMap");
	RNA_def_property_ui_text(prop, "Edited Keymaps", "");
}

static void rna_def_userdef_filepaths(BlenderRNA *brna)
{
	PropertyRNA *prop;
	StructRNA *srna;
	
	static EnumPropertyItem anim_player_presets[] = {
		//{0, "INTERNAL", 0, "Internal", "Built-in animation player"},	// doesn't work yet!
		{1, "BLENDER24", 0, "Blender 2.4", "Blender command line animation playback - path to Blender 2.4"},
		{2, "DJV", 0, "Djv", "Open source frame player: http://djv.sourceforge.net"},
		{3, "FRAMECYCLER", 0, "FrameCycler", "Frame player from IRIDAS"},
		{4, "RV", 0, "rv", "Frame player from Tweak Software"},
		{5, "MPLAYER", 0, "MPlayer", "Media player for video & png/jpeg/sgi image sequences"},
		{50, "CUSTOM", 0, "Custom", "Custom animation player executable path"},
		{0, NULL, 0, NULL, NULL}};
	
	srna= RNA_def_struct(brna, "UserPreferencesFilePaths", NULL);
	RNA_def_struct_sdna(srna, "UserDef");
	RNA_def_struct_nested(brna, srna, "UserPreferences");
	RNA_def_struct_ui_text(srna, "File Paths", "Default paths for external files");
	
	prop= RNA_def_property(srna, "show_hidden_files_datablocks", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_HIDE_DOT);
	RNA_def_property_ui_text(prop, "Hide Dot Files/Datablocks", "Hide files/datablocks that start with a dot(.*)");
	
	prop= RNA_def_property(srna, "use_filter_files", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "uiflag", USER_FILTERFILEEXTS);
	RNA_def_property_ui_text(prop, "Filter File Extensions", "Display only files with extensions in the image select window");
	
	prop= RNA_def_property(srna, "use_relative_paths", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_RELPATHS);
	RNA_def_property_ui_text(prop, "Relative Paths", "Default relative path option for the file selector");

	prop= RNA_def_property(srna, "use_file_compression", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_FILECOMPRESS);
	RNA_def_property_ui_text(prop, "Compress File", "Enable file compression when saving .blend files");

	prop= RNA_def_property(srna, "use_load_ui", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_negative_sdna(prop, NULL, "flag", USER_FILENOUI);
	RNA_def_property_ui_text(prop, "Load UI", "Load user interface setup when loading .blend files");

	prop= RNA_def_property(srna, "font_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "fontdir");
	RNA_def_property_ui_text(prop, "Fonts Directory", "The default directory to search for loading fonts");

	prop= RNA_def_property(srna, "texture_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "textudir");
	RNA_def_property_ui_text(prop, "Textures Directory", "The default directory to search for textures");

	prop= RNA_def_property(srna, "texture_plugin_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "plugtexdir");
	RNA_def_property_ui_text(prop, "Texture Plugin Directory", "The default directory to search for texture plugins");

	prop= RNA_def_property(srna, "sequence_plugin_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "plugseqdir");
	RNA_def_property_ui_text(prop, "Sequence Plugin Directory", "The default directory to search for sequence plugins");

	prop= RNA_def_property(srna, "render_output_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "renderdir");
	RNA_def_property_ui_text(prop, "Render Output Directory", "The default directory for rendering output");

	prop= RNA_def_property(srna, "script_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "pythondir");
	RNA_def_property_ui_text(prop, "Python Scripts Directory", "The default directory to search for Python scripts (resets python module search path: sys.path)");

	prop= RNA_def_property(srna, "sound_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "sounddir");
	RNA_def_property_ui_text(prop, "Sounds Directory", "The default directory to search for sounds");

	prop= RNA_def_property(srna, "temporary_directory", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "tempdir");
	RNA_def_property_ui_text(prop, "Temporary Directory", "The directory for storing temporary save files");
	RNA_def_property_update(prop, 0, "rna_userdef_temp_update");

	prop= RNA_def_property(srna, "image_editor", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "image_editor");
	RNA_def_property_ui_text(prop, "Image Editor", "Path to an image editor");
	
	prop= RNA_def_property(srna, "animation_player", PROP_STRING, PROP_DIRPATH);
	RNA_def_property_string_sdna(prop, NULL, "anim_player");
	RNA_def_property_ui_text(prop, "Animation Player", "Path to a custom animation/frame sequence player");

	prop= RNA_def_property(srna, "animation_player_preset", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "anim_player_preset");
	RNA_def_property_enum_items(prop, anim_player_presets);
	RNA_def_property_ui_text(prop, "Animation Player Preset", "Preset configs for external animation players");
	RNA_def_property_enum_default(prop, 1);		/* set default to blender 2.4 player until an internal one is back */
	
	/* Autosave  */

	prop= RNA_def_property(srna, "save_version", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "versions");
	RNA_def_property_range(prop, 0, 32);
	RNA_def_property_ui_text(prop, "Save Versions", "The number of old versions to maintain in the current directory, when manually saving");

	prop= RNA_def_property(srna, "use_auto_save_temporary_files", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_AUTOSAVE);
	RNA_def_property_ui_text(prop, "Auto Save Temporary Files", "Automatic saving of temporary files");
	RNA_def_property_update(prop, 0, "rna_userdef_autosave_update");

	prop= RNA_def_property(srna, "auto_save_time", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "savetime");
	RNA_def_property_range(prop, 1, 60);
	RNA_def_property_ui_text(prop, "Auto Save Time", "The time (in minutes) to wait between automatic temporary saves");
	RNA_def_property_update(prop, 0, "rna_userdef_autosave_update");

	prop= RNA_def_property(srna, "recent_files", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, 0, 30);
	RNA_def_property_ui_text(prop, "Recent Files", "Maximum number of recently opened files to remember");

	prop= RNA_def_property(srna, "use_save_preview_images", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", USER_SAVE_PREVIEWS);
	RNA_def_property_ui_text(prop, "Save Preview Images", "Enables automatic saving of preview images in the .blend file");
}

void rna_def_userdef_addon_collection(BlenderRNA *brna, PropertyRNA *cprop)
{
	StructRNA *srna;
	FunctionRNA *func;
	PropertyRNA *parm;

	RNA_def_property_srna(cprop, "Addons");
	srna= RNA_def_struct(brna, "Addons", NULL);
	RNA_def_struct_ui_text(srna, "User Add-Ons", "Collection of add-ons");

	func= RNA_def_function(srna, "new", "rna_userdef_addon_new");
	RNA_def_function_flag(func, FUNC_NO_SELF);
	RNA_def_function_ui_description(func, "Add a new addon");
	/* return type */
	parm= RNA_def_pointer(func, "addon", "Addon", "", "Addon datablock.");
	RNA_def_function_return(func, parm);

	func= RNA_def_function(srna, "remove", "rna_userdef_addon_remove");
	RNA_def_function_flag(func, FUNC_NO_SELF);
	RNA_def_function_ui_description(func, "Remove addon.");
	parm= RNA_def_pointer(func, "addon", "Addon", "", "Addon to remove.");
	RNA_def_property_flag(parm, PROP_REQUIRED|PROP_NEVER_NULL);
}

void RNA_def_userdef(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem user_pref_sections[] = {
		{USER_SECTION_INTERFACE, "INTERFACE", 0, "Interface", ""},
		{USER_SECTION_EDIT, "EDITING", 0, "Editing", ""},
		{USER_SECTION_INPUT, "INPUT", 0, "Input", ""},
		{USER_SECTION_ADDONS, "ADDONS", 0, "Add-Ons", ""},
		{USER_SECTION_THEME, "THEMES", 0, "Themes", ""},
		{USER_SECTION_FILE, "FILES", 0, "File", ""},
		{USER_SECTION_SYSTEM, "SYSTEM", 0, "System", ""},
		{0, NULL, 0, NULL, NULL}};

	rna_def_userdef_dothemes(brna);
	rna_def_userdef_solidlight(brna);

	srna= RNA_def_struct(brna, "UserPreferences", NULL);
	RNA_def_struct_sdna(srna, "UserDef");
	RNA_def_struct_ui_text(srna, "User Preferences", "Global user preferences");

	prop= RNA_def_property(srna, "active_section", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "userpref");
	RNA_def_property_enum_items(prop, user_pref_sections);
	RNA_def_property_ui_text(prop, "Active Section", "Active section of the user preferences shown in the user interface");
	RNA_def_property_update(prop, 0, "rna_userdef_update");

	prop= RNA_def_property(srna, "themes", PROP_COLLECTION, PROP_NONE);
	RNA_def_property_collection_sdna(prop, NULL, "themes", NULL);
	RNA_def_property_struct_type(prop, "Theme");
	RNA_def_property_ui_text(prop, "Themes", "");

	prop= RNA_def_property(srna, "ui_styles", PROP_COLLECTION, PROP_NONE);
	RNA_def_property_collection_sdna(prop, NULL, "uistyles", NULL);
	RNA_def_property_struct_type(prop, "ThemeStyle");
	RNA_def_property_ui_text(prop, "Styles", "");
	
	prop= RNA_def_property(srna, "addons", PROP_COLLECTION, PROP_NONE);
	RNA_def_property_collection_sdna(prop, NULL, "addons", NULL);
	RNA_def_property_struct_type(prop, "Addon");
	RNA_def_property_ui_text(prop, "Addon", "");
	rna_def_userdef_addon_collection(brna, prop);


	/* nested structs */
	prop= RNA_def_property(srna, "view", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_struct_type(prop, "UserPreferencesView");
	RNA_def_property_pointer_funcs(prop, "rna_UserDef_view_get", NULL, NULL, NULL);
	RNA_def_property_ui_text(prop, "View & Controls", "Preferences related to viewing data");

	prop= RNA_def_property(srna, "edit", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_struct_type(prop, "UserPreferencesEdit");
	RNA_def_property_pointer_funcs(prop, "rna_UserDef_edit_get", NULL, NULL, NULL);
	RNA_def_property_ui_text(prop, "Edit Methods", "Settings for interacting with Blender data");
	
	prop= RNA_def_property(srna, "inputs", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_struct_type(prop, "UserPreferencesInput");
	RNA_def_property_pointer_funcs(prop, "rna_UserDef_input_get", NULL, NULL, NULL);
	RNA_def_property_ui_text(prop, "Inputs", "Settings for input devices");
	
	prop= RNA_def_property(srna, "filepaths", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_struct_type(prop, "UserPreferencesFilePaths");
	RNA_def_property_pointer_funcs(prop, "rna_UserDef_filepaths_get", NULL, NULL, NULL);
	RNA_def_property_ui_text(prop, "File Paths", "Default paths for external files");
	
	prop= RNA_def_property(srna, "system", PROP_POINTER, PROP_NONE);
	RNA_def_property_flag(prop, PROP_NEVER_NULL);
	RNA_def_property_struct_type(prop, "UserPreferencesSystem");
	RNA_def_property_pointer_funcs(prop, "rna_UserDef_system_get", NULL, NULL, NULL);
	RNA_def_property_ui_text(prop, "System & OpenGL", "Graphics driver and operating system settings");
	
	rna_def_userdef_view(brna);
	rna_def_userdef_edit(brna);
	rna_def_userdef_input(brna);
	rna_def_userdef_filepaths(brna);
	rna_def_userdef_system(brna);
	rna_def_userdef_addon(brna);
	
}

#endif

