/*
 *
 *  Copyright (C) 2010 MeVis Medical Solutions AG All Rights Reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  Further, this software is distributed without any warranty that it is
 *  free of the rightful claim of any third person regarding infringement
 *  or the like.  Any license provided herein, whether implied or
 *  otherwise, applies only to this software file.  Patent licenses, if
 *  any, provided herein do not apply to combinations of this program with
 *  other software, or any other product whatsoever.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact information: MeVis Medical Solutions AG, Universitaetsallee 29,
 *  28359 Bremen, Germany or:
 *
 *  http://www.mevis.de
 *
 */

//----------------------------------------------------------------------------------
/*!
// \file    PythonQtConversion.cpp
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include "PythonQtConversion.h"
#include "PythonQtPythonInclude.h"
#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QVariant>
#include <climits>
#include <iostream>
#include <limits>

QHash<int, PythonQtConvertMetaTypeToPythonCB *> PythonQtConv::_metaTypeToPythonConverters;
QHash<int, PythonQtConvertPythonToMetaTypeCB *> PythonQtConv::_pythonToMetaTypeConverters;

PythonQtConvertPythonSequenceToQVariantListCB *PythonQtConv::_pythonSequenceToQVariantListCB = NULL;

PyObject *PythonQtConv::GetPyBool(bool val)
{
  PyObject *r = val ? Py_True : Py_False;
  Py_INCREF(r);
  return r;
}

PyObject *PythonQtConv::convertQtValueToPythonInternal(int type, const void *data)
{
  switch (type)
  {
  case QMetaType::Void:
    Py_INCREF(Py_None);
    return Py_None;
  case QMetaType::Char:
    return PyInt_FromLong(*((char *)data));
  case QMetaType::UChar:
    return PyInt_FromLong(*((unsigned char *)data));
  case QMetaType::Short:
    return PyInt_FromLong(*((short *)data));
  case QMetaType::UShort:
    return PyInt_FromLong(*((unsigned short *)data));
  case QMetaType::Long:
    return PyInt_FromLong(*((long *)data));
  case QMetaType::ULong:
    // does not fit into simple int of python
    return PyLong_FromUnsignedLong(*((unsigned long *)data));
  case QMetaType::Bool:
    return PythonQtConv::GetPyBool(*((bool *)data));
  case QMetaType::Int:
    return PyInt_FromLong(*((int *)data));
  case QMetaType::UInt:
    // does not fit into simple int of python
    return PyLong_FromUnsignedLong(*((unsigned int *)data));
  case QMetaType::QChar:
    return PyInt_FromLong(*((unsigned short *)data));
  case QMetaType::Float:
    return PyFloat_FromDouble(*((float *)data));
  case QMetaType::Double:
    return PyFloat_FromDouble(*((double *)data));
  case QMetaType::LongLong:
    return PyLong_FromLongLong(*((qint64 *)data));
  case QMetaType::ULongLong:
    return PyLong_FromUnsignedLongLong(*((quint64 *)data));
  case QMetaType::QVariantHash:
    return PythonQtConv::QVariantHashToPyObject(*((QVariantHash *)data));
  case QMetaType::QVariantMap:
    return PythonQtConv::QVariantMapToPyObject(*((QVariantMap *)data));
  case QMetaType::QVariantList:
    return PythonQtConv::QVariantListToPyObject(*((QVariantList *)data));
  case QMetaType::QString:
    return PythonQtConv::QStringToPyObject(*((QString *)data));
  case QMetaType::QStringList:
    return PythonQtConv::QStringListToPyObject(*((QStringList *)data));

#if QT_VERSION >= 0x040800
  case QMetaType::QVariant:
#endif
    return PythonQtConv::QVariantToPyObject(*((QVariant *)data));

  default:
    if (type >= QMetaType::User || type == QMetaType::QByteArrayList)
    {
      // if a converter is registered, we use is:
      PythonQtConvertMetaTypeToPythonCB *converter = _metaTypeToPythonConverters.value(type);
      if (converter)
      {
        return (*converter)(data, type);
      }
    }
    std::cerr << "Unknown type that can not be converted to Python: " << type << ":"
              << QMetaType(type).name() << ", in " << __FILE__ << ":" << __LINE__ << std::endl;
  }
  Py_INCREF(Py_None);
  return Py_None;
}

QStringList PythonQtConv::PyObjToStringList(PyObject *val, bool strict, bool &ok)
{
  QStringList v;
  ok = false;
  // if we are strict, we do not want to convert a string to a stringlist
  // (strings in python are detected to be sequences)
  if (strict && (val->ob_type == &PyBytes_Type || PyUnicode_Check(val)))
  {
    return v;
  }
  if (PySequence_Check(val))
  {
    int count = PySequence_Size(val);
    if (count >= 0)
    {
      for (int i = 0; i < count; i++)
      {
        PyObject *value = PySequence_GetItem(val, i);
        v.append(PyObjGetString(value, false, ok));
        Py_XDECREF(value);
      }
      ok = true;
    }
  }
  return v;
}

QString PythonQtConv::PyObjGetRepresentation(PyObject *val)
{
  QString r;
  PyObject *str = PyObject_Repr(val);
  if (str)
  {
#ifdef PY3K
    r = PyObjGetString(str);
#else
    r = QString(PyString_AS_STRING(str));
#endif
    Py_DECREF(str);
  }
  return r;
}

QString PythonQtConv::PyObjGetString(PyObject *val, bool strict, bool &ok)
{
  QString r;
  ok = true;
#ifndef PY3K
  // in Python 3, we don't want to convert to QString, since we don't know anything about the
  // encoding in Python 2, we assume the default for str is latin-1
  if (val->ob_type == &PyBytes_Type)
  {
    r = QString::fromLatin1(PyBytes_AS_STRING(val));
  }
  else
#endif
      if (PyUnicode_Check(val))
  {
#ifdef PY3K
    r = QString::fromUtf8(PyUnicode_AsUTF8(val));
#else
    PyObject *ptmp = PyUnicode_AsUTF8String(val);
    if (ptmp)
    {
      r = QString::fromUtf8(PyString_AS_STRING(ptmp));
      Py_DECREF(ptmp);
    }
#endif
  }
  else if (!strict)
  {
    PyObject *str = PyObject_Str(val);
    if (str)
    {
#ifdef PY3K
      r = QString::fromUtf8(PyUnicode_AsUTF8(str));
#else
      r = QString(PyString_AS_STRING(str));
#endif
      Py_DECREF(str);
    }
    else
    {
      ok = false;
    }
  }
  else
  {
    ok = false;
  }
  return r;
}

QByteArray PythonQtConv::PyObjGetBytes(PyObject *val, bool /*strict*/, bool &ok)
{
  // TODO: support buffer objects in general
  QByteArray r;
  ok = true;
  if (PyBytes_Check(val))
  {
    r = QByteArray(PyBytes_AS_STRING(val), PyBytes_GET_SIZE(val));
  }
  else
  {
    ok = false;
  }
  return r;
}

bool PythonQtConv::PyObjGetBool(PyObject *val, bool strict, bool &ok)
{
  bool d = false;
  ok = false;
  if (val == Py_False)
  {
    d = false;
    ok = true;
  }
  else if (val == Py_True)
  {
    d = true;
    ok = true;
  }
  else if (!strict)
  {
    int result = PyObject_IsTrue(val);
    d = (result == 1);
    // the result is -1 if an error occurred, handle this:
    ok = (result != -1);
  }
  return d;
}

int PythonQtConv::PyObjGetInt(PyObject *val, bool strict, bool &ok)
{
  int d = 0;
  ok = true;
  if (val->ob_type == &PyInt_Type)
  {
    d = PyInt_AS_LONG(val);
  }
  else if (!strict)
  {
    if (PyObject_TypeCheck(val, &PyInt_Type))
    {
      // support for derived int classes, e.g. for our enums
      d = PyInt_AS_LONG(val);
    }
    else if (val->ob_type == &PyFloat_Type)
    {
      d = floor(PyFloat_AS_DOUBLE(val));
    }
    else if (val->ob_type == &PyLong_Type)
    {
      // handle error on overflow!
      d = PyLong_AsLong(val);
    }
    else if (val == Py_False)
    {
      d = 0;
    }
    else if (val == Py_True)
    {
      d = 1;
    }
    else
    {
      PyErr_Clear();
      // PyInt_AsLong will try conversion to an int if the object is not an int:
      d = PyInt_AsLong(val);
      if (PyErr_Occurred())
      {
        ok = false;
        PyErr_Clear();
      }
    }
  }
  else
  {
    ok = false;
  }
  return d;
}

qint64 PythonQtConv::PyObjGetLongLong(PyObject *val, bool strict, bool &ok)
{
  qint64 d = 0;
  ok = true;
#ifndef PY3K
  if (val->ob_type == &PyInt_Type)
  {
    d = PyInt_AS_LONG(val);
  }
  else
#endif
      if (val->ob_type == &PyLong_Type)
  {
    d = PyLong_AsLongLong(val);
  }
  else if (!strict)
  {
    if (PyObject_TypeCheck(val, &PyInt_Type))
    {
      // support for derived int classes, e.g. for our enums
      d = PyInt_AS_LONG(val);
    }
    else if (val->ob_type == &PyFloat_Type)
    {
      d = floor(PyFloat_AS_DOUBLE(val));
    }
    else if (val == Py_False)
    {
      d = 0;
    }
    else if (val == Py_True)
    {
      d = 1;
    }
    else
    {
      PyErr_Clear();
      // PyLong_AsLongLong will try conversion to an int if the object is not an int:
      d = PyLong_AsLongLong(val);
      if (PyErr_Occurred())
      {
        ok = false;
        PyErr_Clear();
      }
    }
  }
  else
  {
    ok = false;
  }
  return d;
}

quint64 PythonQtConv::PyObjGetULongLong(PyObject *val, bool strict, bool &ok)
{
  quint64 d = 0;
  ok = true;
#ifndef PY3K
  if (Py_TYPE(val) == &PyInt_Type)
  {
    d = PyInt_AS_LONG(val);
  }
  else
#endif
      if (Py_TYPE(val) == &PyLong_Type)
  {
    d = PyLong_AsLongLong(val);
  }
  else if (!strict)
  {
    if (PyObject_TypeCheck(val, &PyInt_Type))
    {
      // support for derived int classes, e.g. for our enums
      d = PyInt_AS_LONG(val);
    }
    else if (val->ob_type == &PyFloat_Type)
    {
      d = floor(PyFloat_AS_DOUBLE(val));
    }
    else if (val == Py_False)
    {
      d = 0;
    }
    else if (val == Py_True)
    {
      d = 1;
    }
    else
    {
      PyErr_Clear();
      // PyLong_AsLongLong will try conversion to an int if the object is not an int:
      d = PyLong_AsLongLong(val);
      if (PyErr_Occurred())
      {
        PyErr_Clear();
        ok = false;
      }
    }
  }
  else
  {
    ok = false;
  }
  return d;
}

double PythonQtConv::PyObjGetDouble(PyObject *val, bool strict, bool &ok)
{
  double d = 0;
  ok = true;
  if (val->ob_type == &PyFloat_Type)
  {
    d = PyFloat_AS_DOUBLE(val);
  }
  else if (!strict)
  {
#ifndef PY3K
    if (PyObject_TypeCheck(val, &PyInt_Type))
    {
      d = PyInt_AS_LONG(val);
    }
    else
#endif
        if (PyLong_Check(val))
    {
      d = static_cast<double>(PyLong_AsLongLong(val));
    }
    else if (val == Py_False)
    {
      d = 0;
    }
    else if (val == Py_True)
    {
      d = 1;
    }
    else
    {
      PyErr_Clear();
      // PyFloat_AsDouble will try conversion to a double if the object is not a float:
      d = PyFloat_AsDouble(val);
      if (PyErr_Occurred())
      {
        PyErr_Clear();
        ok = false;
      }
    }
  }
  else
  {
    ok = false;
  }
  return d;
}

template <typename Map> void PythonQtConv::pythonToMapVariant(PyObject *val, QVariant &result)
{
  if (PyMapping_Check(val))
  {
    Map map;
    PyObject *items = PyMapping_Items(val);
    if (items)
    {
      int count = PyList_Size(items);
      PyObject *value;
      PyObject *key;
      PyObject *tuple;
      for (int i = 0; i < count; i++)
      {
        tuple = PyList_GetItem(items, i);
        key = PyTuple_GetItem(tuple, 0);
        value = PyTuple_GetItem(tuple, 1);
        map.insert(PyObjGetString(key), PyObjToQVariant(value, -1));
      }
      Py_DECREF(items);
      result = map;
    }
  }
}

QVariant PythonQtConv::PyObjToQVariant(PyObject *val, int type)
{
  QVariant v;
  bool ok = true;

  if (type == -1
#if QT_VERSION >= 0x040800
      || type == QMetaType::QVariant
#endif
  )
  {
    // no special type requested
    if (PyBytes_Check(val))
    {
#ifdef PY3K
      // In Python 3, it is a ByteArray
      type = QMetaType::QByteArray;
#else
      // In Python 2, we need to use String, since it might be a string
      type = QVariant::String;
#endif
    }
    else if (PyUnicode_Check(val))
    {
      type = QMetaType::QString;
    }
    else if (val == Py_False || val == Py_True)
    {
      type = QMetaType::Bool;
#ifndef PY3K
    }
    else if (PyObject_TypeCheck(val, &PyInt_Type))
    {
      type = QVariant::Int;
#endif
    }
    else if (PyLong_Check(val))
    {
      // return int if the value fits into that range,
      // otherwise it would not be possible to get an int from Python 3
      qint64 d = PyLong_AsLongLong(val);
      if (d > std::numeric_limits<int>::max() || d < std::numeric_limits<int>::min())
      {
        type = QMetaType::LongLong;
      }
      else
      {
        type = QMetaType::Int;
      }
    }
    else if (PyFloat_Check(val))
    {
      type = QMetaType::Double;
    }
    else if (val == Py_None)
    {
      // none is invalid
      type = QMetaType::UnknownType;
    }
    else if (PyDict_Check(val))
    {
      type = QMetaType::QVariantMap;
    }
    else if (PyList_Check(val) || PyTuple_Check(val) || PySequence_Check(val))
    {
      type = QMetaType::QVariantList;
    }
    else
    {
      type = QMetaType::UnknownType;
    }
  }
  // special type request:
  switch (type)
  {
  case QMetaType::UnknownType:
    return v;
    break;
  case QMetaType::Int: {
    int d = PyObjGetInt(val, false, ok);
    if (ok)
      return QVariant(d);
  }
  break;
  case QMetaType::UInt: {
    int d = PyObjGetInt(val, false, ok);
    if (ok)
      v = QVariant((unsigned int)d);
  }
  break;
  case QMetaType::Bool: {
    int d = PyObjGetBool(val, false, ok);
    if (ok)
      v = QVariant((bool)(d != 0));
  }
  break;
  case QMetaType::Double: {
    double d = PyObjGetDouble(val, false, ok);
    if (ok)
      v = QVariant(d);
    break;
  }
  case QMetaType::Float: {
    float d = (float)PyObjGetDouble(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
    break;
  }
  case QMetaType::Long: {
    long d = (long)PyObjGetLongLong(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
    break;
  }
  case QMetaType::ULong: {
    unsigned long d = (unsigned long)PyObjGetLongLong(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
    break;
  }
  case QMetaType::LongLong: {
    qint64 d = PyObjGetLongLong(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
  }
  break;
  case QMetaType::ULongLong: {
    quint64 d = PyObjGetULongLong(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
  }
  break;
  case QMetaType::Short: {
    short d = (short)PyObjGetInt(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
    break;
  }
  case QMetaType::UShort: {
    unsigned short d = (unsigned short)PyObjGetInt(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
    break;
  }
  case QMetaType::Char: {
    char d = (char)PyObjGetInt(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
    break;
  }
  case QMetaType::UChar: {
    unsigned char d = (unsigned char)PyObjGetInt(val, false, ok);
    if (ok)
      v = QVariant::fromValue(d);
    break;
  }

  case QMetaType::QByteArray: {
    bool ok;
#ifdef PY3K
    v = QVariant(PyObjGetBytes(val, false, ok));
#else
    v = QVariant(PyObjGetString(val, false, ok));
#endif
  }
  case QMetaType::QString: {
    bool ok;
    v = QVariant(PyObjGetString(val, false, ok));
  }
  break;

  case QMetaType::QVariantMap:
    pythonToMapVariant<QVariantMap>(val, v);
    break;
  case QMetaType::QVariantHash:
    pythonToMapVariant<QVariantHash>(val, v);
    break;
  case QMetaType::QVariantList: {
    bool isListOrTuple = PyList_Check(val) || PyTuple_Check(val);
    if (isListOrTuple || PySequence_Check(val))
    {
      if (!isListOrTuple && _pythonSequenceToQVariantListCB)
      {
        // Only call this if we don't have a tuple or list.
        QVariant result = (*_pythonSequenceToQVariantListCB)(val);
        if (result.isValid())
        {
          return result;
        }
      }
      int count = PySequence_Size(val);
      if (count >= 0)
      {
        // only get items if size is valid (>= 0)
        QVariantList list;
        PyObject *value;
        for (int i = 0; i < count; i++)
        {
          value = PySequence_GetItem(val, i);
          list.append(PyObjToQVariant(value, -1));
          Py_XDECREF(value);
        }
        v = list;
      }
    }
  }
  break;
  case QMetaType::QStringList: {
    bool ok;
    QStringList l = PyObjToStringList(val, false, ok);
    if (ok)
    {
      v = l;
    }
  }
  break;

  default:
    if (type >= static_cast<int>(QMetaType::User))
    {
      // not an instance wrapper, but there might be other converters
      // Maybe we have a special converter that is registered for that type:
      PythonQtConvertPythonToMetaTypeCB *converter = _pythonToMetaTypeConverters.value(type);
      if (converter)
      {
        // allocate a default object of the needed type:
        v = QVariant(QMetaType(type), (const void *)NULL);
        // now call the converter, passing the internal object of the variant
        ok = (*converter)(val, (void *)v.constData(), type, true);
        if (!ok)
        {
          v = QVariant();
        }
      }
      else
      {
        v = QVariant();
      }
    }
  }
  return v;
}

PyObject *PythonQtConv::QStringToPyObject(const QString &str)
{
  if (str.isNull())
  {
    return PyString_FromString("");
  }
  else
  {
    return PyUnicode_DecodeUTF16((const char *)str.utf16(), str.length() * 2, NULL, NULL);
  }
}

PyObject *PythonQtConv::QStringListToPyObject(const QStringList &list)
{
  PyObject *result = PyTuple_New(list.count());
  int i = 0;
  QString str;
  Q_FOREACH (str, list)
  {
    PyTuple_SET_ITEM(result, i, PythonQtConv::QStringToPyObject(str));
    i++;
  }
  // why is the error state bad after this?
  PyErr_Clear();
  return result;
}

PyObject *PythonQtConv::QStringListToPyList(const QStringList &list)
{
  PyObject *result = PyList_New(list.count());
  int i = 0;
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
  {
    PyList_SET_ITEM(result, i, PythonQtConv::QStringToPyObject(*it));
    i++;
  }
  return result;
}

PyObject *PythonQtConv::QVariantToPyObject(const QVariant &v)
{
  if (!v.isValid())
  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  PyObject *obj = convertQtValueToPythonInternal(v.userType(), (void *)v.constData());
  return obj;
}

template <typename Map> PyObject *PythonQtConv::mapToPython(const Map &m)
{
  PyObject *result = PyDict_New();
  typename Map::const_iterator t = m.constBegin();
  PyObject *key;
  PyObject *val;
  for (; t != m.constEnd(); t++)
  {
    key = QStringToPyObject(t.key());
    val = QVariantToPyObject(t.value());
    PyDict_SetItem(result, key, val);
    Py_DECREF(key);
    Py_DECREF(val);
  }
  return result;
}

PyObject *PythonQtConv::QVariantMapToPyObject(const QVariantMap &m)
{
  return mapToPython<QVariantMap>(m);
}

PyObject *PythonQtConv::QVariantHashToPyObject(const QVariantHash &m)
{
  return mapToPython<QVariantHash>(m);
}

PyObject *PythonQtConv::QVariantListToPyObject(const QVariantList &l)
{
  PyObject *result = PyTuple_New(l.count());
  int i = 0;
  QVariant v;
  Q_FOREACH (v, l)
  {
    PyTuple_SET_ITEM(result, i, PythonQtConv::QVariantToPyObject(v));
    i++;
  }
  // why is the error state bad after this?
  PyErr_Clear();
  return result;
}

PyObject *PythonQtConv::convertFromStringRef(const void *inObject, int /*metaTypeId*/)
{
  return PythonQtConv::QStringToPyObject(((QStringView *)inObject)->toString());
}

bool PythonQtConv::isStringType(PyTypeObject *type)
{
#ifdef PY3K
  return type == &PyUnicode_Type;
#else
  return type == &PyUnicode_Type || type == &PyString_Type;
#endif
}
