/****************************************************************************
** Meta object code from reading C++ file 'driver_manager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../inc/driver/driver_manager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'driver_manager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DriverManager_t {
    QByteArrayData data[19];
    char stringdata0[244];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DriverManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DriverManager_t qt_meta_stringdata_DriverManager = {
    {
QT_MOC_LITERAL(0, 0, 13), // "DriverManager"
QT_MOC_LITERAL(1, 14, 17), // "driverInitialized"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 10), // "DriverType"
QT_MOC_LITERAL(4, 44, 4), // "type"
QT_MOC_LITERAL(5, 49, 13), // "driverStarted"
QT_MOC_LITERAL(6, 63, 13), // "driverStopped"
QT_MOC_LITERAL(7, 77, 11), // "driverError"
QT_MOC_LITERAL(8, 89, 5), // "error"
QT_MOC_LITERAL(9, 95, 15), // "allDriversReady"
QT_MOC_LITERAL(10, 111, 16), // "gpioDataReceived"
QT_MOC_LITERAL(11, 128, 3), // "pin"
QT_MOC_LITERAL(12, 132, 5), // "state"
QT_MOC_LITERAL(13, 138, 18), // "serialDataReceived"
QT_MOC_LITERAL(14, 157, 4), // "data"
QT_MOC_LITERAL(15, 162, 23), // "handleDriverInitialized"
QT_MOC_LITERAL(16, 186, 19), // "handleDriverStarted"
QT_MOC_LITERAL(17, 206, 19), // "handleDriverStopped"
QT_MOC_LITERAL(18, 226, 17) // "handleDriverError"

    },
    "DriverManager\0driverInitialized\0\0"
    "DriverType\0type\0driverStarted\0"
    "driverStopped\0driverError\0error\0"
    "allDriversReady\0gpioDataReceived\0pin\0"
    "state\0serialDataReceived\0data\0"
    "handleDriverInitialized\0handleDriverStarted\0"
    "handleDriverStopped\0handleDriverError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DriverManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       5,    1,   72,    2, 0x06 /* Public */,
       6,    1,   75,    2, 0x06 /* Public */,
       7,    2,   78,    2, 0x06 /* Public */,
       9,    0,   83,    2, 0x06 /* Public */,
      10,    2,   84,    2, 0x06 /* Public */,
      13,    1,   89,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      15,    1,   92,    2, 0x08 /* Private */,
      16,    1,   95,    2, 0x08 /* Private */,
      17,    1,   98,    2, 0x08 /* Private */,
      18,    2,  101,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   11,   12,
    QMetaType::Void, QMetaType::QByteArray,   14,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    8,

       0        // eod
};

void DriverManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DriverManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->driverInitialized((*reinterpret_cast< DriverType(*)>(_a[1]))); break;
        case 1: _t->driverStarted((*reinterpret_cast< DriverType(*)>(_a[1]))); break;
        case 2: _t->driverStopped((*reinterpret_cast< DriverType(*)>(_a[1]))); break;
        case 3: _t->driverError((*reinterpret_cast< DriverType(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 4: _t->allDriversReady(); break;
        case 5: _t->gpioDataReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->serialDataReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 7: _t->handleDriverInitialized((*reinterpret_cast< DriverType(*)>(_a[1]))); break;
        case 8: _t->handleDriverStarted((*reinterpret_cast< DriverType(*)>(_a[1]))); break;
        case 9: _t->handleDriverStopped((*reinterpret_cast< DriverType(*)>(_a[1]))); break;
        case 10: _t->handleDriverError((*reinterpret_cast< DriverType(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DriverManager::*)(DriverType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DriverManager::driverInitialized)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DriverManager::*)(DriverType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DriverManager::driverStarted)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DriverManager::*)(DriverType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DriverManager::driverStopped)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DriverManager::*)(DriverType , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DriverManager::driverError)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DriverManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DriverManager::allDriversReady)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DriverManager::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DriverManager::gpioDataReceived)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DriverManager::*)(const QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DriverManager::serialDataReceived)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DriverManager::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_DriverManager.data,
    qt_meta_data_DriverManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DriverManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DriverManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DriverManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DriverManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void DriverManager::driverInitialized(DriverType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DriverManager::driverStarted(DriverType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DriverManager::driverStopped(DriverType _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DriverManager::driverError(DriverType _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DriverManager::allDriversReady()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void DriverManager::gpioDataReceived(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void DriverManager::serialDataReceived(const QByteArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
