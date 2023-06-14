#include "TRSVote.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TRSVote w;
    w.show();
    return a.exec();
}
