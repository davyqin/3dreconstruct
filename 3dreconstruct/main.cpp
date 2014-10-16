#include <QApplication>

#include "gui/Controller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Controller controller;
    controller.activate();

    return app.exec();
}
