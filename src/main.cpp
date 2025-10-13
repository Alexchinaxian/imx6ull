#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QCommandLineParser>
#include "driver_manager.h"

const uint8_t SoftwareVersion[3] = {0, 0, 0};       // 软件版本号

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qInfo("dsadsd");
    return app.exec();
}