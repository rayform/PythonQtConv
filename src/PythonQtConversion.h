#ifndef _PYTHONQTCONVERSION_H
#define _PYTHONQTCONVERSION_H

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
// \file    PythonQtConversion.h
// \author  Florian Link
// \author  Last changed by $Author: florian $
// \date    2006-05
*/
//----------------------------------------------------------------------------------

#include <Python.h>

#include <QList>
#include <QVariant>
#include <iostream>
#include <vector>

typedef PyObject *PythonQtConvertMetaTypeToPythonCB(const void *inObject, int metaTypeId);
typedef bool PythonQtConvertPythonToMetaTypeCB(PyObject *inObject, void *outObject, int metaTypeId,
                                               bool strict);
typedef QVariant PythonQtConvertPythonSequenceToQVariantListCB(PyObject *inObject);

#define PythonQtRegisterListTemplateConverter(type, innertype)                                     \
  {                                                                                                \
    int typeId = qRegisterMetaType<type<innertype>>(#type "<" #innertype ">");                     \
    PythonQtConv::registerPythonToMetaTypeConverter(                                               \
        typeId, PythonQtConvertPythonListToListOfValueType<type<innertype>, innertype>);           \
    PythonQtConv::registerMetaTypeToPythonConverter(                                               \
        typeId, PythonQtConvertListOfValueTypeToPythonList<type<innertype>, innertype>);           \
  }

#define PythonQtRegisterListTemplateConverterForKnownClass(type, innertype)                        \
  {                                                                                                \
    int typeId = qRegisterMetaType<type<innertype>>(#type "<" #innertype ">");                     \
    PythonQtConv::registerPythonToMetaTypeConverter(                                               \
        typeId, PythonQtConvertPythonListToListOfKnownClass<type<innertype>, innertype>);          \
    PythonQtConv::registerMetaTypeToPythonConverter(                                               \
        typeId, PythonQtConvertListOfKnownClassToPythonList<type<innertype>, innertype>);          \
  }

#define PythonQtRegisterQPairConverter(type1, type2)                                               \
  {                                                                                                \
    int typeId = qRegisterMetaType<QPair<type1, type2>>("QPair<" #type1 "," #type2 ">");           \
    PythonQtConv::registerPythonToMetaTypeConverter(typeId,                                        \
                                                    PythonQtConvertPythonToPair<type1, type2>);    \
    PythonQtConv::registerMetaTypeToPythonConverter(typeId,                                        \
                                                    PythonQtConvertPairToPython<type1, type2>);    \
  }

#define PythonQtRegisterIntegerMapConverter(type, innertype)                                       \
  {                                                                                                \
    int typeId = qRegisterMetaType<type<int, innertype>>(#type "<int, " #innertype ">");           \
    PythonQtConv::registerPythonToMetaTypeConverter(                                               \
        typeId, PythonQtConvertPythonToIntegerMap<type<int, innertype>, innertype>);               \
    PythonQtConv::registerMetaTypeToPythonConverter(                                               \
        typeId, PythonQtConvertIntegerMapToPython<type<int, innertype>, innertype>);               \
  }

#define PythonQtRegisterListTemplateQPairConverter(listtype, type1, type2)                         \
  {                                                                                                \
    qRegisterMetaType<QPair<type1, type2>>("QPair<" #type1 "," #type2 ">");                        \
    int typeId = qRegisterMetaType<listtype<QPair<type1, type2>>>(#listtype "<QPair<" #type1       \
                                                                            "," #type2 ">>");      \
    PythonQtConv::registerPythonToMetaTypeConverter(                                               \
        typeId,                                                                                    \
        PythonQtConvertPythonListToListOfPair<listtype<QPair<type1, type2>>, type1, type2>);       \
    PythonQtConv::registerMetaTypeToPythonConverter(                                               \
        typeId,                                                                                    \
        PythonQtConvertListOfPairToPythonList<listtype<QPair<type1, type2>>, type1, type2>);       \
  }

#define PythonQtRegisterToolClassesTemplateConverter(innertype)                                    \
  PythonQtRegisterListTemplateConverter(QList, innertype);                                         \
  PythonQtRegisterListTemplateConverter(QVector, innertype);                                       \
  PythonQtRegisterListTemplateConverter(std::vector, innertype);

#define PythonQtRegisterToolClassesTemplateConverterForKnownClass(innertype)                       \
  PythonQtRegisterListTemplateConverterForKnownClass(QList, innertype);                            \
  PythonQtRegisterListTemplateConverterForKnownClass(QVector, innertype);                          \
  PythonQtRegisterListTemplateConverterForKnownClass(std::vector, innertype);

//! a static class that offers methods for type conversion
class PythonQtConv
{

public:
  //! get a ref counted True or False Python object
  static PyObject *GetPyBool(bool val);

  //! converts QString to Python string (unicode!)
  static PyObject *QStringToPyObject(const QString &str);

  //! converts QStringList to Python tuple
  static PyObject *QStringListToPyObject(const QStringList &list);

  //! converts QStringList to Python list
  static PyObject *QStringListToPyList(const QStringList &list);

  //! get string representation of py object
  static QString PyObjGetRepresentation(PyObject *val);

  //! get string value from py object
  static QString PyObjGetString(PyObject *val)
  {
    bool ok;
    QString s = PyObjGetString(val, false, ok);
    return s;
  }
  //! get string value from py object
  static QString PyObjGetString(PyObject *val, bool strict, bool &ok);
  //! get bytes from py object
  static QByteArray PyObjGetBytes(PyObject *val, bool strict, bool &ok);
  //! get int from py object
  static int PyObjGetInt(PyObject *val, bool strict, bool &ok);
  //! get int64 from py object
  static qint64 PyObjGetLongLong(PyObject *val, bool strict, bool &ok);
  //! get int64 from py object
  static quint64 PyObjGetULongLong(PyObject *val, bool strict, bool &ok);
  //! get double from py object
  static double PyObjGetDouble(PyObject *val, bool strict, bool &ok);
  //! get bool from py object
  static bool PyObjGetBool(PyObject *val, bool strict, bool &ok);

  //! create a string list from python sequence
  static QStringList PyObjToStringList(PyObject *val, bool strict, bool &ok);

  //! convert python object to qvariant, if type is given it will try to create a qvariant of that
  //! type, otherwise it will guess from the python type
  static QVariant PyObjToQVariant(PyObject *val, int type = -1);

  //! convert QVariant from PyObject
  static PyObject *QVariantToPyObject(const QVariant &v);

  static PyObject *QVariantHashToPyObject(const QVariantHash &m);
  static PyObject *QVariantMapToPyObject(const QVariantMap &m);
  static PyObject *QVariantListToPyObject(const QVariantList &l);

  //! converts the Qt parameter given in \c data, interpreting it as a \c type registered
  //! qvariant/meta type, into a Python object,
  static PyObject *convertQtValueToPythonInternal(int type, const void *data);

  static PyObject *convertFromStringRef(const void *inObject, int /*metaTypeId*/);

  //! Returns if the given object is a string (or unicode string)
  static bool isStringType(PyTypeObject *type);

protected:
  static QHash<int, PythonQtConvertMetaTypeToPythonCB *> _metaTypeToPythonConverters;
  static QHash<int, PythonQtConvertPythonToMetaTypeCB *> _pythonToMetaTypeConverters;
  static PythonQtConvertPythonSequenceToQVariantListCB *_pythonSequenceToQVariantListCB;

  //! helper template method for conversion from Python to QVariantMap/Hash
  template <typename Map> static void pythonToMapVariant(PyObject *val, QVariant &result);
  //! helper template function for QVariantMapToPyObject/QVariantHashToPyObject
  template <typename Map> static PyObject *mapToPython(const Map &m);
};

//--------------------------------------------------------------------------------------------------------------------

#endif
