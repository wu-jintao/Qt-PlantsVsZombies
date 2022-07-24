//
// Created by sun on 8/25/16.
//
//              QT匹配机制：信号槽
//              *******F1帮助文档YYDS*************
//完善于2022.6.30 22
#include <QtCore>
#include <QtWidgets>
#include "MainView.h"
#include "ImageManager.h"

int main(int argc, char * *argv)
{
    /*
    //QApplication管理GUI程序的控制流和主要设置。
    //QApplication包含窗口系统和其他来源处理过和发送过的主事件循环。
    //它也处理应用程序的初始化和收尾工作，并提供对话管理。
    //QApplication可以对系统和应用的大部分设置项进行设置。
    */
    QApplication app(argc, argv);

    // For QSettings
    /*
    Qt库针对不同的应用程序类型，提供了不同的类来表示，
    QCoreApplication表示Qt控制台程序，
    QApplication 和 QGuiApplication 表示GUI程序。
    关系为QCoreApplication 继承自最顶层的QObject，
    QGuiApplication 又继承自QCoreApplication，
    QApplication又继承自QGuiApplication。
    */
    QCoreApplication::setOrganizationName("Sun Ziping");
    QCoreApplication::setOrganizationDomain("sunziping.com");
    QCoreApplication::setApplicationName("Plants vs Zombies");

    // For Translators
    /*
    QTranslator类就是使用Qt的翻译文件，可以对界面中菜单栏、label、
    对话框等进行翻译。使用的前提是，开发者要自己创建翻译文 。
    */
    QTranslator appTranslator;

    // TODO: change translation back after debugging
    appTranslator.load(QString(":/translations/main.%1.qm").arg(QLocale::system().name()));
    //appTranslator.load(QString(":/translations/main.%1.qm").arg("zh_CN"));
    app.installTranslator(&appTranslator);

    InitImageManager();
    qsrand((uint) QTime::currentTime().msec());
    MainWindow mainWindow;
    gMainView->switchToMenuScene();
    mainWindow.show();
    int res = app.exec();
    DestoryImageManager();
    return res;
}


