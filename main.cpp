#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 整体风格稍微统一一下
    a.setStyleSheet(
        "QMainWindow { background:#f3f6fb; }"
        "QStatusBar { background:#e2e8f5; }"
        );

    MainWindow w;
    w.show();
    return a.exec();
}
