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
#  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8 compliant>
import bpy

narrowui = 180

class ConstraintButtonsPanel(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "constraint"

    def draw_constraint(self, context, con):
        layout = self.layout

        box = layout.template_constraint(con)
        wide_ui = context.region.width > narrowui

        if box:
            # match enum type to our functions, avoids a lookup table.
            getattr(self, con.type)(context, box, con, wide_ui)

            # show/key buttons here are most likely obsolete now, with
            # keyframing functionality being part of every button
            if con.type not in ('RIGID_BODY_JOINT', 'SPLINE_IK', 'NULL'):
                box.prop(con, "influence")

    def space_template(self, layout, con, wide_ui, target=True, owner=True):
        if target or owner:

            split = layout.split(percentage=0.2)

            if wide_ui:
                split.label(text="Space:")
                row = split.row()
            else:
                row = layout.row()


            if target:
                row.prop(con, "target_space", text="")

            if wide_ui:
                if target and owner:
                    row.label(icon='ICON_ARROW_LEFTRIGHT')
            else:
                row = layout.row()
            if owner:
                row.prop(con, "owner_space", text="")

    def target_template(self, layout, con, wide_ui, subtargets=True):
        if wide_ui:
            layout.prop(con, "target") # XXX limiting settings for only 'curves' or some type of object
        else:
            layout.prop(con, "target", text="")

        if con.target and subtargets:
            if con.target.type == 'ARMATURE':
                if wide_ui:
                    layout.prop_object(con, "subtarget", con.target.data, "bones", text="Bone")
                else:
                    layout.prop_object(con, "subtarget", con.target.data, "bones", text="")

                if con.type == 'COPY_LOCATION':
                    row = layout.row()
                    row.label(text="Head/Tail:")
                    row.prop(con, "head_tail", text="")
            elif con.target.type in ('MESH', 'LATTICE'):
                layout.prop_object(con, "subtarget", con.target, "vertex_groups", text="Vertex Group")

    def ik_template(self, layout, con, wide_ui):
        # only used for iTaSC
        layout.prop(con, "pole_target")

        if con.pole_target and con.pole_target.type == 'ARMATURE':
            layout.prop_object(con, "pole_subtarget", con.pole_target.data, "bones", text="Bone")

        if con.pole_target:
            row = layout.row()
            row.label()
            row.prop(con, "pole_angle")

        split = layout.split(percentage=0.33)
        col = split.column()
        col.prop(con, "tail")
        col.prop(con, "stretch")

        col = split.column()
        col.prop(con, "chain_length")
        col.prop(con, "targetless")

    def CHILD_OF(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        split = layout.split()

        col = split.column()
        col.label(text="Location:")
        col.prop(con, "locationx", text="X")
        col.prop(con, "locationy", text="Y")
        col.prop(con, "locationz", text="Z")

        col = split.column()
        col.label(text="Rotation:")
        col.prop(con, "rotationx", text="X")
        col.prop(con, "rotationy", text="Y")
        col.prop(con, "rotationz", text="Z")

        col = split.column()
        col.label(text="Scale:")
        col.prop(con, "sizex", text="X")
        col.prop(con, "sizey", text="Y")
        col.prop(con, "sizez", text="Z")

        split = layout.split()

        col = split.column()
        col.operator("constraint.childof_set_inverse")

        if wide_ui:
            col = split.column()
        col.operator("constraint.childof_clear_inverse")

    def TRACK_TO(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        row = layout.row()
        if wide_ui:
            row.label(text="To:")
        row.prop(con, "track", expand=True)

        split = layout.split()

        col = split.column()
        col.prop(con, "up", text="Up")

        if wide_ui:
            col = split.column()
        col.prop(con, "target_z")

        self.space_template(layout, con, wide_ui)

    def IK(self, context, layout, con, wide_ui):
        if context.object.pose.ik_solver == "ITASC":
            layout.prop(con, "ik_type")
            getattr(self, 'IK_' + con.ik_type)(context, layout, con, wide_ui)
        else:
            # Legacy IK constraint
            self.target_template(layout, con, wide_ui)
            if wide_ui:
                layout.prop(con, "pole_target")
            else:
                layout.prop(con, "pole_target", text="")
            if con.pole_target and con.pole_target.type == 'ARMATURE':
                if wide_ui:
                    layout.prop_object(con, "pole_subtarget", con.pole_target.data, "bones", text="Bone")
                else:
                    layout.prop_object(con, "pole_subtarget", con.pole_target.data, "bones", text="")

            if con.pole_target:
                row = layout.row()
                row.prop(con, "pole_angle")
                if wide_ui:
                    row.label()

            split = layout.split()
            col = split.column()
            col.prop(con, "iterations")
            col.prop(con, "chain_length")

            col.label(text="Weight:")
            col.prop(con, "weight", text="Position", slider=True)
            sub = col.column()
            sub.active = con.use_rotation
            sub.prop(con, "orient_weight", text="Rotation", slider=True)

            if wide_ui:
                col = split.column()
            col.prop(con, "use_tail")
            col.prop(con, "use_stretch")
            col.separator()
            col.prop(con, "use_target")
            col.prop(con, "use_rotation")

    def IK_COPY_POSE(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)
        self.ik_template(layout, con, wide_ui)

        row = layout.row()
        row.label(text="Axis Ref:")
        row.prop(con, "axis_reference", expand=True)
        split = layout.split(percentage=0.33)
        split.row().prop(con, "position")
        row = split.row()
        row.prop(con, "weight", text="Weight", slider=True)
        row.active = con.position
        split = layout.split(percentage=0.33)
        row = split.row()
        row.label(text="Lock:")
        row = split.row()
        row.prop(con, "pos_lock_x", text="X")
        row.prop(con, "pos_lock_y", text="Y")
        row.prop(con, "pos_lock_z", text="Z")
        split.active = con.use_position

        split = layout.split(percentage=0.33)
        split.row().prop(con, "rotation")
        row = split.row()
        row.prop(con, "orient_weight", text="Weight", slider=True)
        row.active = con.use_rotation
        split = layout.split(percentage=0.33)
        row = split.row()
        row.label(text="Lock:")
        row = split.row()
        row.prop(con, "rot_lock_x", text="X")
        row.prop(con, "rot_lock_y", text="Y")
        row.prop(con, "rot_lock_z", text="Z")
        split.active = con.use_rotation

    def IK_DISTANCE(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)
        self.ik_template(layout, con, wide_ui)

        layout.prop(con, "limit_mode")
        row = layout.row()
        row.prop(con, "weight", text="Weight", slider=True)
        row.prop(con, "distance", text="Distance", slider=True)

    def FOLLOW_PATH(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        split = layout.split()

        col = split.column()
        col.prop(con, "use_curve_follow")
        col.prop(con, "use_curve_radius")

        if wide_ui:
            col = split.column()
        col.prop(con, "use_fixed_position")
        if con.use_fixed_position:
            col.prop(con, "offset_factor", text="Offset")
        else:
            col.prop(con, "use_offset")

        row = layout.row()
        if wide_ui:
            row.label(text="Forward:")
        row.prop(con, "forward", expand=True)

        row = layout.row()
        row.prop(con, "up", text="Up")
        if wide_ui:
            row.label()

    def LIMIT_ROTATION(self, context, layout, con, wide_ui):

        split = layout.split()

        col = split.column(align=True)
        col.prop(con, "use_limit_x")
        sub = col.column()
        sub.active = con.use_limit_x
        sub.prop(con, "minimum_x", text="Min")
        sub.prop(con, "maximum_x", text="Max")

        if wide_ui:
            col = split.column(align=True)
        col.prop(con, "use_limit_y")
        sub = col.column()
        sub.active = con.use_limit_y
        sub.prop(con, "minimum_y", text="Min")
        sub.prop(con, "maximum_y", text="Max")

        if wide_ui:
            col = split.column(align=True)
        col.prop(con, "use_limit_z")
        sub = col.column()
        sub.active = con.use_limit_z
        sub.prop(con, "minimum_z", text="Min")
        sub.prop(con, "maximum_z", text="Max")

        row = layout.row()
        row.prop(con, "limit_transform")
        if wide_ui:
            row.label()

        row = layout.row()
        if wide_ui:
            row.label(text="Convert:")
        row.prop(con, "owner_space", text="")

    def LIMIT_LOCATION(self, context, layout, con, wide_ui):
        split = layout.split()

        col = split.column()
        col.prop(con, "use_minimum_x")
        sub = col.column()
        sub.active = con.use_minimum_x
        sub.prop(con, "minimum_x", text="")
        col.prop(con, "use_maximum_x")
        sub = col.column()
        sub.active = con.use_maximum_x
        sub.prop(con, "maximum_x", text="")

        if wide_ui:
            col = split.column()
        col.prop(con, "use_minimum_y")
        sub = col.column()
        sub.active = con.use_minimum_y
        sub.prop(con, "minimum_y", text="")
        col.prop(con, "use_maximum_y")
        sub = col.column()
        sub.active = con.use_maximum_y
        sub.prop(con, "maximum_y", text="")

        if wide_ui:
            col = split.column()
        col.prop(con, "use_minimum_z")
        sub = col.column()
        sub.active = con.use_minimum_z
        sub.prop(con, "minimum_z", text="")
        col.prop(con, "use_maximum_z")
        sub = col.column()
        sub.active = con.use_maximum_z
        sub.prop(con, "maximum_z", text="")

        row = layout.row()
        row.prop(con, "limit_transform")
        if wide_ui:
            row.label()

        row = layout.row()
        if wide_ui:
            row.label(text="Convert:")
        row.prop(con, "owner_space", text="")

    def LIMIT_SCALE(self, context, layout, con, wide_ui):
        split = layout.split()

        col = split.column()
        col.prop(con, "use_minimum_x")
        sub = col.column()
        sub.active = con.use_minimum_x
        sub.prop(con, "minimum_x", text="")
        col.prop(con, "use_maximum_x")
        sub = col.column()
        sub.active = con.use_maximum_x
        sub.prop(con, "maximum_x", text="")

        if wide_ui:
            col = split.column()
        col.prop(con, "use_minimum_y")
        sub = col.column()
        sub.active = con.use_minimum_y
        sub.prop(con, "minimum_y", text="")
        col.prop(con, "use_maximum_y")
        sub = col.column()
        sub.active = con.use_maximum_y
        sub.prop(con, "maximum_y", text="")

        if wide_ui:
            col = split.column()
        col.prop(con, "use_minimum_z")
        sub = col.column()
        sub.active = con.use_minimum_z
        sub.prop(con, "minimum_z", text="")
        col.prop(con, "use_maximum_z")
        sub = col.column()
        sub.active = con.use_maximum_z
        sub.prop(con, "maximum_z", text="")

        row = layout.row()
        row.prop(con, "limit_transform")
        if wide_ui:
            row.label()

        row = layout.row()
        if wide_ui:
            row.label(text="Convert:")
        row.prop(con, "owner_space", text="")

    def COPY_ROTATION(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        split = layout.split()

        col = split.column()
        col.prop(con, "rotate_like_x", text="X")
        sub = col.column()
        sub.active = con.rotate_like_x
        sub.prop(con, "invert_x", text="Invert")

        col = split.column()
        col.prop(con, "rotate_like_y", text="Y")
        sub = col.column()
        sub.active = con.rotate_like_y
        sub.prop(con, "invert_y", text="Invert")

        col = split.column()
        col.prop(con, "rotate_like_z", text="Z")
        sub = col.column()
        sub.active = con.rotate_like_z
        sub.prop(con, "invert_z", text="Invert")

        layout.prop(con, "use_offset")

        self.space_template(layout, con, wide_ui)

    def COPY_LOCATION(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        split = layout.split()

        col = split.column()
        col.prop(con, "locate_like_x", text="X")
        sub = col.column()
        sub.active = con.locate_like_x
        sub.prop(con, "invert_x", text="Invert")

        col = split.column()
        col.prop(con, "locate_like_y", text="Y")
        sub = col.column()
        sub.active = con.locate_like_y
        sub.prop(con, "invert_y", text="Invert")

        col = split.column()
        col.prop(con, "locate_like_z", text="Z")
        sub = col.column()
        sub.active = con.locate_like_z
        sub.prop(con, "invert_z", text="Invert")

        layout.prop(con, "use_offset")

        self.space_template(layout, con, wide_ui)

    def COPY_SCALE(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        row = layout.row(align=True)
        row.prop(con, "size_like_x", text="X")
        row.prop(con, "size_like_y", text="Y")
        row.prop(con, "size_like_z", text="Z")

        layout.prop(con, "use_offset")

        self.space_template(layout, con, wide_ui)

    #def SCRIPT(self, context, layout, con):

    def ACTION(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        if wide_ui:
            layout.prop(con, "action")
        else:
            layout.prop(con, "action", text="")

        if wide_ui:
            layout.prop(con, "transform_channel")
        else:
            layout.prop(con, "transform_channel", text="")

        split = layout.split()

        col = split.column(align=True)
        col.label(text="Action Length:")
        col.prop(con, "start_frame", text="Start")
        col.prop(con, "end_frame", text="End")

        if wide_ui:
            col = split.column(align=True)
        col.label(text="Target Range:")
        col.prop(con, "minimum", text="Min")
        col.prop(con, "maximum", text="Max")

        row = layout.row()
        if wide_ui:
            row.label(text="Convert:")
        row.prop(con, "target_space", text="")

    def LOCKED_TRACK(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        row = layout.row()
        if wide_ui:
            row.label(text="To:")
        row.prop(con, "track", expand=True)

        row = layout.row()
        if wide_ui:
            row.label(text="Lock:")
        row.prop(con, "locked", expand=True)

    def LIMIT_DISTANCE(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        col = layout.column(align=True)
        col.prop(con, "distance")
        col.operator("constraint.limitdistance_reset")

        row = layout.row()
        row.label(text="Clamp Region:")
        row.prop(con, "limit_mode", text="")

    def STRETCH_TO(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        split = layout.split()

        col = split.column()
        col.prop(con, "original_length", text="Rest Length")

        if wide_ui:
            col = split.column()
        col.operator("constraint.stretchto_reset", text="Reset")

        col = layout.column()
        col.prop(con, "bulge", text="Volume Variation")

        row = layout.row()
        if wide_ui:
            row.label(text="Volume:")
        row.prop(con, "volume", expand=True)
        if not wide_ui:
            row = layout.row()
        row.label(text="Plane:")
        row.prop(con, "keep_axis", expand=True)

    def FLOOR(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        split = layout.split()

        col = split.column()
        col.prop(con, "sticky")

        if wide_ui:
            col = split.column()
        col.prop(con, "use_rotation")

        layout.prop(con, "offset")

        row = layout.row()
        if wide_ui:
            row.label(text="Min/Max:")
        row.prop(con, "floor_location", expand=True)

    def RIGID_BODY_JOINT(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        if wide_ui:
            layout.prop(con, "pivot_type")
        else:
            layout.prop(con, "pivot_type", text="")
        if wide_ui:
            layout.prop(con, "child")
        else:
            layout.prop(con, "child", text="")

        split = layout.split()

        col = split.column()
        col.prop(con, "disable_linked_collision", text="No Collision")

        if wide_ui:
            col = split.column()
        col.prop(con, "draw_pivot", text="Display Pivot")

        split = layout.split()

        col = split.column(align=True)
        col.label(text="Pivot:")
        col.prop(con, "pivot_x", text="X")
        col.prop(con, "pivot_y", text="Y")
        col.prop(con, "pivot_z", text="Z")

        if wide_ui:
            col = split.column(align=True)
        col.label(text="Axis:")
        col.prop(con, "axis_x", text="X")
        col.prop(con, "axis_y", text="Y")
        col.prop(con, "axis_z", text="Z")

        #Missing: Limit arrays (not wrapped in RNA yet)

    def CLAMP_TO(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        row = layout.row()
        if wide_ui:
            row.label(text="Main Axis:")
        row.prop(con, "main_axis", expand=True)

        row = layout.row()
        row.prop(con, "cyclic")

    def TRANSFORM(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        layout.prop(con, "extrapolate_motion", text="Extrapolate")

        col = layout.column()
        col.row().label(text="Source:")
        col.row().prop(con, "map_from", expand=True)

        split = layout.split()

        sub = split.column(align=True)
        sub.label(text="X:")
        sub.prop(con, "from_min_x", text="Min")
        sub.prop(con, "from_max_x", text="Max")

        if wide_ui:
            sub = split.column(align=True)
        sub.label(text="Y:")
        sub.prop(con, "from_min_y", text="Min")
        sub.prop(con, "from_max_y", text="Max")

        if wide_ui:
            sub = split.column(align=True)
        sub.label(text="Z:")
        sub.prop(con, "from_min_z", text="Min")
        sub.prop(con, "from_max_z", text="Max")

        split = layout.split()

        col = split.column()
        col.label(text="Destination:")
        col.row().prop(con, "map_to", expand=True)

        split = layout.split()

        col = split.column()
        col.label(text="X:")
        col.row().prop(con, "map_to_x_from", expand=True)

        sub = col.column(align=True)
        sub.prop(con, "to_min_x", text="Min")
        sub.prop(con, "to_max_x", text="Max")

        if wide_ui:
            col = split.column()
        col.label(text="Y:")
        col.row().prop(con, "map_to_y_from", expand=True)

        sub = col.column(align=True)
        sub.prop(con, "to_min_y", text="Min")
        sub.prop(con, "to_max_y", text="Max")

        if wide_ui:
            col = split.column()
        col.label(text="Z:")
        col.row().prop(con, "map_to_z_from", expand=True)

        sub = col.column(align=True)
        sub.prop(con, "to_min_z", text="Min")
        sub.prop(con, "to_max_z", text="Max")

        self.space_template(layout, con, wide_ui)

    def SHRINKWRAP(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        layout.prop(con, "distance")
        layout.prop(con, "shrinkwrap_type")

        if con.shrinkwrap_type == 'PROJECT':
            row = layout.row(align=True)
            row.prop(con, "axis_x")
            row.prop(con, "axis_y")
            row.prop(con, "axis_z")

    def DAMPED_TRACK(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        row = layout.row()
        if wide_ui:
            row.label(text="To:")
        row.prop(con, "track", expand=True)

    def SPLINE_IK(self, context, layout, con, wide_ui):
        self.target_template(layout, con, wide_ui)

        col = layout.column()
        col.label(text="Spline Fitting:")
        col.prop(con, "chain_length")
        col.prop(con, "even_divisions")
        col.prop(con, "chain_offset")

        col = layout.column()
        col.label(text="Chain Scaling:")
        col.prop(con, "y_stretch")
        if wide_ui:
            col.prop(con, "xz_scaling_mode")
        else:
            col.prop(con, "xz_scaling_mode", text="")
        col.prop(con, "use_curve_radius")


class OBJECT_PT_constraints(ConstraintButtonsPanel):
    bl_label = "Object Constraints"
    bl_context = "constraint"

    def poll(self, context):
        return (context.object)

    def draw(self, context):
        layout = self.layout
        ob = context.object
        wide_ui = context.region.width > narrowui

        row = layout.row()
        row.operator_menu_enum("object.constraint_add", "type")
        if wide_ui:
            row.label()

        for con in ob.constraints:
            self.draw_constraint(context, con)


class BONE_PT_inverse_kinematics(ConstraintButtonsPanel):
    bl_label = "Inverse Kinematics"
    bl_default_closed = True
    bl_context = "bone_constraint"

    def poll(self, context):
        ob = context.object
        bone = context.bone

        if ob and bone:
            pchan = ob.pose.bones[bone.name]
            return pchan.has_ik

        return False

    def draw(self, context):
        layout = self.layout

        ob = context.object
        bone = context.bone
        pchan = ob.pose.bones[bone.name]
        wide_ui = context.region.width > narrowui

        row = layout.row()
        row.prop(ob.pose, "ik_solver")

        split = layout.split(percentage=0.25)
        split.prop(pchan, "ik_dof_x", text="X")
        row = split.row()
        row.prop(pchan, "ik_stiffness_x", text="Stiffness", slider=True)
        row.active = pchan.ik_dof_x

        if wide_ui:
            split = layout.split(percentage=0.25)
            sub = split.row()
        else:
            sub = layout.column(align=True)
        sub.prop(pchan, "ik_limit_x", text="Limit")
        sub.active = pchan.ik_dof_x
        if wide_ui:
            sub = split.row(align=True)
        sub.prop(pchan, "ik_min_x", text="")
        sub.prop(pchan, "ik_max_x", text="")
        sub.active = pchan.ik_dof_x and pchan.ik_limit_x

        split = layout.split(percentage=0.25)
        split.prop(pchan, "ik_dof_y", text="Y")
        row = split.row()
        row.prop(pchan, "ik_stiffness_y", text="Stiffness", slider=True)
        row.active = pchan.ik_dof_y

        if wide_ui:
            split = layout.split(percentage=0.25)
            sub = split.row()
        else:
            sub = layout.column(align=True)
        sub.prop(pchan, "ik_limit_y", text="Limit")
        sub.active = pchan.ik_dof_y
        if wide_ui:
            sub = split.row(align=True)
        sub.prop(pchan, "ik_min_y", text="")
        sub.prop(pchan, "ik_max_y", text="")
        sub.active = pchan.ik_dof_y and pchan.ik_limit_y

        split = layout.split(percentage=0.25)
        split.prop(pchan, "ik_dof_z", text="Z")
        sub = split.row()
        sub.prop(pchan, "ik_stiffness_z", text="Stiffness", slider=True)
        sub.active = pchan.ik_dof_z

        if wide_ui:
            split = layout.split(percentage=0.25)
            sub = split.row()
        else:
            sub = layout.column(align=True)
        sub.prop(pchan, "ik_limit_z", text="Limit")
        sub.active = pchan.ik_dof_z
        if wide_ui:
            sub = split.row(align=True)
        sub.prop(pchan, "ik_min_z", text="")
        sub.prop(pchan, "ik_max_z", text="")
        sub.active = pchan.ik_dof_z and pchan.ik_limit_z
        split = layout.split()
        split.prop(pchan, "ik_stretch", text="Stretch", slider=True)
        if wide_ui:
            split.label()

        if ob.pose.ik_solver == 'ITASC':
            split = layout.split()
            col = split.column()
            col.prop(pchan, "ik_rot_control", text="Control Rotation")
            if wide_ui:
                col = split.column()
            col.prop(pchan, "ik_rot_weight", text="Weight", slider=True)
            # not supported yet
            #row = layout.row()
            #row.prop(pchan, "ik_lin_control", text="Joint Size")
            #row.prop(pchan, "ik_lin_weight", text="Weight", slider=True)


class BONE_PT_iksolver_itasc(ConstraintButtonsPanel):
    bl_label = "iTaSC parameters"
    bl_default_closed = True
    bl_context = "bone_constraint"

    def poll(self, context):
        ob = context.object
        bone = context.bone

        if ob and bone:
            pchan = ob.pose.bones[bone.name]
            return pchan.has_ik and ob.pose.ik_solver == 'ITASC' and ob.pose.ik_param

        return False

    def draw(self, context):
        layout = self.layout

        ob = context.object
        itasc = ob.pose.ik_param
        wide_ui = context.region.width > narrowui

        layout.prop(itasc, "mode", expand=True)
        simulation = itasc.mode == 'SIMULATION'
        if simulation:
            layout.label(text="Reiteration:")
            layout.prop(itasc, "reiteration", expand=True)

        split = layout.split()
        split.active = not simulation or itasc.reiteration != 'NEVER'
        col = split.column()
        col.prop(itasc, "precision")

        if wide_ui:
            col = split.column()
        col.prop(itasc, "num_iter")


        if simulation:
            layout.prop(itasc, "auto_step")
            row = layout.row()
            if itasc.auto_step:
                row.prop(itasc, "min_step", text="Min")
                row.prop(itasc, "max_step", text="Max")
            else:
                row.prop(itasc, "num_step")

        layout.prop(itasc, "solver")
        if simulation:
            layout.prop(itasc, "feedback")
            layout.prop(itasc, "max_velocity")
        if itasc.solver == 'DLS':
            row = layout.row()
            row.prop(itasc, "dampmax", text="Damp", slider=True)
            row.prop(itasc, "dampeps", text="Eps", slider=True)


class BONE_PT_constraints(ConstraintButtonsPanel):
    bl_label = "Bone Constraints"
    bl_context = "bone_constraint"

    def poll(self, context):
        ob = context.object
        return (ob and ob.type == 'ARMATURE' and context.bone)

    def draw(self, context):
        layout = self.layout

        ob = context.object
        pchan = ob.pose.bones[context.bone.name]
        wide_ui = context.region.width > narrowui

        row = layout.row()
        row.operator_menu_enum("pose.constraint_add", "type")
        if wide_ui:
            row.label()

        for con in pchan.constraints:
            self.draw_constraint(context, con)

bpy.types.register(OBJECT_PT_constraints)
bpy.types.register(BONE_PT_iksolver_itasc)
bpy.types.register(BONE_PT_inverse_kinematics)
bpy.types.register(BONE_PT_constraints)
