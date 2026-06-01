#include "mainwindow.h"// 引入主窗口类的头文件
#include <QApplication>
#include <QDir>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);// 创建Qt应用程序对象，并传入命令行参数
    MainWindow w;// 创建主窗口对象
    w.showMaximized();// 最大化显示主窗口
    if (argc > 1) {
        const QString filePath = QDir::fromNativeSeparators(QString::fromLocal8Bit(argv[1]));
        QTimer::singleShot(600, &w, [&w, filePath]() {
            w.OpenFileFromPath(filePath);
        });
    }

    return a.exec();// 进入Qt应用程序的事件循环，等待事件的发生并处理，直到应用程序终止
}
