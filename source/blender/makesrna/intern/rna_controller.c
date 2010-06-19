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

#include "WM_types.h"
#include "RNA_define.h"

#include "rna_internal.h"
#include "DNA_object_types.h"
#include "DNA_controller_types.h"

EnumPropertyItem controller_type_items[] ={
	{CONT_LOGIC_AND, "LOGIC_AND", 0, "And", "Logic And"},
	{CONT_LOGIC_OR, "LOGIC_OR", 0, "Or", "Logic Or"},
	{CONT_LOGIC_NAND, "LOGIC_NAND", 0, "Nand", "Logic Nand"},
	{CONT_LOGIC_NOR, "LOGIC_NOR", 0, "Nor", "Logic Nor"},
	{CONT_LOGIC_XOR, "LOGIC_XOR", 0, "Xor", "Logic Xor"},
	{CONT_LOGIC_XNOR, "LOGIC_XNOR", 0, "Xnor", "Logic Xnor"},
	{CONT_EXPRESSION, "EXPRESSION", 0, "Expression", ""},
	{CONT_PYTHON, "PYTHON", 0, "Python Script", ""},
	{0, NULL, 0, NULL, NULL}};

#ifdef RNA_RUNTIME

#include "BKE_sca.h"

static struct StructRNA* rna_Controller_refine(struct PointerRNA *ptr)
{
	bController *controller= (bController*)ptr->data;

	switch(controller->type) {
		case CONT_LOGIC_AND:
			return &RNA_AndController;
		case CONT_LOGIC_OR:
			return &RNA_OrController;
		case CONT_LOGIC_NAND:
			return &RNA_NandController;
		case CONT_LOGIC_NOR:
			return &RNA_NorController;
		case CONT_LOGIC_XOR:
			return &RNA_XorController;
		case CONT_LOGIC_XNOR:
			return &RNA_XnorController;
		 case CONT_EXPRESSION:
			return &RNA_ExpressionController;
		case CONT_PYTHON:
			return &RNA_PythonController;
		default:
			return &RNA_Controller;
	}
}

static void rna_Controller_type_set(struct PointerRNA *ptr, int value)
{
	bController *cont= (bController *)ptr->data;
	if (value != cont->type)
	{
		cont->type = value;
		init_controller(cont);
	}
}

static int rna_Controller_state_number_get(struct PointerRNA *ptr)
{
	bController *cont= (bController *)ptr->data;
	int bit;

	for (bit=0; bit<32; bit++) {
		if (cont->state_mask & (1<<bit))
			return bit+1;
	}
	return 0;
}

static void rna_Controller_state_number_set(struct PointerRNA *ptr, const int value)
{
	bController *cont= (bController *)ptr->data;
	if (value < 1 || value > OB_MAX_STATES)
		return;

	cont->state_mask = (1 << (value - 1));
}

static void rna_Controller_state_get(PointerRNA *ptr, int *values)
{
	bController *cont= (bController *)ptr->data;
	int i;

	memset(values, 0, sizeof(int)*OB_MAX_STATES);
	for(i=0; i<OB_MAX_STATES; i++)
		values[i] = (cont->state_mask & (1<<i));
}

static void rna_Controller_state_set(PointerRNA *ptr, const int *values)
{
	bController *cont= (bController *)ptr->data;
	int i, tot= 0;

	/* ensure we always have some state selected */
	for(i=0; i<OB_MAX_STATES; i++)
		if(values[i])
			tot++;
	
	if(tot==0)
		return;

	/* only works for one state at once */
	if(tot>1)
		return;

	for(i=0; i<OB_MAX_STATES; i++) {
		if(values[i]) cont->state_mask |= (1<<i);
		else cont->state_mask &= ~(1<<i);
	}
}

#else

void RNA_def_controller(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem python_controller_modes[] ={
		{CONT_PY_SCRIPT, "SCRIPT", 0, "Script", ""},
		{CONT_PY_MODULE, "MODULE", 0, "Module", ""},
		{0, NULL, 0, NULL, NULL}};

	/* Controller */
	srna= RNA_def_struct(brna, "Controller", NULL);
	RNA_def_struct_sdna(srna, "bController");
	RNA_def_struct_refine_func(srna, "rna_Controller_refine");
	RNA_def_struct_ui_text(srna, "Controller", "Game engine logic brick to process events, connecting sensors to actuators");

	prop= RNA_def_property(srna, "name", PROP_STRING, PROP_NONE);
	RNA_def_property_ui_text(prop, "Name", "");
	RNA_def_struct_name_property(srna, prop);
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	prop= RNA_def_property(srna, "type", PROP_ENUM, PROP_NONE);
	RNA_def_property_clear_flag(prop, PROP_ANIMATABLE);
	RNA_def_property_enum_funcs(prop, NULL, "rna_Controller_type_set", NULL);
	RNA_def_property_enum_items(prop, controller_type_items);
	RNA_def_property_ui_text(prop, "Type", "");
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	prop= RNA_def_property(srna, "expanded", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", CONT_SHOW);
	RNA_def_property_ui_text(prop, "Expanded", "Set controller expanded in the user interface");
	RNA_def_property_ui_icon(prop, ICON_TRIA_RIGHT, 1);	
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	prop= RNA_def_property(srna, "priority", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", CONT_PRIO);
	RNA_def_property_ui_text(prop, "Priority", "Mark controller for execution before all non-marked controllers (good for startup scripts)");
	RNA_def_property_ui_icon(prop, ICON_BOOKMARKS, 1);
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	/* State */
	
	// array of OB_MAX_STATES
	prop= RNA_def_property(srna, "state", PROP_BOOLEAN, PROP_LAYER_MEMBER);
	RNA_def_property_array(prop, OB_MAX_STATES);
	RNA_def_property_clear_flag(prop, PROP_EDITABLE);
	RNA_def_property_ui_text(prop, "", "Set Controller state index (1 to 30)");
	RNA_def_property_boolean_funcs(prop, "rna_Controller_state_get", "rna_Controller_state_set");
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	// number of the state
	prop= RNA_def_property(srna, "state_number", PROP_INT, PROP_UNSIGNED);
	RNA_def_property_int_sdna(prop, NULL, "state_mask");
	RNA_def_property_range(prop, 1, OB_MAX_STATES);
	RNA_def_property_ui_text(prop, "", "Set Controller state index (1 to 30)");
	RNA_def_property_int_funcs(prop, "rna_Controller_state_number_get", "rna_Controller_state_number_set", NULL);
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	/* Expression Controller */
	srna= RNA_def_struct(brna, "ExpressionController", "Controller");
	RNA_def_struct_sdna_from(srna, "bExpressionCont", "data");
	RNA_def_struct_ui_text(srna, "Expression Controller", "Controller passing on events based on the evaluation of an expression");

	prop= RNA_def_property(srna, "expression", PROP_STRING, PROP_NONE);
	RNA_def_property_string_sdna(prop, NULL, "str");
	RNA_def_property_string_maxlength(prop, 127);
	RNA_def_property_ui_text(prop, "Expression", "");
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	/* Python Controller */
	srna= RNA_def_struct(brna, "PythonController", "Controller" );
	RNA_def_struct_sdna_from(srna, "bPythonCont", "data");
	RNA_def_struct_ui_text(srna, "Python Controller", "Controller executing a python script");

	prop= RNA_def_property(srna, "mode", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, python_controller_modes);
	RNA_def_property_ui_text(prop, "Execution Method", "Python script type (textblock or module - faster)");
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	prop= RNA_def_property(srna, "text", PROP_POINTER, PROP_NONE);
	RNA_def_property_struct_type(prop, "Text");
	RNA_def_property_flag(prop, PROP_EDITABLE);
	RNA_def_property_ui_text(prop, "Text", "Text datablock with the python script");
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	prop= RNA_def_property(srna, "module", PROP_STRING, PROP_NONE);
	RNA_def_property_ui_text(prop, "Module", "Module name and function to run e.g. \"someModule.main\". Internal texts and external python files can be used");
	RNA_def_struct_name_property(srna, prop);
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	prop= RNA_def_property(srna, "debug", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", CONT_PY_DEBUG);
	RNA_def_property_ui_text(prop, "D", "Continuously reload the module from disk for editing external modules without restarting");
	RNA_def_property_update(prop, NC_LOGIC, NULL);

	/* Other Controllers */
	srna= RNA_def_struct(brna, "AndController", "Controller");
	RNA_def_struct_ui_text(srna, "And Controller", "Controller passing on events based on a logical AND operation");
	
	srna= RNA_def_struct(brna, "OrController", "Controller");
	RNA_def_struct_ui_text(srna, "Or Controller", "Controller passing on events based on a logical OR operation");
	
	srna= RNA_def_struct(brna, "NorController", "Controller");
	RNA_def_struct_ui_text(srna, "Nor Controller", "Controller passing on events based on a logical NOR operation");
	
	srna= RNA_def_struct(brna, "NandController", "Controller");
	RNA_def_struct_ui_text(srna, "Nand Controller", "Controller passing on events based on a logical NAND operation");
	
	srna= RNA_def_struct(brna, "XorController", "Controller");
	RNA_def_struct_ui_text(srna, "Xor Controller", "Controller passing on events based on a logical XOR operation");
	
	srna= RNA_def_struct(brna, "XnorController", "Controller");
	RNA_def_struct_ui_text(srna, "Xnor Controller", "Controller passing on events based on a logical XNOR operation");
}

#endif

