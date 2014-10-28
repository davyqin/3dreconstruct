#include <QApplication>

#include "gui/Controller.h"
//#include "cuda/cuda_info.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //cudaAvaliable();

    Controller controller;
    controller.activate();

    return app.exec();
}
