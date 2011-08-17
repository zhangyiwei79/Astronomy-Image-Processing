/****************************************************************************
** Meta object code from reading C++ file 'WaveletKSigmaDlg.h'
**
** Created: Wed May 18 12:07:13 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "WaveletKSigmaDlg.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WaveletKSigmaDlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WaveletKSigmaDlg[] = {

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
      18,   17,   17,   17, 0x08,
      37,   17,   17,   17, 0x08,
      52,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WaveletKSigmaDlg[] = {
    "WaveletKSigmaDlg\0\0setThresholdMode()\0"
    "setThreshold()\0setCurrentLevel(int)\0"
};

const QMetaObject WaveletKSigmaDlg::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_WaveletKSigmaDlg,
      qt_meta_data_WaveletKSigmaDlg, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WaveletKSigmaDlg::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WaveletKSigmaDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WaveletKSigmaDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WaveletKSigmaDlg))
        return static_cast<void*>(const_cast< WaveletKSigmaDlg*>(this));
    return QDialog::qt_metacast(_clname);
}

int WaveletKSigmaDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setThresholdMode(); break;
        case 1: setThreshold(); break;
        case 2: setCurrentLevel((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
