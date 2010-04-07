//
//  Filename         : Interface1D.h
//  Author(s)        : Emmanuel Turquin
//  Purpose          : Interface to 1D elts
//  Date of creation : 01/07/2003
//
///////////////////////////////////////////////////////////////////////////////


//
//  Copyright (C) : Please refer to the COPYRIGHT file distributed 
//   with this source distribution. 
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  INTERFACE1D_H
# define INTERFACE1D_H

# include <Python.h>
# include <string>
# include <iostream>
# include <float.h>
# include "../system/Id.h"
# include "../system/Precision.h"
# include "../winged_edge/Nature.h"
# include "Functions0D.h"

using namespace std;
/*! \file Interface1D.h
 *  Interface1D and related tools definitions
 */
// Integration method
/*! The different integration
 *  methods that can be invoked
 *  to integrate into a single value the set of values obtained
 *  from each 0D element of a 1D element.
 */
typedef enum {
  MEAN,/*!< The value computed for the 1D element is the mean of the values obtained for the 0D elements.*/
  MIN,/*!< The value computed for the 1D element is the minimum of the values obtained for the 0D elements.*/
  MAX,/*!< The value computed for the 1D element is the maximum of the values obtained for the 0D elements.*/
  FIRST,/*!< The value computed for the 1D element is the first of the values obtained for the 0D elements.*/
  LAST/*!< The value computed for the 1D element is the last of the values obtained for the 0D elements.*/
} IntegrationType;

/*! Returns a single
 *  value from a set of values evaluated at each 0D element
 *  of this 1D element.
 *  \param fun
 *    The UnaryFunction0D used to compute a value at each Interface0D.
 *  \param it
 *    The Interface0DIterator used to iterate over the 0D elements of
 *    this 1D element. The integration will occur over the 0D elements
 *    starting from the one pointed by it.
 *  \param it_end
 *    The Interface0DIterator pointing the end of the 0D elements of the
 *    1D element.
 *  \param integration_type
 *    The integration method used to compute a single value from
 *    a set of values.
 *  \return the single value obtained for the 1D element.
 */
template <class T>
T integrate(UnaryFunction0D<T>& fun,
	    Interface0DIterator it,
	    Interface0DIterator it_end,
	    IntegrationType integration_type = MEAN) {
  T res;
  unsigned size;
  switch (integration_type) {
  case MIN:
	fun(it);
	res = fun.result;++it;
    for (; !it.isEnd(); ++it) {
	  fun(it);
      if (fun.result < res)
		res = fun.result;
    }
    break;
  case MAX:
	fun(it);
	res = fun.result;++it;
    for (; !it.isEnd(); ++it) {
	  fun(it);
      if (fun.result > res)
	    res = fun.result;
    }
    break;
  case FIRST:
	fun(it);
	res = fun.result;
    break;
  case LAST:
	fun(--it_end);
	res = fun.result;
    break;
  case MEAN:
  default:
	fun(it);
	res = fun.result;++it;
	for (size = 1; !it.isEnd(); ++it, ++size) {
	  fun(it);
	  res += fun.result;
	}
    res /= (size ? size : 1);
    break;
  }
  return res;
}

//
// Interface1D
//
//////////////////////////////////////////////////

/*! Base class for any 1D element. */
class Interface1D
{
public:

  /*! Default constructor */
  Interface1D() {_timeStamp=0;}
  virtual ~Interface1D() {}; //soc

  /*! Returns the string "Interface1D" .*/
  virtual string getExactTypeName() const {
    return "Interface1D";
  }

  // Iterator access

  /*! Returns an iterator over the Interface1D vertices,
   *  pointing to the first vertex.
   */
  virtual Interface0DIterator verticesBegin() {
	  PyErr_SetString(PyExc_TypeError, "method verticesBegin() not properly overridden");
	  return Interface0DIterator();
  }
	
  /*! Returns an iterator over the Interface1D vertices,
   *  pointing after the last vertex.
   */
  virtual Interface0DIterator verticesEnd(){
	  PyErr_SetString(PyExc_TypeError, "method verticesEnd() not properly overridden");
	  return Interface0DIterator();
  }

  /*! Returns an iterator over the Interface1D points,
   *  pointing to the first point. The difference with
   *  verticesBegin() is that here we can iterate over
   *  points of the 1D element at a any given sampling.
   *  Indeed, for each iteration, a virtual point is created.
   *  \param t
   *    The sampling with which we want to iterate over points of
   *    this 1D element.
   */
  virtual Interface0DIterator pointsBegin(float t=0.f) {
	  PyErr_SetString(PyExc_TypeError, "method pointsBegin() not properly overridden");
	  return Interface0DIterator();
  }

  /*! Returns an iterator over the Interface1D points,
   *  pointing after the last point. The difference with
   *  verticesEnd() is that here we can iterate over
   *  points of the 1D element at a any given sampling.
   *  Indeed, for each iteration, a virtual point is created.
   *  \param t
   *    The sampling with which we want to iterate over points of
   *    this 1D element.
   */
  virtual Interface0DIterator pointsEnd(float t=0.f) {
	  PyErr_SetString(PyExc_TypeError, "method pointsEnd() not properly overridden");
	  return Interface0DIterator();
  }

  // Data access methods

  /*! Returns the 2D length of the 1D element. */
  virtual real getLength2D() const {
	  PyErr_SetString(PyExc_TypeError, "method getLength2D() not properly overridden");
	  return 0;
  }

  /*! Returns the Id of the 1D element .*/
  virtual Id getId() const {
	  PyErr_SetString(PyExc_TypeError, "method getId() not properly overridden");
	  return Id(0, 0);
  }	


  // FIXME: ce truc n'a rien a faire la...(c une requete complexe qui doit etre ds les Function1D)
  /*! Returns the nature of the 1D element. */
  virtual Nature::EdgeNature getNature() const {
	  PyErr_SetString(PyExc_TypeError, "method getNature() not properly overridden");
	  return Nature::NO_FEATURE;
  }	
  
  /*! Returns the time stamp of the 1D element. Mainly used for selection. */
  virtual unsigned getTimeStamp() const {
    return _timeStamp;
  }
  
  /*! Sets the time stamp for the 1D element. */
  inline void setTimeStamp(unsigned iTimeStamp){
    _timeStamp = iTimeStamp;
  }

protected:
  unsigned _timeStamp;
};

#endif // INTERFACE1D_H
