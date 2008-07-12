#ifndef FREESTYLE_PYTHON_ID_H
#define FREESTYLE_PYTHON_ID_H

#include <iostream>
using namespace std;

#include "../system/Id.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

#include <Python.h>

extern PyTypeObject Id_Type;

#define BPy_Id_Check(v) \
    ((v)->ob_type == &Id_Type)

/*---------------------------Python BPy_Id structure definition----------*/
typedef struct {
	PyObject_HEAD
	Id *id;
} BPy_Id;

/*---------------------------Python BPy_Id visible prototypes-----------*/

PyObject *Id_Init( void );


///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* FREESTYLE_PYTHON_ID_H */
