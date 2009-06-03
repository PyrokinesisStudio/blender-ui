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
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Contributor(s): Blender Foundation (2008).
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <float.h>
#include <stdlib.h>

#include "RNA_define.h"
#include "RNA_types.h"

#include "rna_internal.h"

#include "DNA_material_types.h"
#include "DNA_texture_types.h"

#ifdef RNA_RUNTIME

StructRNA *rna_Texture_refine(struct PointerRNA *ptr)
{
	Tex *tex= (Tex*)ptr->data;

	switch(tex->type) {
		case TEX_CLOUDS:
			return &RNA_CloudsTexture;
		case TEX_WOOD:
			return &RNA_WoodTexture;
		case TEX_MARBLE:
			return &RNA_MarbleTexture;
		case TEX_MAGIC:
			return &RNA_MagicTexture;
		case TEX_BLEND:
			return &RNA_BlendTexture; 
		case TEX_STUCCI:
			return &RNA_StucciTexture;
		case TEX_NOISE:
			return &RNA_NoiseTexture;
		case TEX_IMAGE:
			return &RNA_ImageTexture;
		case TEX_PLUGIN:
			return &RNA_PluginTexture;
		case TEX_ENVMAP:
			return &RNA_EnvironmentMapTexture;
		case TEX_MUSGRAVE:
			return &RNA_MusgraveTexture;
		case TEX_VORONOI:
			return &RNA_VoronoiTexture;
		case TEX_DISTNOISE:
			return &RNA_DistortedNoiseTexture;
		default:
			return &RNA_Texture;
	}
}

#else

static void rna_def_color_ramp_element(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	srna= RNA_def_struct(brna, "ColorRampElement", NULL);
	RNA_def_struct_sdna(srna, "CBData");
	RNA_def_struct_ui_text(srna, "Color Ramp Element", "Element defining a color at a position in the color ramp.");

	prop= RNA_def_property(srna, "color", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "r");
	RNA_def_property_array(prop, 4);
	RNA_def_property_ui_text(prop, "Color", "");

	prop= RNA_def_property(srna, "position", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "pos");
	RNA_def_property_range(prop, 0, 1);
	RNA_def_property_ui_text(prop, "Position", "");
}

static void rna_def_color_ramp(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_interpolation_items[] = {
		{1, "EASE", "Ease", ""},
		{3, "CARDINAL", "Cardinal", ""},
		{0, "LINEAR", "Linear", ""},
		{2, "B_SPLINE", "B-Spline", ""},
		{4, "CONSTANT", "Constant", ""},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "ColorRamp", NULL);
	RNA_def_struct_sdna(srna, "ColorBand");
	RNA_def_struct_ui_text(srna, "Color Ramp", "Color ramp mapping a scalar value to a color.");

	prop= RNA_def_property(srna, "elements", PROP_COLLECTION, PROP_COLOR);
	RNA_def_property_collection_sdna(prop, NULL, "data", "tot");
	RNA_def_property_struct_type(prop, "ColorRampElement");
	RNA_def_property_ui_text(prop, "Elements", "");

	prop= RNA_def_property(srna, "interpolation", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "ipotype");
	RNA_def_property_enum_items(prop, prop_interpolation_items);
	RNA_def_property_ui_text(prop, "Interpolation", "");
}

static void rna_def_texmapping(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;
	
	srna= RNA_def_struct(brna, "TexMapping", NULL);
	RNA_def_struct_ui_text(srna, "Texture Mapping", "Mapping settings");

	prop= RNA_def_property(srna, "location", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "loc");
	RNA_def_property_ui_text(prop, "Location", "");
	
	prop= RNA_def_property(srna, "rotation", PROP_FLOAT, PROP_ROTATION);
	RNA_def_property_float_sdna(prop, NULL, "rot");
	RNA_def_property_ui_text(prop, "Rotation", "");
	
	prop= RNA_def_property(srna, "scale", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "size");
	RNA_def_property_ui_text(prop, "Scale", "");
	
	prop= RNA_def_property(srna, "minimum", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "min");
	RNA_def_property_ui_text(prop, "Minimum", "Minimum value for clipping");
	
	prop= RNA_def_property(srna, "maximum", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "max");
	RNA_def_property_ui_text(prop, "Maximum", "Maximum value for clipping");
	
	prop= RNA_def_property(srna, "has_minimum", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEXMAP_CLIP_MIN);
	RNA_def_property_ui_text(prop, "Has Minimum", "Whether to use minimum clipping value");
	
	prop= RNA_def_property(srna, "has_maximum", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEXMAP_CLIP_MAX);
	RNA_def_property_ui_text(prop, "Has Maximum", "Whether to use maximum clipping value");
}

static void rna_def_mtex(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_blend_type_items[] = {
		{MTEX_BLEND, "MIX", "Mix", ""},
		{MTEX_ADD, "ADD", "Add", ""},
		{MTEX_SUB, "SUBTRACT", "Subtract", ""},
		{MTEX_MUL, "MULTIPLY", "Multiply", ""},
		{MTEX_SCREEN, "SCREEN", "Screen", ""},
		{MTEX_OVERLAY, "OVERLAY", "Overlay", ""},
		{MTEX_DIFF, "DIFFERENCE", "Difference", ""},
		{MTEX_DIV, "DIVIDE", "Divide", ""},
		{MTEX_DARK, "DARKEN", "Darken", ""},
		{MTEX_LIGHT, "LIGHTEN", "Lighten", ""},
		{MTEX_BLEND_HUE, "HUE", "Hue", ""},
		{MTEX_BLEND_SAT, "SATURATION", "Saturation", ""},
		{MTEX_BLEND_VAL, "VALUE", "Value", ""},
		{MTEX_BLEND_COLOR, "COLOR", "Color", ""},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "TextureSlot", NULL);
	RNA_def_struct_sdna(srna, "MTex");
	RNA_def_struct_ui_text(srna, "Texture Slot", "Texture slot defining the mapping and influence of a texture.");

	prop= RNA_def_property(srna, "texture", PROP_POINTER, PROP_NONE);
	RNA_def_property_pointer_sdna(prop, NULL, "tex");
	RNA_def_property_struct_type(prop, "Texture");
	RNA_def_property_flag(prop, PROP_EDITABLE);
	RNA_def_property_ui_text(prop, "Texture", "Texture datablock used by this texture slot.");

	/* mapping */
	prop= RNA_def_property(srna, "offset", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "ofs");
	RNA_def_property_ui_range(prop, -10, 10, 10, 2);
	RNA_def_property_ui_text(prop, "Offset", "Fine tunes texture mapping X, Y and Z locations.");

	prop= RNA_def_property(srna, "size", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_ui_range(prop, -100, 100, 10, 2);
	RNA_def_property_ui_text(prop, "Size", "Sets scaling for the texture's X, Y and Z sizes.");

	prop= RNA_def_property(srna, "color", PROP_FLOAT, PROP_COLOR);
	RNA_def_property_float_sdna(prop, NULL, "r");
	RNA_def_property_array(prop, 3);
	RNA_def_property_ui_text(prop, "Color", "The default color for textures that don't return RGB.");

	prop= RNA_def_property(srna, "blend_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "blendtype");
	RNA_def_property_enum_items(prop, prop_blend_type_items);
	RNA_def_property_ui_text(prop, "Blend Type", "");

	prop= RNA_def_property(srna, "stencil", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "texflag", MTEX_STENCIL);
	RNA_def_property_ui_text(prop, "Stencil", "Use this texture as a blending value on the next texture.");

	prop= RNA_def_property(srna, "negate", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "texflag", MTEX_NEGATIVE);
	RNA_def_property_ui_text(prop, "Negate", "Inverts the values of the texture to reverse its effect.");

	prop= RNA_def_property(srna, "no_rgb", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "texflag", MTEX_RGBTOINT);
	RNA_def_property_ui_text(prop, "No RGB", "Converts texture RGB values to intensity (gray) values.");

	prop= RNA_def_property(srna, "default_value", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "def_var");
	RNA_def_property_range(prop, 0, 1);
	RNA_def_property_ui_text(prop, "Default Value", "Value to use for Ref, Spec, Amb, Emit, Alpha, RayMir, TransLu and Hard.");

	prop= RNA_def_property(srna, "variable_factor", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "varfac");
	RNA_def_property_range(prop, 0, 1);
	RNA_def_property_ui_text(prop, "Variable Factor", "Amount texture affects other values.");

	prop= RNA_def_property(srna, "color_factor", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "colfac");
	RNA_def_property_range(prop, 0, 1);
	RNA_def_property_ui_text(prop, "Color Factor", "Amount texture affects color values.");

	prop= RNA_def_property(srna, "normal_factor", PROP_FLOAT, PROP_VECTOR);
	RNA_def_property_float_sdna(prop, NULL, "norfac");
	RNA_def_property_range(prop, 0, 25);
	RNA_def_property_ui_text(prop, "Normal Factor", "Amount texture affects normal values.");
}

static void rna_def_filter_size_common(StructRNA *srna) 
{
	PropertyRNA *prop;

	/* XXX: not sure about the name of this, "Min" seems a bit off */
	prop= RNA_def_property(srna, "use_filter", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_FILTER_MIN);
	RNA_def_property_ui_text(prop, "Use Filter", "Use Filter Size as a minimal filter value in pixels");

	prop= RNA_def_property(srna, "filter_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "filtersize");
	RNA_def_property_range(prop, 0.1, 50.0);
	RNA_def_property_ui_range(prop, 0.1, 50.0, 1, 0.2);
	RNA_def_property_ui_text(prop, "Filter Size", "Multiplies the filter size used by MIP Map and Interpolation");
}

static void rna_def_environment_map_common(StructRNA *srna)
{
	PropertyRNA *prop;

	static EnumPropertyItem prop_source_items[] = {
		{ENV_STATIC, "STATIC", "Static", "Calculates environment map only once"},
		{ENV_ANIM, "ANIMATED", "Animated", "Calculates environment map at each rendering"},
		{ENV_LOAD, "LOADED", "Loaded", "Loads saved environment map from disk"},
		{0, NULL, NULL, NULL}};

	prop= RNA_def_property(srna, "source", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "stype");
	RNA_def_property_enum_items(prop, prop_source_items);
	RNA_def_property_ui_text(prop, "Source", "");

	/* XXX: move this to specific types if needed */
	prop= RNA_def_property(srna, "image", PROP_POINTER, PROP_NONE);
	RNA_def_property_pointer_sdna(prop, NULL, "ima");
	RNA_def_property_struct_type(prop, "Image");
	RNA_def_property_ui_text(prop, "Image", "");
}

static void rna_def_environment_map(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_type_items[] = {
		{ENV_CUBE, "CUBE", "Cube", "Use environment map with six cube sides."},
		{ENV_PLANE, "PLANE", "Plane", "Only one side is rendered, with Z axis pointing in direction of image."},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "EnvironmentMap", NULL);
	RNA_def_struct_sdna(srna, "EnvMap");
	RNA_def_struct_ui_text(srna, "EnvironmentMap", "Environment map created by the renderer and cached for subsequent renders.");

	rna_def_environment_map_common(srna);

	prop= RNA_def_property(srna, "type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "type");
	RNA_def_property_enum_items(prop, prop_type_items);
	RNA_def_property_ui_text(prop, "Type", "");

	prop= RNA_def_property(srna, "clip_start", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "clipsta");
	RNA_def_property_range(prop, 0.01, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.01, 50, 100, 2);
	RNA_def_property_ui_text(prop, "Clip Start", "Objects nearer than this are not visible to map.");

	prop= RNA_def_property(srna, "clip_end", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "clipend");
	RNA_def_property_range(prop, 0.01, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.10, 20000, 100, 2);
	RNA_def_property_ui_text(prop, "Clip End", "Objects further than this are not visible to map.");

	prop= RNA_def_property(srna, "zoom", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "viewscale");
	RNA_def_property_range(prop, 0.01, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.5, 5, 100, 2);
	RNA_def_property_ui_text(prop, "Zoom", "");

	/* XXX: EnvMap.notlay */
	
	prop= RNA_def_property(srna, "resolution", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "cuberes");
	RNA_def_property_range(prop, 50, 4096);
	RNA_def_property_ui_text(prop, "Resolution", "Pixel resolution of the rendered environment map.");

	prop= RNA_def_property(srna, "depth", PROP_INT, PROP_NONE);
	RNA_def_property_range(prop, 0, 5);
	RNA_def_property_ui_text(prop, "Depth", "Number of times a map will be rendered recursively (mirror effects.)");
}

static EnumPropertyItem prop_noise_basis_items[] = {
	{TEX_BLENDER, "BLENDER_ORIGINAL", "Blender Original", ""},
	{TEX_STDPERLIN, "ORIGINAL_PERLIN", "Original Perlin", ""},
	{TEX_NEWPERLIN, "IMPROVED_PERLIN", "Improved Perlin", ""},
	{TEX_VORONOI_F1, "VORONOI_F1", "Voronoi F1", ""},
	{TEX_VORONOI_F2, "VORONOI_F2", "Voronoi F2", ""},
	{TEX_VORONOI_F3, "VORONOI_F3", "Voronoi F3", ""},
	{TEX_VORONOI_F4, "VORONOI_F4", "Voronoi F4", ""},
	{TEX_VORONOI_F2F1, "VORONOI_F2_F1", "Voronoi F2-F1", ""},
	{TEX_VORONOI_CRACKLE, "VORONOI_CRACKLE", "Voronoi Crackle", ""},
	{TEX_CELLNOISE, "CELL_NOISE", "Cell Noise", ""},
	{0, NULL, NULL, NULL}};

static EnumPropertyItem prop_noise_type[] = {
	{TEX_NOISESOFT, "SOFT_NOISE", "Soft", ""},
	{TEX_NOISEPERL, "HARD_NOISE", "Hard", ""},
	{0, NULL, NULL, NULL}};


static void rna_def_texture_clouds(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_clouds_stype[] = {
	{TEX_DEFAULT, "GREYSCALE", "Greyscale", ""},
	{TEX_COLOR, "COLOR", "Color", ""},
	{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "CloudsTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Clouds Texture", "Procedural noise texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "noise_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "noisesize");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 2, 10, 2);
	RNA_def_property_ui_text(prop, "Noise Size", "Sets scaling for noise input");

	prop= RNA_def_property(srna, "noise_depth", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "noisedepth");
	RNA_def_property_range(prop, 0, INT_MAX);
	RNA_def_property_ui_range(prop, 0, 6, 0, 2);
	RNA_def_property_ui_text(prop, "Noise Depth", "Sets the depth of the cloud calculation");


	prop= RNA_def_property(srna, "noise_basis", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis");
	RNA_def_property_enum_items(prop, prop_noise_basis_items);
	RNA_def_property_ui_text(prop, "Noise Basis", "Sets the noise basis used for turbulence");

	prop= RNA_def_property(srna, "noise_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisetype");
	RNA_def_property_enum_items(prop, prop_noise_type);
	RNA_def_property_ui_text(prop, "Noise Type", "");

	prop= RNA_def_property(srna, "stype", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "stype");
	RNA_def_property_enum_items(prop, prop_clouds_stype);
	RNA_def_property_ui_text(prop, "Color", "");

	prop= RNA_def_property(srna, "nabla", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.001, 0.1);
	RNA_def_property_ui_range(prop, 0.001, 0.1, 1, 2);
	RNA_def_property_ui_text(prop, "Nabla", "Size of derivative offset used for calculating normal");
}

static void rna_def_texture_wood(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_wood_stype[] = {
	{TEX_BAND, "BANDS", "Bands", "Uses standard wood texture in bands"},
	{TEX_RING, "RINGS", "Rings", "Uses wood texture in rings"},
	{TEX_BANDNOISE, "BANDNOISE", "Band Noise", "Adds noise to standard wood"},
	{TEX_RINGNOISE, "RINGNOISE", "Ring Noise", "Adds noise to rings"},
	{0, NULL, NULL, NULL}};

	static EnumPropertyItem prop_wood_noisebasis2[] = {
	{TEX_SIN, "SIN", "Sine", "Uses a sine wave to produce bands"},
	{TEX_SAW, "SAW", "Saw", "Uses a saw wave to produce bands"},
	{TEX_TRI, "TRI", "Tri", "Uses a triangle wave to produce bands"},
	{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "WoodTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Wood Texture", "Procedural noise texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "noise_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "noisesize");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 2, 10, 2);
	RNA_def_property_ui_text(prop, "Noise Size", "Sets scaling for noise input");

	prop= RNA_def_property(srna, "turbulence", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "turbul");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 200, 10, 2);
	RNA_def_property_ui_text(prop, "Turbulence", "Sets the turbulence of the bandnoise and ringnoise types");

	prop= RNA_def_property(srna, "noise_basis", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis");
	RNA_def_property_enum_items(prop, prop_noise_basis_items);
	RNA_def_property_ui_text(prop, "Noise Basis", "Sets the noise basis used for turbulence");

	prop= RNA_def_property(srna, "noise_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisetype");
	RNA_def_property_enum_items(prop, prop_noise_type);
	RNA_def_property_ui_text(prop, "Noise Type", "");

	prop= RNA_def_property(srna, "stype", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "stype");
	RNA_def_property_enum_items(prop, prop_wood_stype);
	RNA_def_property_ui_text(prop, "Pattern", "");

	prop= RNA_def_property(srna, "noisebasis2", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis2");
	RNA_def_property_enum_items(prop, prop_wood_noisebasis2);
	RNA_def_property_ui_text(prop, "Noise Basis 2", "");

	prop= RNA_def_property(srna, "nabla", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.001, 0.1);
	RNA_def_property_ui_range(prop, 0.001, 0.1, 1, 2);
	RNA_def_property_ui_text(prop, "Nabla", "Size of derivative offset used for calculating normal.");

}

static void rna_def_texture_marble(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_marble_stype[] = {
	{TEX_SOFT, "SOFT", "Soft", "Uses soft marble"},
	{TEX_SHARP, "SHARP", "Sharp", "Uses more clearly defined marble"},
	{TEX_SHARPER, "SHARPER", "Sharper", "Uses very clearly defined marble"},
	{0, NULL, NULL, NULL}};

	static EnumPropertyItem prop_marble_noisebasis2[] = {
	{TEX_SIN, "SIN", "Sin", "Uses a sine wave to produce bands"},
	{TEX_SAW, "SAW", "Saw", "Uses a saw wave to produce bands"},
	{TEX_TRI, "TRI", "Tri", "Uses a triangle wave to produce bands"},
	{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "MarbleTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Marble Texture", "Procedural noise texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "noise_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "noisesize");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 2, 10, 2);
	RNA_def_property_ui_text(prop, "Noise Size", "Sets scaling for noise input");

	prop= RNA_def_property(srna, "turbulence", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "turbul");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 200, 10, 2);
	RNA_def_property_ui_text(prop, "Turbulence", "Sets the turbulence of the bandnoise and ringnoise types");

	prop= RNA_def_property(srna, "noise_depth", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "noisedepth");
	RNA_def_property_range(prop, 0, INT_MAX);
	RNA_def_property_ui_range(prop, 0, 6, 0, 2);
	RNA_def_property_ui_text(prop, "Noise Depth", "Sets the depth of the cloud calculation");

	prop= RNA_def_property(srna, "noise_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisetype");
	RNA_def_property_enum_items(prop, prop_noise_type);
	RNA_def_property_ui_text(prop, "Noise Type", "");

	prop= RNA_def_property(srna, "stype", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "stype");
	RNA_def_property_enum_items(prop, prop_marble_stype);
	RNA_def_property_ui_text(prop, "Pattern", "");
	
	prop= RNA_def_property(srna, "noise_basis", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis");
	RNA_def_property_enum_items(prop, prop_noise_basis_items);
	RNA_def_property_ui_text(prop, "Noise Basis", "Sets the noise basis used for turbulence");

	prop= RNA_def_property(srna, "noisebasis2", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis2");
	RNA_def_property_enum_items(prop, prop_marble_noisebasis2);
	RNA_def_property_ui_text(prop, "Noise Basis 2", "");

	prop= RNA_def_property(srna, "nabla", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.001, 0.1);
	RNA_def_property_ui_range(prop, 0.001, 0.1, 1, 2);
	RNA_def_property_ui_text(prop, "Nabla", "Size of derivative offset used for calculating normal.");

}

static void rna_def_texture_magic(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	srna= RNA_def_struct(brna, "MagicTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Magic Texture", "Procedural noise texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "turbulence", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "turbul");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 200, 10, 2);
	RNA_def_property_ui_text(prop, "Turbulence", "Sets the turbulence of the bandnoise and ringnoise types");

	prop= RNA_def_property(srna, "noise_depth", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "noisedepth");
	RNA_def_property_range(prop, 0, INT_MAX);
	RNA_def_property_ui_range(prop, 0, 6, 0, 2);
	RNA_def_property_ui_text(prop, "Noise Depth", "Sets the depth of the cloud calculation");
}

static void rna_def_texture_blend(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_blend_progression[] = {
		{TEX_LIN, "LINEAR", "Linear", "Creates a linear progression"},
		{TEX_QUAD, "QUADRATIC", "Quadratic", "Creates a quadratic progression"},
		{TEX_EASE, "EASING", "Easing", "Creates a progression easing from one step to the next"},
		{TEX_DIAG, "DIAGONAL", "Diagonal", "Creates a diagonal progression"},
		{TEX_SPHERE, "SPHERICAL", "Spherical", "Creates a spherical progression"},
		{TEX_HALO, "QUADRATIC_SPHERE", "Quadratic sphere", "Creates a quadratic progression in the shape of a sphere"},
		{TEX_RAD, "RADIAL", "Radial", "Creates a radial progression"},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "BlendTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Blend Texture", "Procedural color blending texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "progression", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "stype");
	RNA_def_property_enum_items(prop, prop_blend_progression);
	RNA_def_property_ui_text(prop, "Progression", "Sets the style of the color blending");

	prop= RNA_def_property(srna, "flip_axis", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEX_FLIPBLEND);
	RNA_def_property_ui_text(prop, "Flip Axis", "Flips the texture's X and Y axis");
}

static void rna_def_texture_stucci(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_stucci_stype[] = {
	{TEX_PLASTIC, "PLASTIC", "Plastic", "Uses standard stucci"},
	{TEX_WALLIN, "WALL_IN", "Wall in", "Creates Dimples"},
	{TEX_WALLOUT, "WALL_OUT", "Wall out", "Creates Ridges"},
	{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "StucciTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Stucci Texture", "Procedural noise texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "turbulence", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "turbul");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 200, 10, 2);
	RNA_def_property_ui_text(prop, "Turbulence", "Sets the turbulence of the bandnoise and ringnoise types");
	
	prop= RNA_def_property(srna, "noise_basis", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis");
	RNA_def_property_enum_items(prop, prop_noise_basis_items);
	RNA_def_property_ui_text(prop, "Noise Basis", "Sets the noise basis used for turbulence");

	prop= RNA_def_property(srna, "noise_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "noisesize");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 2, 10, 2);
	RNA_def_property_ui_text(prop, "Noise Size", "Sets scaling for noise input");

	prop= RNA_def_property(srna, "noise_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisetype");
	RNA_def_property_enum_items(prop, prop_noise_type);
	RNA_def_property_ui_text(prop, "Noise Type", "");

	prop= RNA_def_property(srna, "stype", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "stype");
	RNA_def_property_enum_items(prop, prop_stucci_stype);
	RNA_def_property_ui_text(prop, "Pattern", "");
}

static void rna_def_texture_noise(BlenderRNA *brna)
{
	StructRNA *srna;

	srna= RNA_def_struct(brna, "NoiseTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Noise Texture", "Procedural noise texture.");
	RNA_def_struct_sdna(srna, "Tex");
}

static void rna_def_texture_image(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_image_extension[] = {
		{1, "EXTEND", "Extend", "Extends by repeating edge pixels of the image"},
		{2, "CLIP", "Clip", "Clips to image size and sets exterior pixels as transparent"},
		{4, "CLIP_CUBE", "Clip Cube", "Clips to cubic-shaped area around the image and sets exterior pixels as transparent"},
		{3, "REPEAT", "Repeat", "Causes the image to repeat horizontally and vertically"},
		{5, "CHECKER", "Checker", "Causes the image to repeat in checker board pattern"},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "ImageTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Image Texture", "");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "mipmap", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_MIPMAP);
	RNA_def_property_ui_text(prop, "MIP Map", "Uses auto-generated MIP maps for the image");

	prop= RNA_def_property(srna, "mipmap_gauss", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_GAUSS_MIP);
	RNA_def_property_ui_text(prop, "MIP Map Gauss", "Uses Gauss filter to sample down MIP maps");

	prop= RNA_def_property(srna, "interpolation", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_INTERPOL);
	RNA_def_property_ui_text(prop, "Interpolation", "Interpolates pixels using Area filter");

	/* XXX: I think flip_axis should be a generic Texture property, enabled for all the texture types */
	prop= RNA_def_property(srna, "flip_axis", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_IMAROT);
	RNA_def_property_ui_text(prop, "Flip Axis", "Flips the texture's X and Y axis");

	prop= RNA_def_property(srna, "use_alpha", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_USEALPHA);
	RNA_def_property_ui_text(prop, "Use Alpha", "Uses the alpha channel information in the image");

	prop= RNA_def_property(srna, "calculate_alpha", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_CALCALPHA);
	RNA_def_property_ui_text(prop, "Calculate Alpha", "Calculates an alpha channel based on RGB values in the image");

	prop= RNA_def_property(srna, "invert_alpha", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEX_NEGALPHA);
	RNA_def_property_ui_text(prop, "Invert Alpha", "Inverts all the alpha values in the image");

	rna_def_filter_size_common(srna);

	prop= RNA_def_property(srna, "normal_map", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "imaflag", TEX_NORMALMAP);
	RNA_def_property_ui_text(prop, "Normal Map", "Uses image RGB values for normal mapping");

	/* XXX: mtex->normapspace "Sets space of normal map image" "Normal Space %t|Camera %x0|World %x1|Object %x2|Tangent %x3" 
	 *			not sure why this goes in mtex instead of texture directly? */

	prop= RNA_def_property(srna, "extension", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "extend");
	RNA_def_property_enum_items(prop, prop_image_extension);
	RNA_def_property_ui_text(prop, "Extension", "Sets how the image is stretched in the texture");

	prop= RNA_def_property(srna, "repeat_x", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "xrepeat");
	RNA_def_property_range(prop, 1, 512);
	RNA_def_property_ui_text(prop, "Repeat X", "Sets a repetition multiplier in the X direction");

	prop= RNA_def_property(srna, "repeat_y", PROP_INT, PROP_NONE);
	RNA_def_property_int_sdna(prop, NULL, "yrepeat");
	RNA_def_property_range(prop, 1, 512);
	RNA_def_property_ui_text(prop, "Repeat Y", "Sets a repetition multiplier in the Y direction");

	prop= RNA_def_property(srna, "mirror_x", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEX_REPEAT_XMIR);
	RNA_def_property_ui_text(prop, "Mirror X", "Mirrors the image repetition on the X direction");

	prop= RNA_def_property(srna, "mirror_y", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEX_REPEAT_YMIR);
	RNA_def_property_ui_text(prop, "Mirror Y", "Mirrors the image repetition on the Y direction");

	prop= RNA_def_property(srna, "checker_odd", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEX_CHECKER_ODD);
	RNA_def_property_ui_text(prop, "Checker Odd", "Sets odd checker tiles");

	prop= RNA_def_property(srna, "checker_even", PROP_BOOLEAN, PROP_NONE);
	RNA_def_property_boolean_sdna(prop, NULL, "flag", TEX_CHECKER_EVEN);
	RNA_def_property_ui_text(prop, "Checker Even", "Sets even checker tiles");

	prop= RNA_def_property(srna, "checker_distance", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "checkerdist");
	RNA_def_property_range(prop, 0.0, 0.99);
	RNA_def_property_ui_range(prop, 0.0, 0.99, 0.1, 0.01);
	RNA_def_property_ui_text(prop, "Checker Distance", "Sets distance between checker tiles");

#if 0

	/* XXX: did this as an array, but needs better descriptions than "1 2 3 4"
	perhaps a new subtype could be added? 
	--I actually used single values for this, maybe change later with a RNA_Rect thing? */
	prop= RNA_def_property(srna, "crop_rectangle", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "cropxmin");
	RNA_def_property_array(prop, 4);
	RNA_def_property_range(prop, -10, 10);
	RNA_def_property_ui_text(prop, "Crop Rectangle", "");

#endif

	prop= RNA_def_property(srna, "crop_min_x", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "cropxmin");
	RNA_def_property_range(prop, -10.0, 10.0);
	RNA_def_property_ui_range(prop, -10.0, 10.0, 1, 0.2);
	RNA_def_property_ui_text(prop, "Crop Minimum X", "Sets minimum X value to crop the image");

	prop= RNA_def_property(srna, "crop_min_y", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "cropymin");
	RNA_def_property_range(prop, -10.0, 10.0);
	RNA_def_property_ui_range(prop, -10.0, 10.0, 1, 0.2);
	RNA_def_property_ui_text(prop, "Crop Minimum Y", "Sets minimum Y value to crop the image");

	prop= RNA_def_property(srna, "crop_max_x", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "cropxmax");
	RNA_def_property_range(prop, -10.0, 10.0);
	RNA_def_property_ui_range(prop, -10.0, 10.0, 1, 0.2);
	RNA_def_property_ui_text(prop, "Crop Maximum X", "Sets maximum X value to crop the image");

	prop= RNA_def_property(srna, "crop_max_y", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "cropymax");
	RNA_def_property_range(prop, -10.0, 10.0);
	RNA_def_property_ui_range(prop, -10.0, 10.0, 1, 0.2);
	RNA_def_property_ui_text(prop, "Crop Maximum Y", "Sets maximum Y value to crop the image");

	prop= RNA_def_property(srna, "image", PROP_POINTER, PROP_NONE);
	RNA_def_property_pointer_sdna(prop, NULL, "ima");
	RNA_def_property_struct_type(prop, "Image");
	RNA_def_property_flag(prop, PROP_EDITABLE);
	RNA_def_property_ui_text(prop, "Image", "");
}

static void rna_def_texture_plugin(BlenderRNA *brna)
{
	StructRNA *srna;

	srna= RNA_def_struct(brna, "PluginTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Plugin", "External plugin texture.");
	RNA_def_struct_sdna(srna, "Tex");

	/* XXX: todo */
}

static void rna_def_texture_environment_map(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	srna= RNA_def_struct(brna, "EnvironmentMapTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Environment Map", "Environment map texture.");
	RNA_def_struct_sdna(srna, "Tex");

	rna_def_environment_map_common(srna);

	prop= RNA_def_property(srna, "environment_map", PROP_POINTER, PROP_NONE);
	RNA_def_property_pointer_sdna(prop, NULL, "env");
	RNA_def_property_struct_type(prop, "EnvironmentMap");
	RNA_def_property_ui_text(prop, "Environment Map", "Gets the environment map associated with this texture");

	rna_def_filter_size_common(srna);
}

static void rna_def_texture_musgrave(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_musgrave_type[] = {
		{TEX_MFRACTAL, "MULTIFRACTAL", "Multifractal", ""},
		{TEX_RIDGEDMF, "RIDGED_MULTIFRACTAL", "Ridged Multifractal", ""},
		{TEX_HYBRIDMF, "HYBRID_MULTIFRACTAL", "Hybrid Multifractal", ""},
		{TEX_FBM, "FBM", "fBM", ""},
		{TEX_HTERRAIN, "HETERO_TERRAIN", "Hetero Terrain", ""},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "MusgraveTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Musgrave", "Procedural musgrave texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "musgrave_type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "stype");
	RNA_def_property_enum_items(prop, prop_musgrave_type);
	RNA_def_property_ui_text(prop, "Type", "");

	prop= RNA_def_property(srna, "highest_dimension", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "mg_H");
	RNA_def_property_range(prop, 0.0001, 2);
	RNA_def_property_ui_text(prop, "Highest Dimension", "Highest fractal dimension");

	prop= RNA_def_property(srna, "lacunarity", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "mg_lacunarity");
	RNA_def_property_range(prop, 0, 6);
	RNA_def_property_ui_text(prop, "Lacunarity", "Gap between succesive frequencies");

	prop= RNA_def_property(srna, "octaves", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "mg_octaves");
	RNA_def_property_range(prop, 0, 8);
	RNA_def_property_ui_text(prop, "Octaves", "Number of frequencies used");

	prop= RNA_def_property(srna, "offset", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "mg_offset");
	RNA_def_property_range(prop, 0, 6);
	RNA_def_property_ui_text(prop, "Offset", "The fractal offset");

	prop= RNA_def_property(srna, "gain", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "mg_gain");
	RNA_def_property_range(prop, 0, 6);
	RNA_def_property_ui_text(prop, "Gain", "The gain multiplier");

	prop= RNA_def_property(srna, "noise_intensity", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "ns_outscale");
	RNA_def_property_range(prop, 0, 10);
	RNA_def_property_ui_text(prop, "Noise Intensity", "");

	prop= RNA_def_property(srna, "noise_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "noisesize");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 2, 10, 2);
	RNA_def_property_ui_text(prop, "Noise Size", "Sets scaling for noise input");

	prop= RNA_def_property(srna, "noise_basis", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis");
	RNA_def_property_enum_items(prop, prop_noise_basis_items);
	RNA_def_property_ui_text(prop, "Noise Basis", "Sets the noise basis used for turbulence");

	prop= RNA_def_property(srna, "nabla", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.001, 0.1);
	RNA_def_property_ui_range(prop, 0.001, 0.1, 1, 2);
	RNA_def_property_ui_text(prop, "Nabla", "Size of derivative offset used for calculating normal");
}

static void rna_def_texture_voronoi(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_distance_metric_items[] = {
		{TEX_DISTANCE, "DISTANCE", "Actual Distance", ""},
		{TEX_DISTANCE_SQUARED, "DISTANCE_SQUARED", "Distance Squared", ""},
		{TEX_MANHATTAN, "MANHATTAN", "Manhattan", ""},
		{TEX_CHEBYCHEV, "CHEBYCHEV", "Chebychev", ""},
		{TEX_MINKOVSKY_HALF, "MINKOVSKY_HALF", "Minkovsky 1/2", ""},
		{TEX_MINKOVSKY_FOUR, "MINKOVSKY_FOUR", "Minkovsky 4", ""},
		{TEX_MINKOVSKY, "MINKOVSKY", "Minkovsky", ""},
		{0, NULL, NULL, NULL}};

	static EnumPropertyItem prop_coloring_items[] = {
		/* XXX: OK names / descriptions? */
		{TEX_INTENSITY, "INTENSITY", "Intensity", "Only calculate intensity."},
		{TEX_COL1, "POSITION", "Position", "Color cells by position."},
		{TEX_COL2, "POSITION_OUTLINE", "Position and Outline", "Use position plus an outline based on F2-F.1"},
		{TEX_COL3, "POSITION_OUTLINE_INTENSITY", "Position, Outline, and Intensity", "Multiply position and outline by intensity."},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "VoronoiTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Voronoi", "Procedural voronoi texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "feature_weights", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "vn_w1");
	RNA_def_property_array(prop, 4);
	RNA_def_property_range(prop, -2, 2);
	RNA_def_property_ui_text(prop, "Feature Weights", "");

	prop= RNA_def_property(srna, "minkovsky_exponent", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "vn_mexp");
	RNA_def_property_range(prop, 0.01, 10);
	RNA_def_property_ui_text(prop, "Minkovsky Exponent", "");

	prop= RNA_def_property(srna, "distance_metric", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "vn_distm");
	RNA_def_property_enum_items(prop, prop_distance_metric_items);
	RNA_def_property_ui_text(prop, "Distance Metric", "");

	prop= RNA_def_property(srna, "coloring", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "vn_coltype");
	RNA_def_property_enum_items(prop, prop_coloring_items);
	RNA_def_property_ui_text(prop, "Coloring", "");

	prop= RNA_def_property(srna, "noise_intensity", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "ns_outscale");
	RNA_def_property_range(prop, 0.01, 10);
	RNA_def_property_ui_text(prop, "Noise Intensity", "");

	prop= RNA_def_property(srna, "noise_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "noisesize");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 2, 10, 2);
	RNA_def_property_ui_text(prop, "Noise Size", "Sets scaling for noise input");

	prop= RNA_def_property(srna, "nabla", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.001, 0.1);
	RNA_def_property_ui_range(prop, 0.001, 0.1, 1, 2);
	RNA_def_property_ui_text(prop, "Nabla", "Size of derivative offset used for calculating normal");
}

static void rna_def_texture_distorted_noise(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	srna= RNA_def_struct(brna, "DistortedNoiseTexture", "Texture");
	RNA_def_struct_ui_text(srna, "Distorted Noise", "Procedural distorted noise texture.");
	RNA_def_struct_sdna(srna, "Tex");

	prop= RNA_def_property(srna, "distortion_amount", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "dist_amount");
	RNA_def_property_range(prop, 0, 10);
	RNA_def_property_ui_text(prop, "Distortion Amount", "");

	prop= RNA_def_property(srna, "noise_size", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "noisesize");
	RNA_def_property_range(prop, 0.0001, FLT_MAX);
	RNA_def_property_ui_range(prop, 0.0001, 2, 10, 2);
	RNA_def_property_ui_text(prop, "Noise Size", "Sets scaling for noise input");

	prop= RNA_def_property(srna, "noise_basis", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis");
	RNA_def_property_enum_items(prop, prop_noise_basis_items);
	RNA_def_property_ui_text(prop, "Noise Basis", "Sets the noise basis used for turbulence");

	prop= RNA_def_property(srna, "noise_distortion", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_sdna(prop, NULL, "noisebasis2");
	RNA_def_property_enum_items(prop, prop_noise_basis_items);
	RNA_def_property_ui_text(prop, "Noise Distortion", "Sets the noise basis for the distortion");

	prop= RNA_def_property(srna, "nabla", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.001, 0.1);
	RNA_def_property_ui_range(prop, 0.001, 0.1, 1, 2);
	RNA_def_property_ui_text(prop, "Nabla", "Size of derivative offset used for calculating normal");
}

static void rna_def_texture(BlenderRNA *brna)
{
	StructRNA *srna;
	PropertyRNA *prop;

	static EnumPropertyItem prop_type_items[] = {
		{0, "NONE", "None", ""},
		{TEX_CLOUDS, "CLOUDS", "Clouds", ""},
		{TEX_WOOD, "WOOD", "Wood", ""},
		{TEX_MARBLE, "MARBLE", "Marble", ""},
		{TEX_MAGIC, "MAGIC", "Magic", ""},
		{TEX_BLEND, "BLEND", "Blend", ""},
		{TEX_STUCCI, "STUCCI", "Stucci", ""},
		{TEX_NOISE, "NOISE", "Noise", ""},
		{TEX_IMAGE, "IMAGE", "Image", ""},
		{TEX_PLUGIN, "PLUGIN", "Plugin", ""},
		{TEX_ENVMAP, "ENVIRONMENT_MAP", "Environment Map", ""},
		{TEX_MUSGRAVE, "MUSGRAVE", "Musgrave", ""},
		{TEX_VORONOI, "VORONOI", "Voronoi", ""},
		{TEX_DISTNOISE, "DISTORTED_NOISE", "Distorted Noise", ""},
		{0, NULL, NULL, NULL}};

	srna= RNA_def_struct(brna, "Texture", "ID");
	RNA_def_struct_sdna(srna, "Tex");
	RNA_def_struct_ui_text(srna, "Texture", "Texture datablock used by materials, lamps, worlds and brushes.");
	RNA_def_struct_ui_icon(srna, ICON_TEXTURE_DATA);
	RNA_def_struct_refine_func(srna, "rna_Texture_refine");

	prop= RNA_def_property(srna, "type", PROP_ENUM, PROP_NONE);
	RNA_def_property_enum_items(prop, prop_type_items);
	RNA_def_property_ui_text(prop, "Type", "");

	prop= RNA_def_property(srna, "color_ramp", PROP_POINTER, PROP_NEVER_NULL);
	RNA_def_property_pointer_sdna(prop, NULL, "coba");
	RNA_def_property_struct_type(prop, "ColorRamp");
	RNA_def_property_ui_text(prop, "Color Ramp", "");

	prop= RNA_def_property(srna, "brightness", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "bright");
	RNA_def_property_range(prop, 0, 2);
	RNA_def_property_ui_text(prop, "Brightness", "");

	prop= RNA_def_property(srna, "contrast", PROP_FLOAT, PROP_NONE);
	RNA_def_property_range(prop, 0.01, 5);
	RNA_def_property_ui_text(prop, "Contrast", "");

	/* XXX: would be nicer to have this as a color selector?
	   but the values can go past [0,1]. */
	prop= RNA_def_property(srna, "rgb_factor", PROP_FLOAT, PROP_NONE);
	RNA_def_property_float_sdna(prop, NULL, "rfac");
	RNA_def_property_array(prop, 3);
	RNA_def_property_range(prop, 0, 2);
	RNA_def_property_ui_text(prop, "RGB Factor", "");

	rna_def_animdata_common(srna);

	/* specific types */
	rna_def_texture_clouds(brna);
	rna_def_texture_wood(brna);
	rna_def_texture_marble(brna);
	rna_def_texture_magic(brna);
	rna_def_texture_blend(brna);
	rna_def_texture_stucci(brna);
	rna_def_texture_noise(brna);
	rna_def_texture_image(brna);
	rna_def_texture_plugin(brna);
	rna_def_texture_environment_map(brna);
	rna_def_texture_musgrave(brna);
	rna_def_texture_voronoi(brna);
	rna_def_texture_distorted_noise(brna);
	/* XXX add more types here .. */
}

void RNA_def_texture(BlenderRNA *brna)
{
	rna_def_texture(brna);
	rna_def_mtex(brna);
	rna_def_environment_map(brna);
	rna_def_color_ramp(brna);
	rna_def_color_ramp_element(brna);
	rna_def_texmapping(brna);
}

#endif
