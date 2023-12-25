#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "mainform.h"

int main(int argc, char** argv) {

    QGuiApplication app(argc, argv);
    app.setOrganizationName("mpv-ytdl-gui");
    app.setApplicationName("mpv-ytdl-gui");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/mpvytdlgui/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    MainForm mf;
    engine.rootContext()->setContextProperty("mf", &mf);
    engine.load(url);

    Py_Initialize();
    int code = app.exec();
    Py_Finalize();
    return code;
}
