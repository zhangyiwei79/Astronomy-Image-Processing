/****************************************************************************
** Meta object code from reading C++ file 'DeconvolutionDlg.h'
**
** Created: Thu Jul 7 15:21:49 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DeconvolutionDlg.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DeconvolutionDlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DeconvolutionDlg[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   18,   17,   17, 0x08,
      47,   18,   17,   17, 0x08,
      78,   73,   17,   17, 0x08,
      99,   73,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DeconvolutionDlg[] = {
    "DeconvolutionDlg\0\0nIndex\0setCurrentFilter(int)\0"
    "setCurrentWindowSize(int)\0dVal\0"
    "setGamaValue(double)\0setSigmaValue(double)\0"
};

const QMetaObject DeconvolutionDlg::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DeconvolutionDlg,
      qt_meta_data_DeconvolutionDlg, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DeconvolutionDlg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DeconvolutionDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DeconvolutionDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DeconvolutionDlg))
        return static_cast<void*>(const_cast< DeconvolutionDlg*>(this));
    return QDialog::qt_metacast(_clname);
}

int DeconvolutionDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setCurrentFilter((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: setCurrentWindowSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: setGamaValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: setSigmaValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
