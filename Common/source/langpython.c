
/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#include "frontier.h"
#include "standard.h"

#define FRONTIER_PYTHON 1

#ifdef MACVERSION
#include <iac.h>
#endif

#include "error.h"
#include "file.h"
#include "memory.h"
#include "ops.h"
#include "resources.h"
#include "strings.h"
#include "lang.h"
#include "langipc.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "langhtml.h"
#include "langwinipc.h"
#include "process.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "op.h"
#include "opinternal.h"
#include "oplist.h"
#include "opverbs.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "shell.rsrc.h"
#include "timedate.h"
#include "winsocknetevents.h"
#ifdef flcomponent
#include "osacomponent.h"
#endif
#include "langpython.h"

#ifdef FRONTIER_PYTHON

#ifdef MACVERSION
#include <Python/Python.h>
#else
#endif

#include <time.h>

/* forward declarations */

static PyObject *newTableObject(hdlhashtable hdl);
static PyObject *newOutlineObject(void);
static PyObject *newWPTextObject(void);
static PyObject *newScriptObject(void);
static PyObject *newInternalValueObject(tyvaluetype vt);
static PyObject *newExternalValueObject(tyexternalid ei);

static time_t currentCookie = (time_t)0;
static PyObject *frontierModule = NULL;

typedef struct {
    PyObject *pyObject;
    time_t cookie;
} FrontierPythonPtr;

#if 0
/*
static boolean
getPythonPtr(FrontierPythonPtr *fpp, PyObject **pptr) {
    if (fpp->cookie == currentCookie) {
        *pptr = fpp->pyObject;
        return true;
    }
    *pptr = NULL;
    return false;
}

static boolean
frontierToPython(tyvaluerecord *tvr, PyObject **pptr) {
    if (tvr->valuetype != binaryvaluetype)
        return false;
    
    if (gethandlesize(tvr->data.binaryvalue) != sizeof(FrontierPythonPtr))
        return false;
    
    if (!getPythonPtr((FrontierPythonPtr *)(*(tvr->data.binaryvalue)), pptr))
        return false;
     
    return true;
}

static boolean
pythonToFrontier(PyObject *pptr, tyvaluerecord *tvr) {
    FrontierPythonPtr fpp;
    fpp.pyObject = pptr;
    fpp.cookie = currentCookie;
    
    if (!newheapvalue(&fpp, sizeof(FrontierPythonPtr), binaryvaluetype, tvr))
        return false;
    
    return true;
}
*/
#endif

static boolean convertPython(PyObject *, hdlhashtable, bigstring);

static boolean
convertPythonDict(PyObject *obj, hdlhashtable hdl, bigstring name) {
    PyObject *keys = PyDict_Keys(obj);
    int len = PyList_Size(keys);
    int i;
    
    hdlhashtable newTable;
    if (!langassignnewtablevalue(hdl, name, &newTable))
        return false;
    
    for (i = 0; i < len; i++) {
        PyObject *k = PyList_GetItem(keys, i);
        PyObject *v = PyDict_GetItem(obj, k);
        char *key = PyString_AsString(k);
        
        bigstring tn;
        
        copyctopstring(key, tn);
        
        if (!convertPython(v, newTable, tn))
            return false;
    }
    
    return true;
}

static boolean
convertPython(PyObject *obj, hdlhashtable hdl, bigstring name) {
    PyTypeObject *ob_type = obj->ob_type;
    tyvaluerecord tvr;
    
    setbooleanvalue(false, &tvr);
    
    if (ob_type == &PyFloat_Type) {
        
        double d = PyFloat_AsDouble(obj);
        if (!setdoublevalue(d, &tvr))
            return false;
        
    } else if (ob_type == &PyInt_Type) {
        
        long l = PyInt_AsLong(obj);
        if (!setlongvalue(l, &tvr))
            return false;
        
    } else if (ob_type == &PyLong_Type) {
        
        long l = PyInt_AsLong(obj);
        if (!setlongvalue(l, &tvr))
            return false;
        
    } else if (ob_type == &PyString_Type) {
        
        char *s = PyString_AsString(obj);
        if (!newheapvalue(s, strlen(s), stringvaluetype, &tvr))
            return false;
        
    } else if (ob_type == &PyDict_Type) {
        
        return convertPythonDict(obj, hdl, name);
        
    } else if (ob_type == &PyComplex_Type) {
        
        hdlhashtable newTable;
        if (!langassignnewtablevalue(hdl, name, &newTable))
            return false;
        
        tyvaluerecord realPart;
        tyvaluerecord imagPart;
        
        double r, i;
        
        r = PyComplex_RealAsDouble(obj);
        i = PyComplex_ImagAsDouble(obj);
        
        if (!setdoublevalue(r, &realPart))
            return false;
        
        if (!setdoublevalue(i, &imagPart))
            return false;
        
        exemptfromtmpstack(&realPart);
        exemptfromtmpstack(&imagPart);
        
        bigstring realName;
        bigstring imagName;
        
        copyctopstring("real", realName);
        copyctopstring("imag", imagName);
        
        if (!hashtableassign(newTable, realName, realPart))
            return false;
        
        if (!hashtableassign(newTable, imagName, imagPart))
            return false;
        
        return true;
        
    } else if (ob_type == &PyInstance_Type) {
        
        return convertPythonDict(PyObject_GetAttrString(obj, "__dict__"), hdl, name);

    } else if (ob_type == &PyList_Type) {
        //printf("list\n");
    } else if (ob_type == &PyFile_Type) {
        //printf("file\n");
    } else if (ob_type == &PyMethod_Type) {
        //printf("method\n");
    } else if (ob_type == &PyModule_Type) {
        //printf("module\n");
    } else if (ob_type == &PySeqIter_Type) {
        //printf("seqiter\n");
    } else if (ob_type == &PySlice_Type) {
        //printf("slice\n");
    } else if (ob_type == &PyTuple_Type) {
        //printf("tuple\n");
    } else if (ob_type == &PyType_Type) {
        //printf("type\n");
    } else if (ob_type == &PyUnicode_Type) {
        //printf("unicode\n");
    }
    
    exemptfromtmpstack(&tvr);
    if (!hashtableassign(hdl, name, tvr))
        return false;
    
    return true;
}

static boolean
convertfrontiervalue(tyvaluerecord *tvr, PyObject **pptr) {
    
    tyexternalid extType = ctexternalprocessors;
    
    switch (tvr->valuetype) {
		case stringvaluetype: {
            int len = gethandlesize(tvr->data.stringvalue);
            char *val = malloc(len + 1);
            strncpy(val, *(tvr->data.stringvalue), len);
            val[len] = 0;
            
            *pptr = Py_BuildValue("s", val);
            
            free(val);
            
            break;
        }
            
		case novaluetype:
            Py_INCREF(Py_None);
            *pptr = Py_None;
            break;
            
            /*
             case charvaluetype:
             case filespecvaluetype:
             case enumvaluetype:
             case directionvaluetype:
             case datevaluetype:
             case addressvaluetype: {
                 if (!coercetostring(tvr))
                     return false;
                 
                 int len = gethandlesize(tvr->data.stringvalue);
                 char *val = malloc(len + 1);
                 strncpy(val, *(tvr->data.stringvalue), len);
                 val[len] = 0;
                 
                 *pptr = Py_BuildValue("s", val);
                 
                 free(val);
             }
                 */
            
		case booleanvaluetype: {
            int val = 0;
            if (tvr->data.flvalue)
                val = -1;
            
            *pptr = Py_BuildValue("i", val);
            
            break;
        }
            
		case intvaluetype:
            *pptr = Py_BuildValue("i", tvr->data.intvalue);
            break;
            
		case longvaluetype:
            *pptr = Py_BuildValue("i", tvr->data.longvalue);
            break;
            
		case fixedvaluetype: {
			double x = (double)(((*tvr).data.longvalue) / 65536);
			*pptr = Py_BuildValue("d", x);
			break;
        }
            
		case singlevaluetype:
            *pptr = Py_BuildValue("f", tvr->data.singlevalue);
            break;
            
		case doublevaluetype:
            *pptr = Py_BuildValue("d", **(tvr->data.doublevalue));
            break;
            
		case pointvaluetype:
            *pptr = Py_BuildValue("(ii)", tvr->data.pointvalue.v, tvr->data.pointvalue.h);
            break;
            
		case rectvaluetype:
            *pptr = Py_BuildValue("(iiii)", (**(tvr->data.rectvalue)).top, (**(tvr->data.rectvalue)).left, (**(tvr->data.rectvalue)).bottom, (**(tvr->data.rectvalue)).right);
            break;
            
		case rgbvaluetype:
            *pptr = Py_BuildValue("(iii)", (**(tvr->data.rgbvalue)).red, (**(tvr->data.rgbvalue)).green, (**(tvr->data.rgbvalue)).blue);
            break;
            
        case externalvaluetype: {
            
            // alrighty then.  What type are we looking at?
            //
            extType = langexternalgettype(*tvr);
            
            // now we need to look up the python class that handles
            // this particular external type, and instantiate it.
            //
            // For tables, we need to store the name and a handle to the
            // table.
            
            switch (extType) {
                case idtableprocessor: {
                    hdlhashtable table;
                    if (!langexternalvaltotable(*tvr, &table, HNoNode))
                        return false;
                        
                    *pptr = newTableObject(table);
                    
                    break;
                }
                
                case idoutlineprocessor: {

                    *pptr = newOutlineObject();
                    
                    break;
                }
                
                case idwordprocessor: {

                    *pptr = newWPTextObject();
                    
                    break;
                }
                
                case idscriptprocessor: {
                
                    *pptr = newScriptObject();
                    
                    break;
                }
                
                default: {
                    *pptr = newExternalValueObject(extType);
                    break;
                }
            }
            
            break;
        }
            
            
        // support these later
        //
        //case binaryvaluetype:
        //case listvaluetype:
        //case recordvaluetype:
        
        // maybe support these
        //
        //case patternvaluetype:
        //case objspecvaluetype:
        //case aliasvaluetype:
        //case codevaluetype:
        //case ostypevaluetype:
            
        default: {
            if (extType != ctexternalprocessors) {
                *pptr = newExternalValueObject(extType);
            } else {
                *pptr = newInternalValueObject(tvr->valuetype);
            }
            
            break;
        }
    }
    return true;
}

static char *
namestacktoaddress(PyObject *nameStack) {
    int len = PyList_GET_SIZE(nameStack);
    int addressLen = 0;
    for (int i = 0; i < len; i++)
        addressLen += PyString_GET_SIZE(PyList_GET_ITEM(nameStack, i));
    addressLen += len * 5;
    
    char *s = (char *)malloc(addressLen + 1);
    *s = 0;
    for (int i = 0; i < len; i++) {
        strcat(s, "[\"");
        strcat(s, PyString_AsString(PyList_GET_ITEM(nameStack, i)));
        strcat(s, "\"].");
    }
    s[--addressLen] = 0;
    
    return s;
}

/* the python library side */

static PyObject *
fetchAux(char *as) {
    tyvaluerecord tvr;
    
    if (!newheapvalue(as, strlen(as), stringvaluetype, &tvr)) {
        PyErr_SetString(PyExc_SystemError, "Cannot allocate memory");
        return NULL;
    }
    
    if (!coercetoaddress(&tvr)) {
        char msg[300];
        sprintf(msg, "Invalid address %s", as);
        PyErr_SetString(PyExc_KeyError, msg);
        return NULL;
    }
    
    bigstring loc;
    hdlhashtable adr;
    
    if (!getaddressvalue(tvr, &adr, loc)) {
        char msg[300];
        sprintf(msg, "Invalid address %s", as);
        PyErr_SetString(PyExc_KeyError, msg);
        return NULL;
    }
    
    tyvaluerecord val;
    hdlhashnode node;
    
    if (!hashtablelookup(adr, loc, &val, &node)) {
        char msg[300];
        sprintf(msg, "Cannot find key %s in table", as);
        PyErr_SetString(PyExc_KeyError, msg);
        return NULL;
    }
    
    char cname[stringsize(loc)];
    copyptocstring(loc, cname);
    
    PyObject *result;
    if (!convertfrontiervalue(&val, &result)) {
        PyErr_SetString(PyExc_SystemError, "Cannot convert value");
        return NULL;
    }
    
    return result;
}

static PyObject*
fetch(PyObject *self, PyObject *args) {
    char *as;
    
    if (!PyArg_ParseTuple(args, "s:fetch", &as)) {
        PyErr_SetString(PyExc_ValueError, "Requires a string representing an address in the Frontier database");
        return NULL;
    }
    
    return fetchAux(as);
}

static PyObject*
evaluateString(PyObject *self, PyObject *args) {
    char *s;
    
    if (!PyArg_ParseTuple(args, "s:evaluate", &s)) {
        PyErr_SetString(PyExc_ValueError, "Couldn't execute because evaluate requires a string argument.");
        return NULL;
    }
    
    Handle htext;
    if (!newfilledhandle(s, strlen(s), &htext)) {
        PyErr_SetString(PyExc_SystemError, "Couldn't execute because I couldn't allocate space for the string.");
        return NULL;
    }
    
    tyvaluerecord v;
    if (!langrun(htext, &v)) {
        PyErr_SetString(PyExc_SyntaxError, "Couldn't execute because of a syntax error in the UserTalk code.");
        return NULL;
    }
    
    PyObject *result;
    if (!convertfrontiervalue(&v, &result)) {
        PyErr_SetString(PyExc_SystemError, "Couldn't execute because of an error converting the UserTalk return value.");
        return NULL;
    }
    
    return result;
}

static PyObject *
nameStackToAddress(PyObject *self, PyObject *args) {
    PyObject *nameStack;
    if (!PyArg_ParseTuple(args, "O:nameStackToAddress", &nameStack))
        return NULL;
    
    char *s = namestacktoaddress(nameStack);
    
    PyObject *result = PyString_FromString(s);
    
    free(s);
    
    return result;
}

static PyMethodDef embMethods[] = {
    {"fetch", fetch, METH_VARARGS,
        "fetch the value at the frontier address"},
    {"evaluate", evaluateString, METH_VARARGS,
        "Compile and execute the string as UserTalk code."},
    {"nameStackToAddress", nameStackToAddress, METH_VARARGS,
        "turn a name stack into a frontier address specifier"},
    {NULL, NULL, 0, NULL}
};

/* python classes to handle frontier types */

typedef struct {
    PyObject_HEAD
    tyvaluetype vartype;
} frontier_InternalValue;

static PyTypeObject frontier_InternalValueType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
    "frontier.InternalValue",       /*tp_name*/
    sizeof(frontier_InternalValue), /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    0,                              /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,             /*tp_flags*/
    "Frontier Internal Value objects",       /* tp_doc */
};

typedef struct {
    PyObject_HEAD
    tyexternalid extvartype;
} frontier_ExternalValue;

static PyTypeObject frontier_ExternalValueType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
    "frontier.ExternalValue",       /*tp_name*/
    sizeof(frontier_ExternalValue), /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    0,                              /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,             /*tp_flags*/
    "Frontier External Value objects",       /* tp_doc */
};

typedef struct {
    PyObject_HEAD
    hdlhashtable table;
} frontier_TableObject;

static PyObject *Table_keys(frontier_TableObject *);
static PyObject *Table_newTable(frontier_TableObject *, PyObject *);

static PyMethodDef tableMethods[] = {
    {"keys", (PyCFunction)Table_keys, METH_NOARGS, "return a list of the keys in this table"},
    {"newTable", (PyCFunction)Table_newTable, METH_VARARGS, "create a new table within this table"},
    {NULL}
};

static PyObject *
Table_get(frontier_TableObject *self, char *name) {
    bigstring loc;
    
    copyctopstring(name, loc);
    
    tyvaluerecord val;
    hdlhashnode node;
    
    if (!hashtablelookup(self->table, loc, &val, &node)) {
        char msg[300];
        sprintf(msg, "No table entry named %s.", name);
        PyErr_SetString(PyExc_KeyError, msg);
        return NULL;
    }
        
    PyObject *result;
        
    if (!convertfrontiervalue(&val, &result)) {
        char msg[300];
        sprintf(msg, "Cannot convert table entry named %s.", name);
        PyErr_SetString(PyExc_SystemError, msg);
        return NULL;
    }
 
    return result;
}

static PyObject *
Table_getattr(frontier_TableObject *self, char *name) {
    PyObject *method = Py_FindMethod(tableMethods, (PyObject *)self, name);
    if (method != NULL)
        return method;
        
    PyErr_Clear();

    return Table_get(self, name);
}

static int
Table_setattr(frontier_TableObject *self, char *name, PyObject *val) {
    bigstring loc;
    
    copyctopstring(name, loc);
    
    if (!convertPython(val, self->table, loc))
        return -1;
        
    return 0;
}

static int
Table_numEntries(frontier_TableObject *self) {
    long count;
    
    if (!hashcountitems(self->table, &count))
        return -1;
    
    return (int)count;
}

static PyObject *
Table_getIndex(frontier_TableObject *self, int index) {
    hdlhashnode node;
    if (!hashgetnthnode(self->table, index, &node)) {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }
    
    PyObject *result;
    if (!convertfrontiervalue(&((**node).val), &result)) {
        PyErr_SetString(PyExc_ValueError, "Cannot convert value");
        return NULL;
    }
    
    return result;
}

static int
Table_setIndex(frontier_TableObject *self, int index, PyObject *val) {
    hdlhashnode node;
    if (!hashgetnthnode(self->table, index, &node)) {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }
    
    if (!convertPython(val, self->table, (**node).hashkey))
        return -1;
        
    return 0;
}

static PyObject *
Table_getItem(frontier_TableObject *self, PyObject *key) {
    if (PyInt_Check(key))
        return Table_getIndex(self, PyInt_AsLong(key));
    char *keyName = PyString_AsString(PyObject_Str(key));
    return Table_get(self, keyName);    
}

static int
Table_setItem(frontier_TableObject *self, PyObject *key, PyObject *val) {
    if (PyInt_Check(key))
        return Table_setIndex(self, PyInt_AsLong(key), val);
    char *keyName = PyString_AsString(PyObject_Str(key));
    return Table_setattr(self, keyName, val);
}

PySequenceMethods seq = {
    (inquiry)Table_numEntries,      //inquiry sq_length;
    0,                              //binaryfunc sq_concat;
    0,                              //intargfunc sq_repeat;
    (intargfunc)Table_getIndex,     //intargfunc sq_item;
    0,                              //intintargfunc sq_slice;
    (intobjargproc)Table_setIndex,  //intobjargproc sq_ass_item;
    0,                              //intintobjargproc sq_ass_slice;
    0,                              //objobjproc sq_contains;
    /* Added in release 2.0 */
    0,                              //binaryfunc sq_inplace_concat;
    0,                              //intargfunc sq_inplace_repeat;
};

PyMappingMethods mapping = {
    (inquiry)Table_numEntries,
    (binaryfunc)Table_getItem,
    (objobjargproc)Table_setItem
};

static boolean
Table_keysCallback(hdlhashnode node, ptrvoid refcon) {
    PyObject *keyList = (PyObject *)refcon;
    
    char name[stringsize((**node).hashkey)];
    copyptocstring((**node).hashkey, name);
    
    PyList_Append(keyList, PyString_FromString(name));
    
    return true;
}

static PyObject *
Table_keys(frontier_TableObject *self) {
    PyObject *keyList = PyList_New(0);

    if (!hashtablevisit(self->table, Table_keysCallback, (ptrvoid)keyList)) {
        PyErr_SetString(PyExc_SystemError, "Cannot create key list");
        return NULL;
    }

    return keyList;
}

static PyObject *
Table_newTable(frontier_TableObject *self, PyObject *args) {
    char *name;
    if (!PyArg_ParseTuple(args, "s:newTable", &name)) {
        PyErr_SetString(PyExc_TypeError, "Table.newTable requires a string argument");
        return NULL;
    }
    
    bigstring loc;
    
    copyctopstring(name, loc);
    
    hdlhashtable newTable;
    if (!langassignnewtablevalue(self->table, loc, &newTable)) {
        PyErr_SetString(PyExc_SystemError, "Cannot create table");
        return NULL;
    }
    
    return newTableObject(newTable);
}

static PyTypeObject frontier_TableType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /*ob_size*/
    "frontier.Table",             /*tp_name*/
    sizeof(frontier_TableObject), /*tp_basicsize*/
    0,                            /*tp_itemsize*/
    0,                            /*tp_dealloc*/
    0,                            /*tp_print*/
    (getattrfunc)Table_getattr,   /*tp_getattr*/
    (setattrfunc)Table_setattr,   /*tp_setattr*/
    0,                            /*tp_compare*/
    0,                            /*tp_repr*/
    0,                            /*tp_as_number*/
    &seq,                         /*tp_as_sequence*/
    &mapping,                     /*tp_as_mapping*/
    0,                            /*tp_hash */
    0,                            /*tp_call*/
    0,                            /*tp_str*/
    0,                            /*tp_getattro*/
    0,                            /*tp_setattro*/
    0,                            /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,           /*tp_flags*/
    "Frontier Table objects",     /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    tableMethods,                 /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    0,                            /* tp_new */
};

typedef struct {
    PyObject_HEAD
} frontier_OutlineObject;

static PyTypeObject frontier_OutlineType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
    "frontier.Outline",             /*tp_name*/
    sizeof(frontier_OutlineObject), /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    0,                              /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,             /*tp_flags*/
    "Frontier Outline objects",     /* tp_doc */
};

typedef struct {
    PyObject_HEAD
} frontier_WPTextObject;

static PyTypeObject frontier_WPTextType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
    "frontier.WPText",              /*tp_name*/
    sizeof(frontier_WPTextObject),  /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    0,                              /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,             /*tp_flags*/
    "Frontier WPText objects",      /* tp_doc */
};

typedef struct {
    PyObject_HEAD
} frontier_ScriptObject;

static PyTypeObject frontier_ScriptType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
    "frontier.Script",              /*tp_name*/
    sizeof(frontier_ScriptObject),  /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    0,                              /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,             /*tp_flags*/
    "Frontier Script objects",      /* tp_doc */
};

static boolean
addType(PyObject *m, PyTypeObject *to, char *name) {
    to->tp_new = PyType_GenericNew;
    if (PyType_Ready(to) < 0)
        return false;        
        
    Py_INCREF(to);
    PyModule_AddObject(m, name, (PyObject *)to);
    
    return true;
}

static boolean
addModuleVars(PyObject *m) {

    boolean result = addType(m, &frontier_TableType, "Table") &&
                     addType(m, &frontier_OutlineType, "Outline") &&
                     addType(m, &frontier_WPTextType, "WPText") &&
                     addType(m, &frontier_ScriptType, "Script") &&
                     addType(m, &frontier_InternalValueType, "InternalValue") &&
                     addType(m, &frontier_ExternalValueType, "ExternalValue");
                     
    if (!result)
        return false;
        
    PyModule_AddObject(m, "examples", fetchAux("examples"));
    PyModule_AddObject(m, "scratchpad", fetchAux("scratchpad"));
    PyModule_AddObject(m, "suites", fetchAux("suites"));
    PyModule_AddObject(m, "user", fetchAux("user"));
    PyModule_AddObject(m, "websites", fetchAux("websites"));
    PyModule_AddObject(m, "workspace", fetchAux("workspace"));
    
    return true;
}

// utility methods for creating frontier objects

static PyObject *
newTableObject(hdlhashtable hdl) {
    PyObject *class = PyObject_GetAttrString(frontierModule, "Table");
    frontier_TableObject *obj = PyObject_New(frontier_TableObject, (PyTypeObject *)class);
    obj->table = hdl;
    return (PyObject *)obj;
}

static PyObject *
newOutlineObject() {
    PyObject *class = PyObject_GetAttrString(frontierModule, "Outline");
    frontier_OutlineObject *obj = PyObject_New(frontier_OutlineObject, (PyTypeObject *)class);
    return (PyObject *)obj;
}

static PyObject *
newWPTextObject() {
    PyObject *class = PyObject_GetAttrString(frontierModule, "WPText");
    frontier_WPTextObject *obj = PyObject_New(frontier_WPTextObject, (PyTypeObject *)class);
    return (PyObject *)obj;
}

static PyObject *
newScriptObject() {
    PyObject *class = PyObject_GetAttrString(frontierModule, "Script");
    frontier_ScriptObject *obj = PyObject_New(frontier_ScriptObject, (PyTypeObject *)class);
    return (PyObject *)obj;
}

static PyObject *
newInternalValueObject(tyvaluetype vt) {
    PyObject *class = PyObject_GetAttrString(frontierModule, "InternalValue");
    frontier_InternalValue *obj = PyObject_New(frontier_InternalValue, (PyTypeObject *)class);
    obj->vartype = vt;
    return (PyObject *)obj;
}

static PyObject *
newExternalValueObject(tyexternalid ei) {
    PyObject *class = PyObject_GetAttrString(frontierModule, "ExternalValue");
    frontier_ExternalValue *obj = PyObject_New(frontier_ExternalValue, (PyTypeObject *)class);
    obj->extvartype = ei;
    return (PyObject *)obj;
}

/* the frontier kernel verb side */

typedef enum typythonverbtoken {
    startinterpreterfunc,
    stopinterpreterfunc,
    runstringfunc,
    ctpythonverbs
} typythonverbtoken;

PyObject *globals = 0;

static boolean initGlobals() {
    if (globals)
        return true;
    
    globals = PyDict_New();
    
    if (PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins()))
        return false;
    
    if (PyDict_SetItemString(globals, "__name__", PyString_FromString("__main__")))
        return false;
    
    if (PyDict_SetItemString(globals, "__doc__", PyString_FromString("Frontier")))
        return false;
    
    return true;
}

static boolean disposeGlobals() {
    if (globals) {
        Py_DECREF(globals);
        globals = 0;
    }
    return true;
}

static boolean startInterpreter() {
    if (!Py_IsInitialized()) {
        Py_Initialize();
        initGlobals();
        
        frontierModule = Py_InitModule3("frontier", embMethods, "Python to Frontier interface library");
        
        if (!addModuleVars(frontierModule))
            return false;
            
        currentCookie = time(NULL);
    }
    return true;
}

static boolean stopInterpreter() {
    if (Py_IsInitialized()) {
        disposeGlobals();
        Py_Finalize();
        frontierModule = NULL;
        currentCookie = (time_t)0;
    }
    return true;
}

boolean runstringverb(hdltreenode hp1, tyvaluerecord *v) {

    // if Python hasn't been started, start it and create our globals.
    //
    startInterpreter();
    
    // set up default frontier return value
    //
    if (!setbooleanvalue(true, v))
        return false;

    Handle pythonCode;
    tyvaluerecord adrOutput;
    tyvaluerecord adrError;
	
    // first param: handle to python code string
    //
    if (!gettextvalue (hp1, 1, &pythonCode))
		return (false);
    
    // second param: pointer to var to store output
    //
    if (!getaddressparam(hp1, 2, &adrOutput))
        return (false);
    
    // third param: pointer to var for error output
    //
    if (!getaddressparam(hp1, 3, &adrError))
        return false;
    
    hdlhashtable htableOutput;
    hdlhashtable htableError;
    bigstring locOutput;
    bigstring locError;
    
    // derefernce pointer to output variable
    //
    if (!getaddressvalue(adrOutput, &htableOutput, locOutput))
        return (false);
    
    // dereference pointer to error output
    //
    if (!getaddressvalue(adrError, &htableError, locError))
        return (false);
    
    // convert the text to a C string
    //
    if (!enlargehandle(pythonCode, gethandlesize(pythonCode) + 1, "\0"))
        return false;
        
    PyObject *locals = globals; //PyDict_New();
    PyObject *result;
    
    // set up the interpreter to capture the output
    //
    result = PyRun_String("from cStringIO import StringIO; import sys; _f_old_output = sys.stdout; _f_output = StringIO(); sys.stdout = _f_output; _f_old_err = sys.stderr; _f_err = StringIO(); sys.stderr = _f_err; del StringIO", Py_file_input, globals, locals);
    if (result) Py_DECREF(result);
    
    // run the python code
    //
    result = PyRun_String(*pythonCode, Py_file_input, globals, locals);
    if (result) Py_DECREF(result);    
    PyObject *err = PyErr_Occurred();

    if (err) {
        PyErr_Print();

        // get the error output
        //
        PyObject *ferr = PyDict_GetItemString(locals, "_f_err");
        PyObject *outputString = PyObject_CallMethod(ferr, "getvalue", NULL);
        result = PyObject_CallMethod(ferr, "close", NULL);
        if (result) Py_DECREF(result);    
        
        char *output = PyString_AsString(outputString);

        tyvaluerecord errorValue;

        if (!newheapvalue(output, strlen(output), stringvaluetype, &errorValue)) {
            Py_DECREF(outputString);
            return false;
        }
        
        exemptfromtmpstack(&errorValue);
    
        if (!hashtableassign(htableError, locError, errorValue)) {
            Py_DECREF(outputString);
            return false;
        }

        if (!setbooleanvalue(false, v)) {
            Py_DECREF(outputString);
            return false;
        }

        Py_DECREF(outputString);
    }

	// get the output
    //
    PyObject *foutput = PyDict_GetItemString(locals, "_f_output");
    PyObject *outputString = PyObject_CallMethod(foutput, "getvalue", NULL);
    result = PyObject_CallMethod(foutput, "close", NULL);
    if (result) Py_DECREF(result);    

    // convert the output from a python string into a cstring
    //
    char *output = PyString_AsString(outputString);

    // copy the output into the frontier variable that was passed in
    //
    tyvaluerecord outputValue;
    
    boolean retcode = true;
    
    if (!newheapvalue(output, strlen(output), stringvaluetype, &outputValue))
        retcode = false;
    
    exemptfromtmpstack(&outputValue);
    
    if (retcode && !hashtableassign(htableOutput, locOutput, outputValue))
        retcode = false;
    
    // erase all sign of us
    //
    result = PyRun_String("sys.stdout = _f_old_output; sys.stderr = _f_old_err", Py_file_input, globals, locals);
    if (result) Py_DECREF(result);
    
    Py_DECREF(outputString);
    //Py_DECREF(locals);
    
    return retcode;
}

static boolean pythonfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
    hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*by default, string functions return false*/
	
	switch (token) {
        
		case startinterpreterfunc:
            return startInterpreter();
            
        case stopinterpreterfunc:
            return stopInterpreter();
            
        case runstringfunc:
            return runstringverb(hp1, v);
            
		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			return (false);
    } /*switch*/
} /*htmlfunctionvalue*/


boolean pythoninitverbs (void) {
    return (loadfunctionprocessor (idpythonverbs, &pythonfunctionvalue));
}


#endif
