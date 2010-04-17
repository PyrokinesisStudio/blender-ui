#include "BPy_GetCompleteViewMapDensityF1D.h"

#include "../../../stroke/AdvancedFunctions1D.h"
#include "../../BPy_Convert.h"
#include "../../BPy_IntegrationType.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////

//------------------------INSTANCE METHODS ----------------------------------

static char GetCompleteViewMapDensityF1D___doc__[] =
".. method:: __init__(level, iType=IntegrationType.MEAN, sampling=2.0)\n"
"\n"
"   Builds a GetCompleteViewMapDensityF1D object.\n"
"\n"
"   :arg level: The level of the pyramid from which the pixel must be\n"
"      read.\n"
"   :type level: int\n"
"   :arg iType: The integration method used to compute a single value\n"
"      from a set of values.\n"
"   :type iType: :class:`IntegrationType`\n"
"   :arg sampling: The resolution used to sample the chain: the\n"
"      corresponding 0D function is evaluated at each sample point and\n"
"      the result is obtained by combining the resulting values into a\n"
"      single one, following the method specified by iType.\n"
"   :type sampling: float\n"
"\n"
".. method:: __call__(inter)\n"
"\n"
"   Returns the density evaluated for an Interface1D in the complete\n"
"   viewmap image.  The density is evaluated for a set of points along\n"
"   the Interface1D (using the :class:`ReadCompleteViewMapPixelF0D`\n"
"   functor) and then integrated into a single value using a\n"
"   user-defined integration method.\n"
"\n"
"   :arg inter: An Interface1D object.\n"
"   :type inter: :class:`Interface1D`\n"
"   :return: The density evaluated for the Interface1D in the complete\n"
"      viewmap image.\n"
"   :rtype: float\n";

static int GetCompleteViewMapDensityF1D___init__( BPy_GetCompleteViewMapDensityF1D* self, PyObject *args)
{
	PyObject *obj = 0;
	unsigned i;
	float f = 2.0;

	if( !PyArg_ParseTuple(args, "i|O!f", &i, &IntegrationType_Type, &obj, &f) )
		return -1;
	
	IntegrationType t = ( obj ) ? IntegrationType_from_BPy_IntegrationType(obj) : MEAN;
	self->py_uf1D_double.uf1D_double = new Functions1D::GetCompleteViewMapDensityF1D(i,t,f);
	return 0;

}

/*-----------------------BPy_GetCompleteViewMapDensityF1D type definition ------------------------------*/

PyTypeObject GetCompleteViewMapDensityF1D_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"GetCompleteViewMapDensityF1D", /* tp_name */
	sizeof(BPy_GetCompleteViewMapDensityF1D), /* tp_basicsize */
	0,                              /* tp_itemsize */
	0,                              /* tp_dealloc */
	0,                              /* tp_print */
	0,                              /* tp_getattr */
	0,                              /* tp_setattr */
	0,                              /* tp_reserved */
	0,                              /* tp_repr */
	0,                              /* tp_as_number */
	0,                              /* tp_as_sequence */
	0,                              /* tp_as_mapping */
	0,                              /* tp_hash  */
	0,                              /* tp_call */
	0,                              /* tp_str */
	0,                              /* tp_getattro */
	0,                              /* tp_setattro */
	0,                              /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	GetCompleteViewMapDensityF1D___doc__, /* tp_doc */
	0,                              /* tp_traverse */
	0,                              /* tp_clear */
	0,                              /* tp_richcompare */
	0,                              /* tp_weaklistoffset */
	0,                              /* tp_iter */
	0,                              /* tp_iternext */
	0,                              /* tp_methods */
	0,                              /* tp_members */
	0,                              /* tp_getset */
	&UnaryFunction1DDouble_Type,    /* tp_base */
	0,                              /* tp_dict */
	0,                              /* tp_descr_get */
	0,                              /* tp_descr_set */
	0,                              /* tp_dictoffset */
	(initproc)GetCompleteViewMapDensityF1D___init__, /* tp_init */
	0,                              /* tp_alloc */
	0,                              /* tp_new */
};

///////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
