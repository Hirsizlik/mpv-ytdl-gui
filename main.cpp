#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <signal.h>
#include <stacktrace>
#include "mainform.h"

void
handler(int sig) noexcept
{
    fprintf(stderr, "Got Signal %d\n", sig);
    std::string st = std::to_string(std::stacktrace::current());
    fputs(st.data(), stderr);
    exit(1);
}

int
main(int argc, char **argv)
{
    signal(SIGSEGV, handler);
    signal(SIGABRT, handler);
    signal(SIGFPE, handler);
    signal(SIGILL, handler);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("mpv-ytdl-gui");
    app.setApplicationName("mpv-ytdl-gui");
    MainForm mf;
    QQmlApplicationEngine engine;
    const QUrl url("qrc:/qt/qml/mpvytdlgui/main.qml");
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.rootContext()->setContextProperty("mf", &mf);
    engine.load(url);

    return app.exec();
}
