
/********************************************************************************/
/*    Copyright 2012 Mehmet Ali ANIL                                            */
/*                                                                              */
/*    Licensed under the Apache License, Version 2.0 (the "License");           */
/*    you may not use this file except in compliance with the License.          */
/*    You may obtain a copy of the License at                                   */
/*                                                                              */
/*    http://www.apache.org/licenses/LICENSE-2.0                                */
/*                                                                              */
/*    Unless required by applicable law or agreed to in writing, software       */
/*    distributed under the License is distributed on an "AS IS" BASIS,         */
/*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/*    See the License for the specific language governing permissions and       */
/*    limitations under the License.                                            */
/********************************************************************************/


#include <omp.h>
#include <Python.h>
#include "numpy/arrayobject.h"
#include "numpy/ndarraytypes.h"
#include <stdio.h>
#include <stdbool.h>

static PyObject* xor_masking_c(PyObject *self, PyObject *args);
static PyObject* or_masking_c(PyObject *self, PyObject *args);
static PyObject* and_masking_c(PyObject *self, PyObject *args);
static PyObject* advance_c(PyObject *self, PyObject *args);

static char doc_xor[] =
"This is the C extension for xor_masking routine. It interfaces with Python via C-Api, and calculates the"
"next state with C pointer arithmetic";
static char doc_or[] =
"This is the C extension for or_masking routine. It interfaces with Python via C-Api, and calculates the"
"next state with C pointer arithmetic";
static char doc_and[] =
"This is the C extension for and_masking routine. It interfaces with Python via C-Api, and calculates the"
"next state with C pointer arithmetic";
static char doc_advance_c[] =
		"";

static PyMethodDef boolfuncs_c[] = {
        {"xor_masking_c", xor_masking_c, METH_VARARGS, doc_xor},
        {"or_masking_c", or_masking_c, METH_VARARGS, doc_or},
        {"and_masking_c", and_masking_c, METH_VARARGS, doc_and},
        {"advance_c", advance_c, METH_VARARGS, doc_advance_c},
        {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initboolfuncs_c(void)
{
    (void) Py_InitModule("boolfuncs_c", boolfuncs_c);
    import_array();
}

static PyObject* advance_c(PyObject *self, PyObject *args){
    PyObject *adjacency ,*mask, *state;
    PyArrayObject *adjacency_arr, *mask_arr, *state_arr, *state_out;
    int stepcount;

    if (!PyArg_ParseTuple(args,"OOOi:advance_c", &adjacency, &mask, &state, &stepcount)) return NULL;

    adjacency_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(adjacency, NPY_BOOL,2,2);

    if (adjacency_arr == NULL) return NULL;
    mask_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(mask, NPY_BOOL,2,2);

    if (mask_arr == NULL) return NULL;
    state_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(state, NPY_BOOL,1,1);

    if (state_arr == NULL) return NULL;

    int dims[2], dims_new[2];

    dims[0] = adjacency_arr -> dimensions[0];
    dims[1] = adjacency_arr -> dimensions[1];
    dims_new[1] =  adjacency_arr -> dimensions[0];
    dims_new[0] =  stepcount;

    if (!(dims[0]==dims[1] && mask_arr -> dimensions[0] == dims[0]
                         && mask_arr -> dimensions[1] == dims[0]
                         && state_arr -> dimensions[0] == dims[0]))
                         return NULL;

    state_out = (PyArrayObject *) PyArray_FromDims(2,dims_new, NPY_BOOL);


    npy_bool *adj_value_ptr, *mask_value_ptr, *state_value_ptr, *state_out_ptr;
    npy_intp i,j;
    npy_intp step;


    for(step=0; step<stepcount; step++){
    		for(i=0;i<dims[0];i++){
				npy_int sum = 0;
				npy_int conn_ctr = 0;

					for(j=0;j<dims[1];j++){

						adj_value_ptr = (adjacency_arr->data + i*adjacency_arr->strides[0]
								 +j*adjacency_arr->strides[1]);

						if (*adj_value_ptr == true){

							mask_value_ptr = (mask_arr->data + i*mask_arr->strides[0]
							+j*mask_arr->strides[1]);

							if(step==0){
								state_value_ptr = (state_arr->data + j*state_arr->strides[0]);
							} else {
								state_value_ptr = (state_out->data + (step-1)*state_out->strides[0] + j*state_out->strides[1]);
							}

							if ( (*(bool *) mask_value_ptr ^ *(bool *)state_value_ptr) ==  true){
								sum++;
							}
							conn_ctr++;
						}
					}

					state_out_ptr = (state_out->data + step*state_out->strides[0] + i*state_out->strides[1]);

					if (conn_ctr < sum*2){
						*state_out_ptr =  true;
					} else {
						*state_out_ptr =  false;

					}
    		}
    }
    Py_DECREF(adjacency_arr);
    Py_DECREF(mask_arr);
    Py_DECREF(state_arr);
    return PyArray_Return(state_out);
}

static PyObject* xor_masking_c(PyObject *self, PyObject *args){
    PyObject *adjacency ,*mask, *state;
    PyArrayObject *adjacency_arr, *mask_arr, *state_arr, *state_out;

    if (!PyArg_ParseTuple(args,"OOO:trace", &adjacency, &mask, &state)) return NULL;

    adjacency_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(adjacency, NPY_BOOL,2,2);

    if (adjacency_arr == NULL) return NULL;
    mask_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(mask, NPY_BOOL,2,2);

    if (mask_arr == NULL) return NULL;
    state_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(state, NPY_BOOL,1,1);

    if (state_arr == NULL) return NULL;

    int dims[2], dims_new[1];
    dims[0] = adjacency_arr -> dimensions[0];
    dims[1] = adjacency_arr -> dimensions[1];
    dims_new[0] =  adjacency_arr -> dimensions[0];
    if (!(dims[0]==dims[1] && mask_arr -> dimensions[0] == dims[0]
                         && mask_arr -> dimensions[1] == dims[0]
                         && state_arr -> dimensions[0] == dims[0]))
                         return NULL;


    state_out = (PyArrayObject *) PyArray_FromDims(1,dims_new,NPY_BOOL);

    npy_bool *adj_value_ptr, *mask_value_ptr, *state_value_ptr, *state_out_ptr;
    npy_intp i,j;

    for(i=0;i<dims[0];i++){
        npy_int sum = 0;
        npy_int conn_ctr = 0;

            for(j=0;j<dims[1];j++){

                adj_value_ptr = (adjacency_arr->data + i*adjacency_arr->strides[0]
                         +j*adjacency_arr->strides[1]);

                if (*adj_value_ptr == true){

                    mask_value_ptr = (mask_arr->data + i*mask_arr->strides[0]
                    +j*mask_arr->strides[1]);

                    state_value_ptr = (state_arr->data + j*state_arr->strides[0]);

                    if ( (*(bool *) mask_value_ptr ^ *(bool *)state_value_ptr) ==  true){
                        sum++;
                    }
                    conn_ctr++;
                }
            }
            state_out_ptr = (state_out->data + i*state_out->strides[0]);
            if (conn_ctr < sum*2){
                *state_out_ptr =  true;
            }
            else {
                *state_out_ptr =  false;
            }
    }

    Py_DECREF(adjacency_arr);
    Py_DECREF(mask_arr);
    Py_DECREF(state_arr);
    return PyArray_Return(state_out);
}


static PyObject* and_masking_c(PyObject *self, PyObject *args){
    PyObject *adjacency ,*mask, *state;
    PyArrayObject *adjacency_arr, *mask_arr, *state_arr, *state_out;

    if (!PyArg_ParseTuple(args,"OOO:trace", &adjacency, &mask, &state)) return NULL;

    adjacency_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(adjacency, NPY_BOOL,2,2);

    if (adjacency_arr == NULL) return NULL;
    mask_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(mask, NPY_BOOL,2,2);

    if (mask_arr == NULL) return NULL;
    state_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(state, NPY_BOOL,1,1);

    if (state_arr == NULL) return NULL;

    int dims[2], dims_new[1];
    dims[0] = adjacency_arr -> dimensions[0];
    dims[1] = adjacency_arr -> dimensions[1];
    dims_new[0] =  adjacency_arr -> dimensions[0];
    if (!(dims[0]==dims[1] && mask_arr -> dimensions[0] == dims[0]
                         && mask_arr -> dimensions[1] == dims[0]
                         && state_arr -> dimensions[0] == dims[0]))
                         return NULL;


    state_out = (PyArrayObject *) PyArray_FromDims(1,dims_new,NPY_BOOL);

    npy_bool *adj_value_ptr, *mask_value_ptr, *state_value_ptr, *state_out_ptr;
    npy_intp i,j;

    for(i=0;i<dims[0];i++){
        npy_int sum = 0;
        npy_int conn_ctr = 0;

            for(j=0;j<dims[1];j++){

                adj_value_ptr = (adjacency_arr->data + i*adjacency_arr->strides[0]
                         +j*adjacency_arr->strides[1]);

                if (*adj_value_ptr == true){

                    mask_value_ptr = (mask_arr->data + i*mask_arr->strides[0]
                    +j*mask_arr->strides[1]);

                    state_value_ptr = (state_arr->data + j*state_arr->strides[0]);

                    if ( (*(bool *) mask_value_ptr && *(bool *)state_value_ptr) ==  true){
                        sum++;
                    }
                    conn_ctr++;
                }
            }
            state_out_ptr = (state_out->data + i*state_out->strides[0]);
            if (conn_ctr < sum*2){
                *state_out_ptr =  true;
            }
            else {
                *state_out_ptr =  false;
            }
    }

    Py_DECREF(adjacency_arr);
    Py_DECREF(mask_arr);
    Py_DECREF(state_arr);
    return PyArray_Return(state_out);
}

static PyObject* or_masking_c(PyObject *self, PyObject *args){
    PyObject *adjacency ,*mask, *state;
    PyArrayObject *adjacency_arr, *mask_arr, *state_arr, *state_out;

    if (!PyArg_ParseTuple(args,"OOO:trace", &adjacency, &mask, &state)) return NULL;

    adjacency_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(adjacency, NPY_BOOL,2,2);

    if (adjacency_arr == NULL) return NULL;
    mask_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(mask, NPY_BOOL,2,2);

    if (mask_arr == NULL) return NULL;
    state_arr = (PyArrayObject *)
        PyArray_ContiguousFromObject(state, NPY_BOOL,1,1);

    if (state_arr == NULL) return NULL;

    int dims[2], dims_new[1];
    dims[0] = adjacency_arr -> dimensions[0];
    dims[1] = adjacency_arr -> dimensions[1];
    dims_new[0] =  adjacency_arr -> dimensions[0];
    if (!(dims[0]==dims[1] && mask_arr -> dimensions[0] == dims[0]
                         && mask_arr -> dimensions[1] == dims[0]
                         && state_arr -> dimensions[0] == dims[0]))
                         return NULL;


    state_out = (PyArrayObject *) PyArray_FromDims(1,dims_new,NPY_BOOL);

    npy_bool *adj_value_ptr, *mask_value_ptr, *state_value_ptr, *state_out_ptr;
    npy_intp i,j;

    for(i=0;i<dims[0];i++){
        npy_int sum = 0;
        npy_int conn_ctr = 0;

            for(j=0;j<dims[1];j++){

                adj_value_ptr = (adjacency_arr->data + i*adjacency_arr->strides[0]
                         +j*adjacency_arr->strides[1]);

                if (*adj_value_ptr == true){

                    mask_value_ptr = (mask_arr->data + i*mask_arr->strides[0]
                    +j*mask_arr->strides[1]);

                    state_value_ptr = (state_arr->data + j*state_arr->strides[0]);

                    if ( (*(bool *) mask_value_ptr || *(bool *)state_value_ptr) ==  true){
                        sum++;
                    }
                    conn_ctr++;
                }
            }
            state_out_ptr = (state_out->data + i*state_out->strides[0]);
            if (conn_ctr < sum*2){
                *state_out_ptr =  true;
            }
            else {
                *state_out_ptr =  false;
            }
    }

    Py_DECREF(adjacency_arr);
    Py_DECREF(mask_arr);
    Py_DECREF(state_arr);
    return PyArray_Return(state_out);
}
