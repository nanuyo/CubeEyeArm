#include "cubeeye.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CubeEye w;
    w.show();

    return a.exec();
}
