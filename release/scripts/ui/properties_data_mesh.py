# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8 compliant>
import bpy
from rna_prop_ui import PropertyPanel

narrowui = 180


class MESH_MT_vertex_group_specials(bpy.types.Menu):
    bl_label = "Vertex Group Specials"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def draw(self, context):
        layout = self.layout

        layout.operator("object.vertex_group_sort", icon='SORTALPHA')
        layout.operator("object.vertex_group_copy", icon='COPY_ID')
        layout.operator("object.vertex_group_copy_to_linked", icon='LINK_AREA')
        layout.operator("object.vertex_group_copy_to_selected", icon='LINK_AREA')
        layout.operator("object.vertex_group_mirror", icon='ARROW_LEFTRIGHT')


class MESH_MT_shape_key_specials(bpy.types.Menu):
    bl_label = "Shape Key Specials"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def draw(self, context):
        layout = self.layout

        layout.operator("object.shape_key_transfer", icon='COPY_ID') # icon is not ideal
        layout.operator("object.join_shapes", icon='COPY_ID') # icon is not ideal
        layout.operator("object.shape_key_mirror", icon='ARROW_LEFTRIGHT')


class DataButtonsPanel(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "data"

    def poll(self, context):
        engine = context.scene.render.engine
        return context.mesh and (engine in self.COMPAT_ENGINES)


class DATA_PT_context_mesh(DataButtonsPanel):
    bl_label = ""
    bl_show_header = False
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def draw(self, context):
        layout = self.layout

        ob = context.object
        mesh = context.mesh
        space = context.space_data
        wide_ui = context.region.width > narrowui

        if wide_ui:
            split = layout.split(percentage=0.65)
            if ob:
                split.template_ID(ob, "data")
                split.separator()
            elif mesh:
                split.template_ID(space, "pin_id")
                split.separator()
        else:
            if ob:
                layout.template_ID(ob, "data")
            elif mesh:
                layout.template_ID(space, "pin_id")


class DATA_PT_custom_props_mesh(DataButtonsPanel, PropertyPanel):
    _context_path = "object.data"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}


class DATA_PT_normals(DataButtonsPanel):
    bl_label = "Normals"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def draw(self, context):
        layout = self.layout

        mesh = context.mesh
        wide_ui = context.region.width > narrowui

        split = layout.split()

        col = split.column()
        col.prop(mesh, "autosmooth")
        sub = col.column()
        sub.active = mesh.autosmooth
        sub.prop(mesh, "autosmooth_angle", text="Angle")

        if wide_ui:
            col = split.column()
        else:
            col.separator()
        col.prop(mesh, "vertex_normal_flip")
        col.prop(mesh, "double_sided")


class DATA_PT_settings(DataButtonsPanel):
    bl_label = "Settings"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def draw(self, context):
        layout = self.layout

        mesh = context.mesh

        layout.prop(mesh, "texture_mesh")


class DATA_PT_vertex_groups(DataButtonsPanel):
    bl_label = "Vertex Groups"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def poll(self, context):
        engine = context.scene.render.engine
        return (context.object and context.object.type in ('MESH', 'LATTICE') and (engine in self.COMPAT_ENGINES))

    def draw(self, context):
        layout = self.layout

        ob = context.object
        group = ob.active_vertex_group

        rows = 2
        if group:
            rows = 5

        row = layout.row()
        row.template_list(ob, "vertex_groups", ob, "active_vertex_group_index", rows=rows)

        col = row.column(align=True)
        col.operator("object.vertex_group_add", icon='ZOOMIN', text="")
        col.operator("object.vertex_group_remove", icon='ZOOMOUT', text="")
        col.menu("MESH_MT_vertex_group_specials", icon='DOWNARROW_HLT', text="")

        if group:
            row = layout.row()
            row.prop(group, "name")

        if ob.mode == 'EDIT' and len(ob.vertex_groups) > 0:
            row = layout.row()

            sub = row.row(align=True)
            sub.operator("object.vertex_group_assign", text="Assign")
            sub.operator("object.vertex_group_remove_from", text="Remove")

            sub = row.row(align=True)
            sub.operator("object.vertex_group_select", text="Select")
            sub.operator("object.vertex_group_deselect", text="Deselect")

            layout.prop(context.tool_settings, "vertex_group_weight", text="Weight")


class DATA_PT_shape_keys(DataButtonsPanel):
    bl_label = "Shape Keys"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def poll(self, context):
        engine = context.scene.render.engine
        return (context.object and context.object.type in ('MESH', 'LATTICE', 'CURVE', 'SURFACE') and (engine in self.COMPAT_ENGINES))

    def draw(self, context):
        layout = self.layout

        ob = context.object
        key = ob.data.shape_keys
        kb = ob.active_shape_key
        wide_ui = context.region.width > narrowui

        enable_edit = ob.mode != 'EDIT'
        enable_edit_value = False

        if ob.shape_key_lock is False:
            if enable_edit or (ob.type == 'MESH' and ob.shape_key_edit_mode):
                enable_edit_value = True

        row = layout.row()

        rows = 2
        if kb:
            rows = 5
        row.template_list(key, "keys", ob, "active_shape_key_index", rows=rows)

        col = row.column()

        sub = col.column(align=True)
        sub.operator("object.shape_key_add", icon='ZOOMIN', text="")
        sub.operator("object.shape_key_remove", icon='ZOOMOUT', text="")
        sub.menu("MESH_MT_shape_key_specials", icon='DOWNARROW_HLT', text="")

        if kb:
            col.separator()

            sub = col.column(align=True)
            sub.operator("object.shape_key_move", icon='TRIA_UP', text="").type = 'UP'
            sub.operator("object.shape_key_move", icon='TRIA_DOWN', text="").type = 'DOWN'

            split = layout.split(percentage=0.4)
            row = split.row()
            row.enabled = enable_edit
            if wide_ui:
                row.prop(key, "relative")

            row = split.row()
            row.alignment = 'RIGHT'

            if not wide_ui:
                layout.prop(key, "relative")
                row = layout.row()


            sub = row.row(align=True)
            subsub = sub.row(align=True)
            subsub.active = enable_edit_value
            subsub.prop(ob, "shape_key_lock", text="")
            subsub.prop(kb, "mute", text="")
            sub.prop(ob, "shape_key_edit_mode", text="")

            sub = row.row()
            sub.operator("object.shape_key_clear", icon='X', text="")

            row = layout.row()
            row.prop(kb, "name")

            if key.relative:
                if ob.active_shape_key_index != 0:
                    row = layout.row()
                    row.active = enable_edit_value
                    row.prop(kb, "value")

                    split = layout.split()

                    col = split.column(align=True)
                    col.active = enable_edit_value
                    col.label(text="Range:")
                    col.prop(kb, "slider_min", text="Min")
                    col.prop(kb, "slider_max", text="Max")

                    if wide_ui:
                        col = split.column(align=True)
                    col.active = enable_edit_value
                    col.label(text="Blend:")
                    col.prop_object(kb, "vertex_group", ob, "vertex_groups", text="")
                    col.prop_object(kb, "relative_key", key, "keys", text="")

            else:
                row = layout.row()
                row.active = enable_edit_value
                row.prop(key, "slurph")


class DATA_PT_uv_texture(DataButtonsPanel):
    bl_label = "UV Texture"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def draw(self, context):
        layout = self.layout

        me = context.mesh

        row = layout.row()
        col = row.column()

        col.template_list(me, "uv_textures", me, "active_uv_texture_index", rows=2)

        col = row.column(align=True)
        col.operator("mesh.uv_texture_add", icon='ZOOMIN', text="")
        col.operator("mesh.uv_texture_remove", icon='ZOOMOUT', text="")

        lay = me.active_uv_texture
        if lay:
            layout.prop(lay, "name")


class DATA_PT_texface(DataButtonsPanel):
    bl_label = "Texture Face"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def poll(self, context):
        ob = context.active_object
        rd = context.scene.render

        return (context.mode =='EDIT_MESH') and (rd.engine == 'BLENDER_GAME') and ob and ob.type == 'MESH'

    def draw(self, context):
        layout = self.layout
        col = layout.column()

        wide_ui = context.region.width > narrowui
        me = context.mesh

        tf = me.faces.active_tface

        if tf:
            split = layout.split()
            col = split.column()

            col.prop(tf, "tex")
            col.prop(tf, "light")
            col.prop(tf, "invisible")
            col.prop(tf, "collision")

            col.prop(tf, "shared")
            col.prop(tf, "twoside")
            col.prop(tf, "object_color")

            if wide_ui:
                col = split.column()

            col.prop(tf, "halo")
            col.prop(tf, "billboard")
            col.prop(tf, "shadow")
            col.prop(tf, "text")
            col.prop(tf, "alpha_sort")

            col = layout.column()
            col.prop(tf, "transp")
        else:
            col.label(text="No UV Texture")


class DATA_PT_vertex_colors(DataButtonsPanel):
    bl_label = "Vertex Colors"
    COMPAT_ENGINES = {'BLENDER_RENDER', 'BLENDER_GAME'}

    def draw(self, context):
        layout = self.layout

        me = context.mesh

        row = layout.row()
        col = row.column()

        col.template_list(me, "vertex_colors", me, "active_vertex_color_index", rows=2)

        col = row.column(align=True)
        col.operator("mesh.vertex_color_add", icon='ZOOMIN', text="")
        col.operator("mesh.vertex_color_remove", icon='ZOOMOUT', text="")

        lay = me.active_vertex_color
        if lay:
            layout.prop(lay, "name")


classes = [
    MESH_MT_vertex_group_specials,
    MESH_MT_shape_key_specials,

    DATA_PT_context_mesh,
    DATA_PT_normals,
    DATA_PT_settings,
    DATA_PT_vertex_groups,
    DATA_PT_shape_keys,
    DATA_PT_uv_texture,
    DATA_PT_texface,
    DATA_PT_vertex_colors,

    DATA_PT_custom_props_mesh]


def register():
    register = bpy.types.register
    for cls in classes:
        register(cls)


def unregister():
    unregister = bpy.types.unregister
    for cls in classes:
        unregister(cls)

if __name__ == "__main__":
    register()
