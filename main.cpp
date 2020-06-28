#include <QApplication>
#include <QDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    // start event loop
    return a.exec();
}

