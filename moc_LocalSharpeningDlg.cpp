/****************************************************************************
** Meta object code from reading C++ file 'LocalSharpeningDlg.h'
**
** Created: Wed Jun 15 15:02:05 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "LocalSharpeningDlg.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LocalSharpeningDlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_LocalSharpeningDlg[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   20,   19,   19, 0x08,
      49,   20,   19,   19, 0x08,
      80,   75,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_LocalSharpeningDlg[] = {
    "LocalSharpeningDlg\0\0nIndex\0"
    "setCurrentFilter(int)\0setCurrentWindowSize(int)\0"
    "dVal\0setContrastValue(double)\0"
};

const QMetaObject LocalSharpeningDlg::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_LocalSharpeningDlg,
      qt_meta_data_LocalSharpeningDlg, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LocalSharpeningDlg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LocalSharpeningDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LocalSharpeningDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LocalSharpeningDlg))
        return static_cast<void*>(const_cast< LocalSharpeningDlg*>(this));
    return QDialog::qt_metacast(_clname);
}

int LocalSharpeningDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setCurrentFilter((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: setCurrentWindowSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: setContrastValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
