// Copyright  (C)  2007  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Version: 1.0
// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// Maintainer: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// URL: http://www.orocos.org/kdl

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "joint.hpp"

namespace KDL {

    Joint::Joint(const JointType& _type, const double& _scale, const double& _offset,
                 const double& _inertia, const double& _damping, const double& _stiffness):
        type(_type),scale(_scale),offset(_offset),inertia(_inertia),damping(_damping),stiffness(_stiffness)
    {
		// this constructor should not be used for sphere joint, assume no offset in basis
    }

    Joint::Joint(const JointType& _type, const double& _scale, const Rotation& _basis,
                 const double& _inertia, const double& _damping, const double& _stiffness):
        type(_type),scale(_scale),offset(0.0),inertia(_inertia),damping(_damping),stiffness(_stiffness)
    {
		// this constructor should not be used for 1DOF joint
		if (_type == Sphere) {
			basis = _basis;
		}
    }

    Joint::Joint(const Joint& in):
	type(in.type),scale(in.scale),offset(in.offset),basis(in.basis),
        inertia(in.inertia),damping(in.damping),stiffness(in.stiffness)
    {
    }

    Joint& Joint::operator=(const Joint& in)
    {
        type=in.type;
        scale=in.scale;
        offset=in.offset;
		basis=in.basis;
        inertia=in.inertia;
        damping=in.damping;
        stiffness=in.stiffness;
		return *this;
    }


    Joint::~Joint()
    {
    }

    Frame Joint::pose(const double& q)const
    {

        switch(type){
        case RotX:
            return Frame(Rotation::RotX(scale*q+offset));
            break;
        case RotY:
            return  Frame(Rotation::RotY(scale*q+offset));
            break;
        case RotZ:
            return  Frame(Rotation::RotZ(scale*q+offset));
            break;
        case TransX:
            return  Frame(Vector(scale*q+offset,0.0,0.0));
            break;
        case TransY:
            return Frame(Vector(0.0,scale*q+offset,0.0));
            break;
        case TransZ:
            return Frame(Vector(0.0,0.0,scale*q+offset));
            break;
		case Sphere:
			// the joint angles represent a rotation vector expressed in the base frame of the joint
			// (= the frame you get when there is no offset nor rotation)
			return Frame(Rot(Vector((&q)[0], (&q)[1], (&q)[2]))*basis);
			break;
        default:
            return Frame::Identity();
            break;
        }
    }

    Twist Joint::twist(const double& qdot, int dof)const
    {
        switch(type){
        case RotX:
            return Twist(Vector(0.0,0.0,0.0),Vector(scale*qdot,0.0,0.0));
            break;
        case RotY:
            return Twist(Vector(0.0,0.0,0.0),Vector(0.0,scale*qdot,0.0));
            break;
        case RotZ:
            return Twist(Vector(0.0,0.0,0.0),Vector(0.0,0.0,scale*qdot));
            break;
        case TransX:
            return Twist(Vector(scale*qdot,0.0,0.0),Vector(0.0,0.0,0.0));
            break;
        case TransY:
            return Twist(Vector(0.0,scale*qdot,0.0),Vector(0.0,0.0,0.0));
            break;
        case TransZ:
            return Twist(Vector(0.0,0.0,scale*qdot),Vector(0.0,0.0,0.0));
            break;
		case Sphere:
			switch (dof) {
			case 0:
				return Twist(Vector(0.0,0.0,0.0),Vector(scale*qdot,0.0,0.0));
			case 1:
				return Twist(Vector(0.0,0.0,0.0),Vector(0.0,scale*qdot,0.0));
			case 2:
				return Twist(Vector(0.0,0.0,0.0),Vector(0.0,0.0,scale*qdot));
			}
			// walthrough
        default:
            return Twist::Zero();
            break;
        }
	}

	unsigned int Joint::getNDof() const
	{
		switch (type) {
		case Sphere:
			return 3;
		case None:
			return 0;
		default:
			return 1;
		}
	}

} // end of namespace KDL

