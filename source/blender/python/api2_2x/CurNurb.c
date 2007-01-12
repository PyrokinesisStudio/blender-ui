/*
 * $Id$
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
 * This is a new part of Blender.
 *
 * Contributor(s): Stephen Swaney
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#include "CurNurb.h" /*This must come first */

#include "BKE_curve.h"
#include "BDR_editcurve.h"	/* for convertspline */
#include "MEM_guardedalloc.h"
#include "gen_utils.h"
#include "BezTriple.h"


/*
 * forward declarations go here
 */


static PyObject *M_CurNurb_New( PyObject * self, PyObject * args );
static PyObject *CurNurb_setMatIndex( BPy_CurNurb * self, PyObject * args );
static PyObject *CurNurb_getMatIndex( BPy_CurNurb * self );
static PyObject *CurNurb_getFlagU( BPy_CurNurb * self );
static PyObject *CurNurb_setFlagU( BPy_CurNurb * self, PyObject * args );
static PyObject *CurNurb_getFlagV( BPy_CurNurb * self );
static PyObject *CurNurb_setFlagV( BPy_CurNurb * self, PyObject * args );
static PyObject *CurNurb_getType( BPy_CurNurb * self );
static PyObject *CurNurb_setType( BPy_CurNurb * self, PyObject * args );
/* static PyObject* CurNurb_setXXX( BPy_CurNurb* self, PyObject* args ); */
static int CurNurb_setPoint( BPy_CurNurb * self, int index, PyObject * ob );
static int CurNurb_length( PyInstanceObject * inst );
static PyObject *CurNurb_getIter( BPy_CurNurb * self );
static PyObject *CurNurb_iterNext( BPy_CurNurb * self );
PyObject *CurNurb_append( BPy_CurNurb * self, PyObject * args );

static PyObject *CurNurb_isNurb( BPy_CurNurb * self );
static PyObject *CurNurb_isCyclic( BPy_CurNurb * self );
static PyObject *CurNurb_dump( BPy_CurNurb * self );
static PyObject *CurNurb_switchDirection( BPy_CurNurb * self );
static PyObject *CurNurb_recalc( BPy_CurNurb * self );

char M_CurNurb_doc[] = "CurNurb";


/*	
  CurNurb_Type callback function prototypes:                          
*/

static void CurNurb_dealloc( BPy_CurNurb * self );
static int CurNurb_compare( BPy_CurNurb * a, BPy_CurNurb * b );
static PyObject *CurNurb_getAttr( BPy_CurNurb * self, char *name );
static int CurNurb_setAttr( BPy_CurNurb * self, char *name, PyObject * v );
static PyObject *CurNurb_repr( BPy_CurNurb * self );



/*
   table of module methods
   these are the equivalent of class or static methods.
   you do not need an object instance to call one.
  
*/

static PyMethodDef M_CurNurb_methods[] = {
/*   name, method, flags, doc_string                */
	{"New", ( PyCFunction ) M_CurNurb_New, METH_VARARGS | METH_KEYWORDS,
	 " () - doc string"},
/*  {"Get", (PyCFunction) M_CurNurb_method, METH_NOARGS, " () - doc string"}, */
/*   {"method", (PyCFunction) M_CurNurb_method, METH_NOARGS, " () - doc string"}, */

	{NULL, NULL, 0, NULL}
};



/*
 * method table
 * table of instance methods
 * these methods are invoked on an instance of the type.
*/

static PyMethodDef BPy_CurNurb_methods[] = {
/*   name,     method,                    flags,         doc               */
/*  {"method", (PyCFunction) CurNurb_method, METH_NOARGS, " () - doc string"} */
	{"setMatIndex", ( PyCFunction ) CurNurb_setMatIndex, METH_VARARGS,
	 "( index ) - set index into materials list"},
	{"getMatIndex", ( PyCFunction ) CurNurb_getMatIndex, METH_NOARGS,
	 "( ) - get current material index"},
	{"setFlagU", ( PyCFunction ) CurNurb_setFlagU, METH_VARARGS,
	 "( index ) - set flagU and recalculate the knots (0: uniform, 1: endpoints, 2: bezier)"},
	{"getFlagU", ( PyCFunction ) CurNurb_getFlagU, METH_NOARGS,
	 "( ) - get flagU of the knots"},
	{"setFlagV", ( PyCFunction ) CurNurb_setFlagV, METH_VARARGS,
	 "( index ) - set flagV and recalculate the knots (0: uniform, 1: endpoints, 2: bezier)"},
	{"getFlagV", ( PyCFunction ) CurNurb_getFlagV, METH_NOARGS,
	 "( ) - get flagV of the knots"},
	{"setType", ( PyCFunction ) CurNurb_setType, METH_VARARGS,
	 "( type ) - change the type of the curve (Poly: 0, Bezier: 1, NURBS: 4)"},
	{"getType", ( PyCFunction ) CurNurb_getType, METH_NOARGS,
	 "( ) - get the type of the curve (Poly: 0, Bezier: 1, NURBS: 4)"},
	{"append", ( PyCFunction ) CurNurb_append, METH_VARARGS,
	 "( point ) - add a new point.  arg is BezTriple or list of x,y,z,w floats"},
	{"isNurb", ( PyCFunction ) CurNurb_isNurb, METH_NOARGS,
	 "( ) - boolean function tests if this spline is type nurb or bezier"},
	{"isCyclic", ( PyCFunction ) CurNurb_isCyclic, METH_NOARGS,
	 "( ) - boolean function tests if this spline is cyclic (closed) or not (open)"},
	{"dump", ( PyCFunction ) CurNurb_dump, METH_NOARGS,
	 "( ) - dumps Nurb data)"},
	{"switchDirection", ( PyCFunction ) CurNurb_switchDirection, METH_NOARGS,
	 "( ) - swaps curve beginning and end)"},
	{"recalc", ( PyCFunction ) CurNurb_recalc, METH_NOARGS,
	 "( ) - recalc Nurb data)"},
	{NULL, NULL, 0, NULL}
};

/* 
 *   methods for CurNurb as sequece
 */

static PySequenceMethods CurNurb_as_sequence = {
	( inquiry ) CurNurb_length,	/* sq_length   */
	( binaryfunc ) 0,	/* sq_concat */
	( intargfunc ) 0,	/* sq_repeat */
	( intargfunc ) CurNurb_getPoint,	/* sq_item */
	( intintargfunc ) 0,	/* sq_slice */
	( intobjargproc ) CurNurb_setPoint,	/* sq_ass_item */
	0,			/* sq_ass_slice */
	( objobjproc ) 0,	/* sq_contains */
	0,
	0
};



/*
  Object Type definition
  full blown 2.3 struct
  if you are having trouble building with an earlier version of python,
   this is why.
*/

PyTypeObject CurNurb_Type = {
	PyObject_HEAD_INIT( NULL ) /* required py macro */
	0,	/* ob_size */
	/*  For printing, in format "<module>.<name>" */
	"CurNurb",		/* char *tp_name; */
	sizeof( CurNurb_Type ),	/* int tp_basicsize, */
	0,			/* tp_itemsize;  For allocation */

	/* Methods to implement standard operations */

	( destructor ) CurNurb_dealloc,	/*    destructor tp_dealloc; */
	0,			/*    printfunc tp_print; */
	( getattrfunc ) CurNurb_getAttr,	/*    getattrfunc tp_getattr; */
	( setattrfunc ) CurNurb_setAttr,	/*    setattrfunc tp_setattr; */
	( cmpfunc ) CurNurb_compare,	/*    cmpfunc tp_compare; */
	( reprfunc ) CurNurb_repr,	/*    reprfunc tp_repr; */

	/* Method suites for standard classes */

	0,			/*    PyNumberMethods *tp_as_number; */
	&CurNurb_as_sequence,	/*    PySequenceMethods *tp_as_sequence; */
	0,			/*    PyMappingMethods *tp_as_mapping; */

	/* More standard operations (here for binary compatibility) */

	0,			/*    hashfunc tp_hash; */
	0,			/*    ternaryfunc tp_call; */
	0,			/*    reprfunc tp_str; */
	0,			/*    getattrofunc tp_getattro; */
	0,			/*    setattrofunc tp_setattro; */

	/* Functions to access object as input/output buffer */
	0,			/*    PyBufferProcs *tp_as_buffer; */

  /*** Flags to define presence of optional/expanded features ***/
	Py_TPFLAGS_DEFAULT,	/*    long tp_flags; */

	0,			/*  char *tp_doc;  Documentation string */
  /*** Assigned meaning in release 2.0 ***/
	/* call function for all accessible objects */
	0,			/*    traverseproc tp_traverse; */

	/* delete references to contained objects */
	0,			/*    inquiry tp_clear; */

  /***  Assigned meaning in release 2.1 ***/
  /*** rich comparisons ***/
	0,			/*  richcmpfunc tp_richcompare; */

  /***  weak reference enabler ***/
	0,			/* long tp_weaklistoffset; */

  /*** Added in release 2.2 ***/
	/*   Iterators */
	( getiterfunc ) CurNurb_getIter,	/*    getiterfunc tp_iter; */
	( iternextfunc ) CurNurb_iterNext,	/*    iternextfunc tp_iternext; */

  /*** Attribute descriptor and subclassing stuff ***/
	BPy_CurNurb_methods,	/*    struct PyMethodDef *tp_methods; */
	0,			/*    struct PyMemberDef *tp_members; */
	0,			/*    struct PyGetSetDef *tp_getset; */
	0,			/*    struct _typeobject *tp_base; */
	0,			/*    PyObject *tp_dict; */
	0,			/*    descrgetfunc tp_descr_get; */
	0,			/*    descrsetfunc tp_descr_set; */
	0,			/*    long tp_dictoffset; */
	0,			/*    initproc tp_init; */
	0,			/*    allocfunc tp_alloc; */
	0,			/*    newfunc tp_new; */
	/*  Low-level free-memory routine */
	0,			/*    freefunc tp_free;  */
	/* For PyObject_IS_GC */
	0,			/*    inquiry tp_is_gc;  */
	0,			/*    PyObject *tp_bases; */
	/* method resolution order */
	0,			/*    PyObject *tp_mro;  */
	0,			/*    PyObject *tp_cache; */
	0,			/*    PyObject *tp_subclasses; */
	0,			/*    PyObject *tp_weaklist; */
	0
};




void CurNurb_dealloc( BPy_CurNurb * self )
{
	PyObject_DEL( self );
}



static PyObject *CurNurb_getAttr( BPy_CurNurb * self, char *name )
{
	PyObject *attr = Py_None;

	if( strcmp( name, "mat_index" ) == 0 )
		attr = PyInt_FromLong( self->nurb->mat_nr );

	else if( strcmp( name, "points" ) == 0 )
		attr = PyInt_FromLong( self->nurb->pntsu );

	else if( strcmp( name, "flagU" ) == 0 )
		attr = CurNurb_getFlagU( self );

	else if( strcmp( name, "flagV" ) == 0 )
		attr = CurNurb_getFlagV( self );

	else if( strcmp( name, "type" ) == 0 )
		attr = CurNurb_getType( self );

	else if( strcmp( name, "__members__" ) == 0 )
		attr = Py_BuildValue( "[s,s,s,s,s]", "mat_index", "points", "flagU", "flagV", "type" );

	if( !attr )
		return EXPP_ReturnPyObjError( PyExc_MemoryError,
					      "couldn't create PyObject" );

	/* member attribute found, return it */
	if( attr != Py_None )
		return attr;

	/* not an attribute, search the methods table */
	return Py_FindMethod( BPy_CurNurb_methods, ( PyObject * ) self, name );
}


/*
  setattr
*/

static int CurNurb_setAttr( BPy_CurNurb * self, char *name, PyObject * value )
{
	PyObject *valtuple;
	PyObject *error = NULL;

	/* make a tuple to pass to our type methods */
	valtuple = Py_BuildValue( "(O)", value );

	if( !valtuple )
		return EXPP_ReturnIntError( PyExc_MemoryError,
					    "CurNurb.setAttr: cannot create pytuple" );

	if( strcmp( name, "mat_index" ) == 0 )
		error = CurNurb_setMatIndex( self, valtuple );

	else if( strcmp( name, "flagU" ) == 0 )
		error = CurNurb_setFlagU( self, valtuple );

	else if( strcmp( name, "flagV" ) == 0 )
		error = CurNurb_setFlagV( self, valtuple );

	else if( strcmp( name, "type" ) == 0 )
		error = CurNurb_setType( self, valtuple );

	else {			/* error - no match for name */
		Py_DECREF( valtuple );

		if( ( strcmp( name, "ZZZZ" ) == 0 ) ||	/* user tried to change a */
		    ( strcmp( name, "ZZZZ" ) == 0 ) )	/* constant dict type ... */
			return EXPP_ReturnIntError( PyExc_AttributeError,
						    "constant dictionary -- cannot be changed" );
		else
			return EXPP_ReturnIntError( PyExc_KeyError,
						    "attribute not found" );
	}


	Py_DECREF( valtuple );	/* since it is not being returned */
	if( error != Py_None )
		return -1;

	Py_DECREF( Py_None );
	return 0;		/* normal exit */
}

/*
  compare
  in this case, we consider two CurNurbs equal, if they point to the same
  blender data.
*/

static int CurNurb_compare( BPy_CurNurb * a, BPy_CurNurb * b )
{
	Nurb *pa = a->nurb;
	Nurb *pb = b->nurb;

	return ( pa == pb ) ? 0 : -1;
}


/*
  factory method to create a BPy_CurNurb from a Blender Nurb
*/

PyObject *CurNurb_CreatePyObject( Nurb * blen_nurb )
{
	BPy_CurNurb *pyNurb;

	pyNurb = ( BPy_CurNurb * ) PyObject_NEW( BPy_CurNurb, &CurNurb_Type );

	if( !pyNurb )
		return EXPP_ReturnPyObjError( PyExc_MemoryError,
					      "could not create BPy_CurNurb PyObject" );

	pyNurb->nurb = blen_nurb;
	return ( PyObject * ) pyNurb;
}


/*
 *  CurNurb_repr
 */
static PyObject *CurNurb_repr( BPy_CurNurb * self )
{				/* used by 'repr' */

	return PyString_FromFormat( "[CurNurb \"%d\"]", self->nurb->type );
}

/* XXX Can't this be simply removed? */
static PyObject *M_CurNurb_New( PyObject * self, PyObject * args )
{
	return ( PyObject * ) 0;

}

/*
 *	Curve.getType
 */
static PyObject *CurNurb_getType( BPy_CurNurb * self )
{
	/* type is on 3 first bits only */
	return PyInt_FromLong( self->nurb->type & 7 );
}

/*
 *	Curve.setType
 *
 *	Convert the curve using Blender's convertspline fonction
 */
static PyObject *CurNurb_setType( BPy_CurNurb * self, PyObject * args )
{
	Nurb *nurb = self->nurb;
	short type;

	/* parameter type checking */
	if( !PyArg_ParseTuple( args, "h", &type ) )
		return EXPP_ReturnPyObjError
			( PyExc_TypeError, "expected integer argument" );

	/* parameter value checking */
	if (type != CU_POLY &&
		type != CU_BEZIER &&
		type != CU_NURBS)
		return EXPP_ReturnPyObjError
			 ( PyExc_ValueError, "expected integer argument" );

	/* convert and raise error if impossible */
	if (convertspline(type, nurb))
		return EXPP_ReturnPyObjError
			 ( PyExc_ValueError, "Conversion Impossible" );

	return EXPP_incr_ret( Py_None );
}



/*
 * CurNurb_append( point )
 * append a new point to a nurb curve.
 * arg is BezTriple or list of xyzw floats 
 */

PyObject *CurNurb_append( BPy_CurNurb * self, PyObject * args )
{
	Nurb *nurb = self->nurb;

	return CurNurb_appendPointToNurb( nurb, args );
}


/*
 * CurNurb_appendPointToNurb
 * this is a non-bpy utility func to add a point to a given nurb.
 * notice the first arg is Nurb*.
 */

PyObject *CurNurb_appendPointToNurb( Nurb * nurb, PyObject * args )
{

	int i;
	int size;
	PyObject *pyOb;
	int npoints = nurb->pntsu;

	/*
	   do we have a list of four floats or a BezTriple?
	*/
	if( !PyArg_ParseTuple( args, "O", &pyOb ))
		return EXPP_ReturnPyObjError
				( PyExc_RuntimeError,
				  "Internal error parsing arguments" );



	/* if curve is empty, adjust type depending on input type */
	if (nurb->bezt==NULL && nurb->bp==NULL) {
		if (BezTriple_CheckPyObject( pyOb ))
			nurb->type |= CU_BEZIER;
		else if (PySequence_Check( pyOb ))
			nurb->type |= CU_NURBS;
		else
			return( EXPP_ReturnPyObjError( PyExc_TypeError,
					  "Expected a BezTriple or a Sequence of 4 (or 5) floats" ) );
	}



	if ((nurb->type & 7)==CU_BEZIER) {
		BezTriple *tmp;

		if( !BezTriple_CheckPyObject( pyOb ) )
			return( EXPP_ReturnPyObjError( PyExc_TypeError,
					  "Expected a BezTriple\n" ) );

/*		printf("\ndbg: got a BezTriple\n"); */
		tmp = nurb->bezt;	/* save old points */
		nurb->bezt =
			( BezTriple * ) MEM_mallocN( sizeof( BezTriple ) *
						     ( npoints + 1 ),
						     "CurNurb_append2" );

		if( !nurb->bezt )
			return ( EXPP_ReturnPyObjError
				 ( PyExc_MemoryError, "allocation failed" ) );

		/* copy old points to new */
		if( tmp ) {
			memmove( nurb->bezt, tmp, sizeof( BezTriple ) * npoints );
			MEM_freeN( tmp );
		}

		nurb->pntsu++;
		/* add new point to end of list */
		memcpy( nurb->bezt + npoints,
			BezTriple_FromPyObject( pyOb ), sizeof( BezTriple ) );

	}
	else if( PySequence_Check( pyOb ) ) {
		size = PySequence_Size( pyOb );
/*		printf("\ndbg: got a sequence of size %d\n", size );  */
		if( size == 4 || size == 5 ) {
			BPoint *tmp;

			tmp = nurb->bp;	/* save old pts */

			nurb->bp =
				( BPoint * ) MEM_mallocN( sizeof( BPoint ) *
							  ( npoints + 1 ),
							  "CurNurb_append1" );
			if( !nurb->bp )
				return ( EXPP_ReturnPyObjError
					 ( PyExc_MemoryError,
					   "allocation failed" ) );

			memmove( nurb->bp, tmp, sizeof( BPoint ) * npoints );
			if( tmp )
				MEM_freeN( tmp );

			++nurb->pntsu;
			/* initialize new BPoint from old */
			memcpy( nurb->bp + npoints, nurb->bp,
				sizeof( BPoint ) );

			for( i = 0; i < 4; ++i ) {
				PyObject *item = PySequence_GetItem( pyOb, i );

				if (item == NULL)
					return NULL;


				nurb->bp[npoints].vec[i] = ( float ) PyFloat_AsDouble( item );
				Py_DECREF( item );
			}

			if (size == 5) {
				PyObject *item = PySequence_GetItem( pyOb, i );

				if (item == NULL)
					return NULL;

				nurb->bp[npoints].alfa = ( float ) PyFloat_AsDouble( item );
				Py_DECREF( item );
			}
			else {
				nurb->bp[npoints].alfa = 0.0f;
			}

			makeknots( nurb, 1, nurb->flagu >> 1 );

		} else {
			return EXPP_ReturnPyObjError( PyExc_TypeError,
					"expected a sequence of 4 or 5 floats" );
		}

	} else {
		/* bail with error */
		return EXPP_ReturnPyObjError( PyExc_TypeError,
					"expected a sequence of 4 or 5 floats" );

	}

	return ( EXPP_incr_ret( Py_None ) );
}


/*
 *  CurNurb_setMatIndex
 *
 *  set index into material list
 */

static PyObject *CurNurb_setMatIndex( BPy_CurNurb * self, PyObject * args )
{
	int index;

	if( !PyArg_ParseTuple( args, "i", &( index ) ) )
		return ( EXPP_ReturnPyObjError
			 ( PyExc_AttributeError,
			   "expected integer argument" ) );

	/* fixme:  some range checking would be nice! */
	self->nurb->mat_nr = (short)index;

	Py_INCREF( Py_None );
	return Py_None;
}

/*
 * CurNurb_getMatIndex
 *
 * returns index into material list
 */

static PyObject *CurNurb_getMatIndex( BPy_CurNurb * self )
{
	PyObject *index = PyInt_FromLong( ( long ) self->nurb->mat_nr );

	if( index )
		return index;

	return ( EXPP_ReturnPyObjError( PyExc_RuntimeError,
					"could not get material index" ) );
}

/*
 * CurNurb_getFlagU
 *
 * returns curve's flagu
 */

static PyObject *CurNurb_getFlagU( BPy_CurNurb * self )
{
	PyObject *flagu = PyInt_FromLong( ( long ) self->nurb->flagu );

	if( flagu )
		return flagu;

	return ( EXPP_ReturnPyObjError( PyExc_RuntimeError,
					"could not get CurNurb.flagu index" ) );
}

/*
 *  CurNurb_setFlagU
 *
 *  set curve's flagu and recalculate the knots
 *
 *  Possible values: 0 - uniform, 2 - endpoints, 4 - bezier
 *    bit 0 controls CU_CYCLIC
 */

static PyObject *CurNurb_setFlagU( BPy_CurNurb * self, PyObject * args )
{
	int flagu;

	if( !PyArg_ParseTuple( args, "i", &( flagu ) ) )
		return EXPP_ReturnPyObjError( PyExc_TypeError,
				"expected integer argument in range [0,5]" );

	if( flagu < 0 || flagu > 5 )
		return EXPP_ReturnPyObjError( PyExc_AttributeError,
				"expected integer argument in range [0,5]" );

	if( self->nurb->flagu != flagu ) {
		self->nurb->flagu = (short)flagu;
		makeknots( self->nurb, 1, self->nurb->flagu >> 1 );
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*
 * CurNurb_getFlagV
 *
 * returns curve's flagu
 */

static PyObject *CurNurb_getFlagV( BPy_CurNurb * self )
{
	PyObject *flagv = PyInt_FromLong( ( long ) self->nurb->flagv );

	if( flagv )
		return flagv;

	return ( EXPP_ReturnPyObjError( PyExc_RuntimeError,
					"could not get CurNurb.flagv" ) );
}

/*
 *  CurNurb_setFlagV
 *
 *  set curve's flagu and recalculate the knots
 *
 *  Possible values: 0 - uniform, 1 - endpoints, 2 - bezier
 */

static PyObject *CurNurb_setFlagV( BPy_CurNurb * self, PyObject * args )
{
	int flagv;

	if( !PyArg_ParseTuple( args, "i", &( flagv ) ) )
		return EXPP_ReturnPyObjError( PyExc_TypeError,
				"expected integer argument in range [0,5]" );

	if( flagv < 0 || flagv > 5 )
		return EXPP_ReturnPyObjError( PyExc_AttributeError,
				"expected integer argument in range [0,5]" );

	if( self->nurb->flagv != flagv ) {
		self->nurb->flagv = (short)flagv;
		makeknots( self->nurb, 2, self->nurb->flagv >> 1 );
	}

	Py_INCREF( Py_None );
	return Py_None;
}

/*
 * CurNurb_getIter
 *
 * create an iterator for our CurNurb.
 * this iterator returns the points for this CurNurb.
 */

static PyObject *CurNurb_getIter( BPy_CurNurb * self )
{
	self->bp = self->nurb->bp;
	self->bezt = self->nurb->bezt;
	self->atEnd = 0;
	self->nextPoint = 0;

	/* set exhausted flag if both bp and bezt are zero */
	if( ( !self->bp ) && ( !self->bezt ) )
		self->atEnd = 1;

	Py_INCREF( self );
	return ( PyObject * ) self;
}



static PyObject *CurNurb_iterNext( BPy_CurNurb * self )
{
	PyObject *po;		/* return value */
	Nurb *pnurb = self->nurb;
	int npoints = pnurb->pntsu;

	/* are we at end already? */
	if( self->atEnd )
		return ( EXPP_ReturnPyObjError( PyExc_StopIteration,
						"iterator at end" ) );

	if( self->nextPoint < npoints ) {

		po = CurNurb_pointAtIndex( self->nurb, self->nextPoint );
		self->nextPoint++;

		return po;

	} else {
		self->atEnd = 1;	/* set flag true */
	}

	return ( EXPP_ReturnPyObjError( PyExc_StopIteration,
					"iterator at end" ) );
}



/*
 * CurNurb_isNurb()
 * test whether spline nurb or bezier
 */

static PyObject *CurNurb_isNurb( BPy_CurNurb * self )
{
	/* NOTE: a Nurb has bp and bezt pointers
	 * depending on type.
	 * It is possible both are NULL if no points exist.
	 * in that case, we return False
	 */

	if( self->nurb->bp ) {
		return EXPP_incr_ret_True();
	} else {
		return EXPP_incr_ret_False();
	}
}

/*
 * CurNurb_isCyclic()
 * test whether spline cyclic (closed) or not (open)
 */

static PyObject *CurNurb_isCyclic( BPy_CurNurb * self )
{
        /* supposing that the flagu is always set */ 

	if( self->nurb->flagu & CU_CYCLIC ) {
		return EXPP_incr_ret_True();
	} else {
		return EXPP_incr_ret_False();
	}
}

/*
 * CurNurb_length
 * returns the number of points in a Nurb
 * this is a tp_as_sequence method, not a regular instance method.
 */

static int CurNurb_length( PyInstanceObject * inst )
{
	Nurb *nurb;
	int len;

	if( CurNurb_CheckPyObject( ( PyObject * ) inst ) ) {
		nurb = ( ( BPy_CurNurb * ) inst )->nurb;
		len = nurb->pntsu;
		return len;
	}

	return EXPP_ReturnIntError( PyExc_RuntimeError,
				    "arg is not a BPy_CurNurb" );
}


/*
 * CurNurb_getPoint
 * returns the Nth point in a Nurb
 * this is one of the tp_as_sequence methods, hence the int N argument.
 * it is called via the [] operator, not as a usual instance method.
 */

PyObject *CurNurb_getPoint( BPy_CurNurb * self, int index )
{
	Nurb *myNurb;

	int npoints;

	/* for convenince */
	myNurb = self->nurb;
	npoints = myNurb->pntsu;

	/* DELETED: bail if index < 0 */
	/* actually, this check is not needed since python treats */
	/* negative indices as starting from the right end of a sequence */
	/* 
	THAT IS WRONG, when passing a negative index, python adjusts it to be positive
	BUT it can still overflow in the negatives if the index is too small.
	For example, list[-6] when list contains 5 items means index = -1 in here.
	(theeth)
	*/

	/* bail if no Nurbs in Curve */
	if( npoints == 0 )
		return ( EXPP_ReturnPyObjError( PyExc_IndexError,
						"no points in this CurNurb" ) );

	/* check index limits */
	if( index >= npoints || index < 0 )
		return ( EXPP_ReturnPyObjError( PyExc_IndexError,
						"index out of range" ) );

	return CurNurb_pointAtIndex( myNurb, index );
}

/*
 * CurNurb_setPoint
 * modifies the Nth point in a Nurb
 * this is one of the tp_as_sequence methods, hence the int N argument.
 * it is called via the [] = operator, not as a usual instance method.
 */
static int CurNurb_setPoint( BPy_CurNurb * self, int index, PyObject * pyOb )
{
	Nurb *nurb = self->nurb;
	int size;

	/* check index limits */
	if( index < 0 || index >= nurb->pntsu )
		return EXPP_ReturnIntError( PyExc_IndexError,
					    "array assignment index out of range\n" );


	/* branch by curve type */
	if ((nurb->type & 7)==CU_BEZIER) {	/* BEZIER */
		/* check parameter type */
		if( !BezTriple_CheckPyObject( pyOb ) )
			return EXPP_ReturnIntError( PyExc_TypeError,
							"expected a BezTriple\n" );

		/* copy bezier in array */
		memcpy( nurb->bezt + index,
			BezTriple_FromPyObject( pyOb ), sizeof( BezTriple ) );

		return 0;	/* finished correctly */
	}
	else {	/* NURBS or POLY */
		int i;

		/* check parameter type */
		if (!PySequence_Check( pyOb ))
			return EXPP_ReturnIntError( PyExc_TypeError,
							"expected a list of 4 (or optionaly 5 if the curve is 3D) floats\n" );

		size = PySequence_Size( pyOb );

		/* check sequence size */
		if( size != 4 && size != 5 ) 
			return EXPP_ReturnIntError( PyExc_TypeError,
							"expected a list of 4 (or optionaly 5 if the curve is 3D) floats\n" );

		/* copy x, y, z, w */
		for( i = 0; i < 4; ++i ) {
			PyObject *item = PySequence_GetItem( pyOb, i );

			if (item == NULL)
				return -1;

			nurb->bp[index].vec[i] = ( float ) PyFloat_AsDouble( item );
			Py_DECREF( item );
		}

		if (size == 5) {	/* set tilt, if present */
			PyObject *item = PySequence_GetItem( pyOb, i );

			if (item == NULL)
				return -1;

			nurb->bp[index].alfa = ( float ) PyFloat_AsDouble( item );
			Py_DECREF( item );
		}
		else {				/* if not, set default	*/
			nurb->bp[index].alfa = 0.0f;
		}

		return 0;	/* finished correctly */
	}
}


/* 
 * this is an internal routine.  not callable directly from python
 */

PyObject *CurNurb_pointAtIndex( Nurb * nurb, int index )
{
	PyObject *pyo;

	if( nurb->bp ) {	/* we have a nurb curve */
		int i;

		/* add Tilt only if curve is 3D */
		if (nurb->flag & CU_3D)
			pyo = PyList_New( 5 );
		else
			pyo = PyList_New( 4 );

		for( i = 0; i < 4; i++ ) {
			PyList_SetItem( pyo, i,
					PyFloat_FromDouble( nurb->bp[index].
							    vec[i] ) );
		}

		/* add Tilt only if curve is 3D */
		if (nurb->flag & CU_3D)
			PyList_SetItem( pyo, 4,	PyFloat_FromDouble( nurb->bp[index].alfa ) );

	} else if( nurb->bezt ) {	/* we have a bezier */
		/* if an error occurs, we just pass it on */
		pyo = BezTriple_CreatePyObject( &( nurb->bezt[index] ) );

	} else			/* something is horribly wrong */
		/* neither bp or bezt is set && pntsu != 0 */
		return ( EXPP_ReturnPyObjError( PyExc_SystemError,
						"inconsistant structure found" ) );

	return ( pyo );
}


int CurNurb_CheckPyObject( PyObject * py_obj )
{
	return ( py_obj->ob_type == &CurNurb_Type );
}


PyObject *CurNurb_Init( void )
{
	PyObject *submodule;

	CurNurb_Type.ob_type = &PyType_Type;

	submodule =
		Py_InitModule3( "Blender.CurNurb", M_CurNurb_methods,
				M_CurNurb_doc );
	return ( submodule );
}

/*
  dump nurb
*/

PyObject *CurNurb_dump( BPy_CurNurb * self )
{
	BPoint *bp = NULL;
	BezTriple *bezt = NULL;
	Nurb *nurb = self->nurb;
	int npoints = 0;

	if( ! self->nurb ){  /* bail on error */
		printf("\n no Nurb in this CurNurb");
		Py_RETURN_NONE;
	}

	printf(" type: %d, mat_nr: %d hide: %d flag: %d",
		   nurb->type, nurb->mat_nr, nurb->hide, nurb->flag);
	printf("\n pntsu: %d, pntsv: %d, resolu: %d resolv: %d",
		   nurb->pntsu, nurb->pntsv, nurb->resolu, nurb->resolv );
	printf("\n orderu: %d  orderv: %d", nurb->orderu, nurb->orderv );
	printf("\n flagu: %d flagv: %d",
		   nurb->flagu, nurb->flagv );

	npoints = nurb->pntsu;

	if( nurb->bp ) { /* we have a BPoint  */
		int n;
		for( n = 0, bp = nurb->bp;
			 n < npoints;
			 n++, bp++ )
		{
			/* vec[4] */
			printf( "\ncoords[%d]: ", n);
			{
				int i;
				for( i = 0; i < 4; i++){
					printf("%10.3f ", bp->vec[i] );
				}
			}
		
			/* alfa, s[2] */
			printf("\n alpha: %5.2f", bp->alfa);
			/* f1, hide */
			printf(" f1 %d  hide %d", bp->f1, bp->hide );
			printf("\n");
		}
	}
	else { /* we have a BezTriple */
		int n;
		for( n = 0, bezt = nurb->bezt;
			 n < npoints;
			 n++, bezt++ )
		{
			int i, j;
			printf("\npoint %d: ", n);
			for( i = 0; i < 3; i++ ) {
				printf("\nvec[%i] ",i );
				for( j = 0; j < 3; j++ ) {
					printf(" %5.2f ", bezt->vec[i][j] );
				}
			}
				

		}
		printf("\n");
	}

	Py_RETURN_NONE;
}

/*
  recalc nurb
*/

static PyObject *CurNurb_recalc( BPy_CurNurb * self )
{
	calchandlesNurb ( self->nurb );
	Py_RETURN_NONE;
}

PyObject *CurNurb_switchDirection( BPy_CurNurb * self ) {
	Nurb *nurb = self->nurb;
	if( ! self->nurb ){  /* bail on error */
		printf("\n no Nurb in this CurNurb");
		Py_RETURN_NONE;
	}
	
	switchdirectionNurb( nurb );
	
	Py_RETURN_NONE;
}
