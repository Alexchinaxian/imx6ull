#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QVariantMap>
#include "../driver/gpio_driver.h"

class LEDController : public QObject
{
    Q_OBJECT
};

#endif // LEDCONTROLLER_H