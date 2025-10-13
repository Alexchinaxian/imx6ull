/****************************************************************************
** Meta object code from reading C++ file 'gpio_driver.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../inc/driver/gpio_driver.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gpio_driver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GPIODriver_t {
    QByteArrayData data[27];
    char stringdata0[313];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GPIODriver_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GPIODriver_t qt_meta_stringdata_GPIODriver = {
    {
QT_MOC_LITERAL(0, 0, 10), // "GPIODriver"
QT_MOC_LITERAL(1, 11, 11), // "initialized"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 7), // "success"
QT_MOC_LITERAL(4, 32, 7), // "started"
QT_MOC_LITERAL(5, 40, 7), // "stopped"
QT_MOC_LITERAL(6, 48, 13), // "errorOccurred"
QT_MOC_LITERAL(7, 62, 5), // "error"
QT_MOC_LITERAL(8, 68, 15), // "pinStateChanged"
QT_MOC_LITERAL(9, 84, 3), // "pin"
QT_MOC_LITERAL(10, 88, 5), // "state"
QT_MOC_LITERAL(11, 94, 15), // "pinEdgeDetected"
QT_MOC_LITERAL(12, 110, 10), // "risingEdge"
QT_MOC_LITERAL(13, 121, 24), // "setPinDirectionRequested"
QT_MOC_LITERAL(14, 146, 8), // "isOutput"
QT_MOC_LITERAL(15, 155, 17), // "writePinRequested"
QT_MOC_LITERAL(16, 173, 16), // "readPinRequested"
QT_MOC_LITERAL(17, 190, 13), // "enablePolling"
QT_MOC_LITERAL(18, 204, 6), // "enable"
QT_MOC_LITERAL(19, 211, 10), // "intervalMs"
QT_MOC_LITERAL(20, 222, 14), // "setPollingPins"
QT_MOC_LITERAL(21, 237, 10), // "QList<int>"
QT_MOC_LITERAL(22, 248, 4), // "pins"
QT_MOC_LITERAL(23, 253, 8), // "pollPins"
QT_MOC_LITERAL(24, 262, 21), // "handleSetPinDirection"
QT_MOC_LITERAL(25, 284, 14), // "handleWritePin"
QT_MOC_LITERAL(26, 299, 13) // "handleReadPin"

    },
    "GPIODriver\0initialized\0\0success\0started\0"
    "stopped\0errorOccurred\0error\0pinStateChanged\0"
    "pin\0state\0pinEdgeDetected\0risingEdge\0"
    "setPinDirectionRequested\0isOutput\0"
    "writePinRequested\0readPinRequested\0"
    "enablePolling\0enable\0intervalMs\0"
    "setPollingPins\0QList<int>\0pins\0pollPins\0"
    "handleSetPinDirection\0handleWritePin\0"
    "handleReadPin"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GPIODriver[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   94,    2, 0x06 /* Public */,
       4,    1,   97,    2, 0x06 /* Public */,
       5,    0,  100,    2, 0x06 /* Public */,
       6,    1,  101,    2, 0x06 /* Public */,
       8,    2,  104,    2, 0x06 /* Public */,
      11,    2,  109,    2, 0x06 /* Public */,
      13,    2,  114,    2, 0x06 /* Public */,
      15,    2,  119,    2, 0x06 /* Public */,
      16,    1,  124,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      17,    2,  127,    2, 0x0a /* Public */,
      17,    1,  132,    2, 0x2a /* Public | MethodCloned */,
      20,    1,  135,    2, 0x0a /* Public */,
      23,    0,  138,    2, 0x08 /* Private */,
      24,    2,  139,    2, 0x08 /* Private */,
      25,    2,  144,    2, 0x08 /* Private */,
      26,    1,  149,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    9,   10,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    9,   12,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    9,   14,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    9,   10,
    QMetaType::Void, QMetaType::Int,    9,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::Int,   18,   19,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, 0x80000000 | 21,   22,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    9,   14,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    9,   10,
    QMetaType::Void, QMetaType::Int,    9,

       0        // eod
};

void GPIODriver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GPIODriver *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->initialized((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->started((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->stopped(); break;
        case 3: _t->errorOccurred((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->pinStateChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->pinEdgeDetected((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->setPinDirectionRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 7: _t->writePinRequested((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 8: _t->readPinRequested((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->enablePolling((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 10: _t->enablePolling((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->setPollingPins((*reinterpret_cast< const QList<int>(*)>(_a[1]))); break;
        case 12: _t->pollPins(); break;
        case 13: _t->handleSetPinDirection((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 14: _t->handleWritePin((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 15: _t->handleReadPin((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 11:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QList<int> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GPIODriver::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::initialized)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::started)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::stopped)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::errorOccurred)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::pinStateChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::pinEdgeDetected)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::setPinDirectionRequested)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::writePinRequested)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (GPIODriver::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GPIODriver::readPinRequested)) {
                *result = 8;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GPIODriver::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_GPIODriver.data,
    qt_meta_data_GPIODriver,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GPIODriver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GPIODriver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GPIODriver.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GPIODriver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void GPIODriver::initialized(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GPIODriver::started(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GPIODriver::stopped()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GPIODriver::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void GPIODriver::pinStateChanged(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void GPIODriver::pinEdgeDetected(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void GPIODriver::setPinDirectionRequested(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void GPIODriver::writePinRequested(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void GPIODriver::readPinRequested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
