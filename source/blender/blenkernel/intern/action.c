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
 * Contributor(s): Full recode, Ton Roosendaal, Crete 2005
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <math.h>
#include <stdlib.h>	/* for NULL */

#include "MEM_guardedalloc.h"

#include "DNA_action_types.h"
#include "DNA_armature_types.h"
#include "DNA_constraint_types.h"
#include "DNA_curve_types.h"
#include "DNA_ipo_types.h"
#include "DNA_nla_types.h"
#include "DNA_object_types.h"
#include "DNA_scene_types.h"

#include "BKE_action.h"
#include "BKE_anim.h"
#include "BKE_armature.h"
#include "BKE_blender.h"
#include "BKE_constraint.h"
#include "BKE_global.h"
#include "BKE_ipo.h"
#include "BKE_library.h"
#include "BKE_main.h"
#include "BKE_object.h"
#include "BKE_utildefines.h"

#include "BLI_arithb.h"
#include "BLI_blenlib.h"

#include "nla.h"

/* *********************** NOTE ON POSE AND ACTION **********************

  - Pose is the local (object level) component of armature. The current
    object pose is saved in files, and (will be) is presorted for dependency
  - Actions have fewer (or other) channels, and write data to a Pose
  - Currently ob->pose data is controlled in where_is_pose only. The (recalc)
    event system takes care of calling that
  - The NLA system (here too) uses Poses as interpolation format for Actions
  - Therefore we assume poses to be static, and duplicates of poses have channels in
    same order, for quick interpolation reasons

  ****************************** (ton) ************************************ */

/* ***************** Library data level operations on action ************** */

void make_local_action(bAction *act)
{
	Object *ob;
	bAction *actn;
	int local=0, lib=0;
	
	if(act->id.lib==0) return;
	if(act->id.us==1) {
		act->id.lib= 0;
		act->id.flag= LIB_LOCAL;
		new_id(0, (ID *)act, 0);
		return;
	}
	
	ob= G.main->object.first;
	while(ob) {
		if(ob->action==act) {
			if(ob->id.lib) lib= 1;
			else local= 1;
		}
		ob= ob->id.next;
	}
	
	if(local && lib==0) {
		act->id.lib= 0;
		act->id.flag= LIB_LOCAL;
		new_id(0, (ID *)act, 0);
	}
	else if(local && lib) {
		actn= copy_action(act);
		actn->id.us= 0;
		
		ob= G.main->object.first;
		while(ob) {
			if(ob->action==act) {
				
				if(ob->id.lib==0) {
					ob->action = actn;
					ob->activecon = NULL;
					actn->id.us++;
					act->id.us--;
				}
			}
			ob= ob->id.next;
		}
	}
}


void free_action(bAction *act)
{
	bActionChannel *chan;
	
	/* Free channels */
	for (chan=act->chanbase.first; chan; chan=chan->next){
		if (chan->ipo)
			chan->ipo->id.us--;
		free_constraint_channels(&chan->constraintChannels);
	}
	
	if (act->chanbase.first)
		BLI_freelistN (&act->chanbase);
}

bAction* copy_action(bAction *src)
{
	bAction *dst = NULL;
	bActionChannel *dchan, *schan;
	
	if(!src) return NULL;
	
	dst= copy_libblock(src);
	duplicatelist(&(dst->chanbase), &(src->chanbase));
	
	for (dchan=dst->chanbase.first, schan=src->chanbase.first; dchan; dchan=dchan->next, schan=schan->next){
		dchan->ipo = copy_ipo(dchan->ipo);
		copy_constraint_channels(&dchan->constraintChannels, &schan->constraintChannels);
	}
	dst->id.flag |= LIB_FAKEUSER;
	dst->id.us++;
	return dst;
}



/* ************************ Pose channels *************** */

bPoseChannel *get_pose_channel(const bPose *pose, const char *name)
{
	bPoseChannel *chan;

	if(pose==NULL) return NULL;
	
	for (chan=pose->chanbase.first; chan; chan=chan->next){
		if (!strcmp (chan->name, name))
			return chan;
	}

	return NULL;
}

/* Use with care, not on Armature poses but for temporal ones */
/* (currently used for action constraints and in rebuild_pose) */
bPoseChannel *verify_pose_channel(bPose* pose, const char* name)
{
	bPoseChannel *chan;
	
	if (!pose){
		return NULL;
	}
	
	/*      See if this channel exists */
	for (chan=pose->chanbase.first; chan; chan=chan->next){
		if (!strcmp (name, chan->name))
			return chan;
	}
	
	/* If not, create it and add it */
	chan = MEM_callocN(sizeof(bPoseChannel), "verifyPoseChannel");
	
	strcpy (chan->name, name);
	/* init vars to prevent mat errors */
	chan->quat[0] = 1.0F;
	chan->size[0] = chan->size[1] = chan->size[2] = 1.0F;
	Mat3One(chan->ik_mat);
	
	BLI_addtail (&pose->chanbase, chan);
	
	return chan;
}


/* dst should be freed already, makes entire duplicate */
void copy_pose(bPose **dst, bPose *src, int copycon)
{
	bPose *outPose;
	bPoseChannel	*pchan;
	ListBase listb;
	
	if (!src){
		*dst=NULL;
		return;
	}
	
	outPose= MEM_callocN(sizeof(bPose), "pose");
	
	duplicatelist (&outPose->chanbase, &src->chanbase);
	
	if (copycon) {
		for (pchan=outPose->chanbase.first; pchan; pchan=pchan->next) {
			copy_constraints(&listb, &pchan->constraints);  // copy_constraints NULLs listb
			pchan->constraints= listb;
		}
	}
	
	*dst=outPose;
}

void free_pose_channels(bPose *pose) 
{
	bPoseChannel *chan;
	
	if (pose->chanbase.first){
		for (chan = pose->chanbase.first; chan; chan=chan->next){
			free_constraints(&chan->constraints);
		}
		BLI_freelistN (&pose->chanbase);
	}
}

static void copy_pose_channel_data(bPoseChannel *pchan, const bPoseChannel *chan)
{
	VECCOPY(pchan->loc, chan->loc);
	VECCOPY(pchan->size, chan->size);
	QUATCOPY(pchan->quat, chan->quat);
	pchan->flag= chan->flag;
}

/* checks for IK constraint, can do more constraints flags later */
/* pose should be entirely OK */
void update_pose_constraint_flags(bPose *pose)
{
	bPoseChannel *pchan;
	bConstraint *con;
	
	/* clear */
	for (pchan = pose->chanbase.first; pchan; pchan=pchan->next) {
		pchan->constflag= 0;
	}
	/* detect */
	for (pchan = pose->chanbase.first; pchan; pchan=pchan->next) {
		for(con= pchan->constraints.first; con; con= con->next) {
			if(con->type==CONSTRAINT_TYPE_KINEMATIC) {
				pchan->constflag |= PCHAN_HAS_IK;
			}
			else pchan->constflag |= PCHAN_HAS_CONST;
		}
	}
}


/* ************************ END Pose channels *************** */

bActionChannel *get_named_actionchannel(bAction *act, const char *name)
{
	bActionChannel *chan;
	
	if (!act)
		return NULL;
	
	for (chan = act->chanbase.first; chan; chan=chan->next){
		if (!strcmp (chan->name, name))
			return chan;
	}
	
	return NULL;
}

/* ************************ Blending with NLA *************** */


/* Only allowed for Poses with identical channels */
void blend_poses(bPose *dst, const bPose *src, float srcweight, short mode)
{
	bPoseChannel *dchan;
	const bPoseChannel *schan;
	float	dquat[4], squat[4];
	float dstweight;
	int i;
	
	switch (mode){
	case POSE_BLEND:
		dstweight = 1.0F - srcweight;
		break;
	case POSE_ADD:
		dstweight = 1.0F;
		break;
	default :
		dstweight = 1.0F;
	}
	
	schan= src->chanbase.first;
	for (dchan = dst->chanbase.first; dchan; dchan=dchan->next, schan= schan->next){
		if (schan->flag & (POSE_ROT|POSE_LOC|POSE_SIZE)) {
			
			/* replaced quat->matrix->quat conversion with decent quaternion interpol (ton) */
			
			/* Do the transformation blend */
			if (schan->flag & POSE_ROT) {
				QUATCOPY(dquat, dchan->quat);
				QUATCOPY(squat, schan->quat);
				if(mode==POSE_BLEND)
					QuatInterpol(dchan->quat, dquat, squat, srcweight);
				else
					QuatAdd(dchan->quat, dquat, squat, srcweight);
				
				NormalQuat (dchan->quat);
			}

			for (i=0; i<3; i++){
				if (schan->flag & POSE_LOC)
					dchan->loc[i] = (dchan->loc[i]*dstweight) + (schan->loc[i]*srcweight);
				if (schan->flag & POSE_SIZE)
					dchan->size[i] = 1.0f + ((dchan->size[i]-1.0f)*dstweight) + ((schan->size[i]-1.0f)*srcweight);
			}
			dchan->flag |= schan->flag;
		}
	}
}


float calc_action_start(const bAction *act)
{
	const bActionChannel *chan;
	const IpoCurve	*icu;
	float size=999999999.0f;
	int	i;
	int	foundvert=0;
	const bConstraintChannel *conchan;


	if (!act)
		return 0;

	for (chan=act->chanbase.first; chan; chan=chan->next){
		for (icu=chan->ipo->curve.first; icu; icu=icu->next)
			for (i=0; i<icu->totvert; i++){
				size = MIN2 (size, icu->bezt[i].vec[1][0]);
				foundvert=1;
				
			}
			for (conchan=chan->constraintChannels.first; conchan; conchan=conchan->next){
				for (icu=conchan->ipo->curve.first; icu; icu=icu->next)
					for (i=0; i<icu->totvert; i++){
						size = MIN2 (size, icu->bezt[i].vec[1][0]);
						foundvert=1;
					}
			}
	}
	
	if (!foundvert)
		return 0;
	else
		return size;
}

float calc_action_end(const bAction *act)
{
	const bActionChannel	*chan;
	const bConstraintChannel *conchan;
	const IpoCurve		*icu;
	float size=0;
	int	i;

	if (!act)
		return 0;

	for (chan=act->chanbase.first; chan; chan=chan->next){
		for (icu=chan->ipo->curve.first; icu; icu=icu->next)
			for (i=0; i<icu->totvert; i++)
				size = MAX2 (size, icu->bezt[i].vec[1][0]);
			
			for (conchan=chan->constraintChannels.first; conchan; conchan=conchan->next){
				for (icu=conchan->ipo->curve.first; icu; icu=icu->next)
					for (i=0; i<icu->totvert; i++)
						size = MAX2 (size, icu->bezt[i].vec[1][0]);
			}
	}
	return size;
}

/* Copy the data from the action-pose (src) into the pose */
/* both args are assumed to be valid */
/* exported to game engine */
void extract_pose_from_pose(bPose *pose, const bPose *src)
{
	const bPoseChannel *schan;
	bPoseChannel *pchan= pose->chanbase.first;

	for (schan=src->chanbase.first; schan; schan=schan->next, pchan= pchan->next) {
		copy_pose_channel_data(pchan, schan);
	}
}

/* seems to be disabled... (ton) */
#ifdef __NLA_BLENDCON
static void blend_constraints(ListBase *dst, const ListBase *src,
							  float srcweight, short mode)
{
	bConstraint *dcon;
	const bConstraint *scon;
	float dstweight = 0;
	
	switch (mode){
		case POSE_BLEND:
			dstweight = 1.0F - srcweight;
			break;
		case POSE_ADD:
			dstweight = 1.0F;
			break;
	}
	
	/* Blend constraints */
	for (dcon=dst->first; dcon; dcon=dcon->next){
		for (scon = src->first; scon; scon=scon->next){
			if (!strcmp(scon->name, dcon->name))
				break;
		}
		
		if (scon){
			dcon->enforce = (dcon->enforce*dstweight) + (scon->enforce*srcweight);
			if (mode == POSE_BLEND)
				dcon->enforce/=2.0;
			
			if (dcon->enforce>1.0)
				dcon->enforce=1.0;
			if (dcon->enforce<0.0)
				dcon->enforce=0.0;
			
		}
	}
}
#endif

/* seems to be disabled... (ton) */
#ifdef __NLA_BLENDCON
static void get_constraint_influence_from_pose(bPose *dst, bPose *src)
{
	bConstraint *dcon, *scon;

	if (!src || !dst)
		return;

	for (dcon = dst->chanbase.first; dcon; dcon=dcon->next){
		for (scon=src->chanbase.first; scon; scon=scon->next){
			if (!strcmp(scon->name, dcon->name))
				break;
		}
		if (scon){
			dcon->enforce = scon->enforce;
		}
	}
}
#endif

/* Pose should exist, can have any number of channels too (used for constraint) */
void extract_pose_from_action(bPose *pose, bAction *act, float ctime) 
{
	bActionChannel *achan;
	bPoseChannel	*pchan;
	Ipo				*ipo;

	if (!act)
		return;
	if (!pose)
		return;

	/* Copy the data from the action into the pose */
	for (achan=act->chanbase.first; achan; achan=achan->next){
		act->achan= achan;  // for ipos

		ipo = achan->ipo;
		if (ipo) {
			pchan= get_pose_channel(pose, achan->name);
			if(pchan) {
				act->pchan= pchan;  // for ipos
				
				/* Evaluates and sets the internal ipo value */
				calc_ipo(ipo, ctime);

				/* This call also sets the pchan flags */
				execute_ipo((ID*)act, achan->ipo);
			}
		}
	}
}

/* for do_all_actions, clears the pose */
static void rest_pose(bPose *pose, int clearflag)
{
	bPoseChannel *chan;
	int i;
	
	if (!pose)
		return;
	
	for (chan=pose->chanbase.first; chan; chan=chan->next){
		for (i=0; i<3; i++){
			chan->loc[i]=0.0;
			chan->quat[i+1]=0.0;
			chan->size[i]=1.0;
		}
		chan->quat[0]=1.0;
		if (clearflag)
			chan->flag =0;
	}
}

/* do constraint channels = execute the ipos */
static void do_pose_constraint_channels(bPose *pose, bAction *act, float ctime)
{
	bPoseChannel *pchan;
	bActionChannel *achan;
	
	if (!pose || !act)
		return;
	
	for (pchan=pose->chanbase.first; pchan; pchan=pchan->next){
		achan=get_named_actionchannel(act, pchan->name);
		if (achan)
			do_constraint_channels(&pchan->constraints, &achan->constraintChannels, ctime);
	}
}


void do_all_actions(Object *only_this)
{
	Base *base;
	bPose *apose=NULL;
	bPose *tpose=NULL;
	Object *ob;
	bActionStrip *strip;
	int	doit;
	float striptime, frametime, length, actlength;
	float blendfac, stripframe;
	int set;

	//printf("do all actions disabled\n");
	//return;
	
	/* NEW: current scene ob ipo's */
	base= G.scene->base.first;
	set= 0;

	while(base) {
		
		if(only_this) ob= only_this;
		else ob = base->object;

		/* Retrieve data from the NLA */
		if(ob->type==OB_ARMATURE && ob->pose) {
			bArmature *arm= ob->data;

			if(arm->flag & ARM_NO_ACTION) {  // no action set while transform
				;
			}
			else {
				doit=0;

				/* Clear pose */
				if (apose){
					free_pose_channels(apose);
					MEM_freeN(apose);
				}
				/* Clear pose */
				if (tpose){
					free_pose_channels(tpose);
					MEM_freeN(tpose);
				}

				copy_pose(&apose, ob->pose, 1);
				copy_pose(&tpose, ob->pose, 1);
				rest_pose(apose, 1);
	 
				if (ob->nlastrips.first){
					rest_pose(ob->pose, 0);
				}

				for (strip=ob->nlastrips.first; strip; strip=strip->next){
					doit = 0;
					if (strip->act){
				
						/* Determine if the current frame is within the strip's range */
						length = strip->end-strip->start;
						actlength = strip->actend-strip->actstart;
						striptime = (G.scene->r.cfra-(strip->start)) / length;
						stripframe = (G.scene->r.cfra-(strip->start)) ;


						if (striptime>=0.0){
							
							rest_pose(tpose, 1);

							/* Handle path */
							if (strip->flag & ACTSTRIP_USESTRIDE){
								if (ob->parent && ob->parent->type==OB_CURVE){
									Curve *cu = ob->parent->data;
									float ctime, pdist;

									if (cu->flag & CU_PATH){
										/* Ensure we have a valid path */
										if(cu->path==NULL || cu->path->data==NULL) printf("action path error\n");
										else {

											/* Find the position on the path */
											ctime= bsystem_time(ob, ob->parent, (float)G.scene->r.cfra, 0.0);
											
											if(calc_ipo_spec(cu->ipo, CU_SPEED, &ctime)==0) {
												ctime /= cu->pathlen;
												CLAMP(ctime, 0.0, 1.0);
											}
											pdist = ctime*cu->path->totdist;
											
											if (strip->stridelen)
												striptime = pdist / strip->stridelen;
											else
												striptime = 0;
											
											striptime = (float)fmod (striptime, 1.0);
											
											frametime = (striptime * actlength) + strip->actstart;
											extract_pose_from_action (tpose, strip->act, bsystem_time(ob, 0, frametime, 0.0));
		#ifdef __NLA_BLENDCON
											do_pose_constraint_channels(tpose, strip->act, bsystem_time(ob, 0, frametime, 0.0));
		#endif
											doit=1;
										}
									}
								}
							}

							/* Handle repeat */
			
							else if (striptime < 1.0){
								/* Mod to repeat */
								striptime*=strip->repeat;
								striptime = (float)fmod (striptime, 1.0);
								
								frametime = (striptime * actlength) + strip->actstart;
								extract_pose_from_action (tpose, strip->act, bsystem_time(ob, 0, frametime, 0.0));
	#ifdef __NLA_BLENDCON
								do_pose_constraint_channels(tpose, strip->act, bsystem_time(ob, 0, frametime, 0.0));
	#endif
								doit=1;
							}
							/* Handle extend */
							else{
								if (strip->flag & ACTSTRIP_HOLDLASTFRAME){
									striptime = 1.0;
									frametime = (striptime * actlength) + strip->actstart;
									extract_pose_from_action (tpose, strip->act, bsystem_time(ob, 0, frametime, 0.0));
	#ifdef __NLA_BLENDCON
									do_pose_constraint_channels(tpose, strip->act, bsystem_time(ob, 0, frametime, 0.0));
	#endif
									doit=1;
								}
							}

							/* Handle blendin & blendout */
							if (doit){
								/* Handle blendin */

								if (strip->blendin>0.0 && stripframe<=strip->blendin && G.scene->r.cfra>=strip->start){
									blendfac = stripframe/strip->blendin;
								}
								else if (strip->blendout>0.0 && stripframe>=(length-strip->blendout) && G.scene->r.cfra<=strip->end){
									blendfac = (length-stripframe)/(strip->blendout);
								}
								else
									blendfac = 1;

								/* Blend this pose with the accumulated pose */
								blend_poses (apose, tpose, blendfac, strip->mode);
	#ifdef __NLA_BLENDCON
								blend_constraints(&apose->chanbase, &tpose->chanbase, blendfac, strip->mode);
	#endif
							}
						}					
						if (apose){
							extract_pose_from_pose(ob->pose, apose);
	#ifdef __NLA_BLENDCON
							get_constraint_influence_from_pose(ob->pose, apose);
	#endif
						}
					}
					
				}

				/* Do local action (always overrides the nla actions) */
				/*	At the moment, only constraint ipos on the local action have any effect */
				if(ob->action) {
					extract_pose_from_action (ob->pose, ob->action, bsystem_time(ob, 0, (float) G.scene->r.cfra, 0.0));
					do_pose_constraint_channels(ob->pose, ob->action, bsystem_time(ob, 0, (float) G.scene->r.cfra, 0.0));
					doit = 1;
				} 
				
			}
		}		
		if(only_this) break;
		
		base= base->next;
		if(base==0 && set==0 && G.scene->set) {
			set= 1;
			base= G.scene->set->base.first; 
		}
		
	}

	if (apose){
		free_pose_channels(apose);
		MEM_freeN(apose);
		apose = NULL;
	}	
	if (tpose){
		free_pose_channels(tpose);
		MEM_freeN(tpose);
		apose = NULL;
	}

}

