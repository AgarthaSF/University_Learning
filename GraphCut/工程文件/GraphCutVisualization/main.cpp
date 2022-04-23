#include "GraphCutVisualization.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GraphCutVisualization w;
    w.show();
    return a.exec();
}
