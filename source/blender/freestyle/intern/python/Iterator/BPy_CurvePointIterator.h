#ifndef FREESTYLE_PYTHON_CURVEPOINTITERATOR_H
#define FREESTYLE_PYTHON_CURVEPOINTITERATOR_H

#include "../../stroke/CurveIterators.h"

#include "../BPy_Iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

#include <Python.h>

extern PyTypeObject CurvePointIterator_Type;

#define BPy_CurvePointIterator_Check(v)	(  PyObject_IsInstance( (PyObject *) v, (PyObject *) &CurvePointIterator_Type)  )

/*---------------------------Python BPy_CurvePointIterator structure definition----------*/
typedef struct {
	BPy_Iterator py_it;
	CurveInternal::CurvePointIterator *cp_it;
} BPy_CurvePointIterator;

///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* FREESTYLE_PYTHON_CURVEPOINTITERATOR_H */
