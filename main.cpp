#include "mainwindow.h"
#include "aboutdialog.h"
#include "selectmergemapdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

//    AboutDialog about;
//    about.show();

//    SelectMergeMapDialog select("111");
//    select.show();

    return a.exec();
}
