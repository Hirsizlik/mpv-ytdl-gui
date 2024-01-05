#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <backtrace.h>
#include <cxxabi.h>
#include <signal.h>
#include "mainform.h"

static backtrace_state *bt_state = nullptr;

static void
bt_error(void *, const char *msg, int errnum)
{
    fprintf(stderr, "backtrace error %d: %s\n", errnum, msg);
}

static int
bt_full(void *, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    if (!filename) {
        return 0;
    }
    int dstatus;
    const char *demangled_function = abi::__cxa_demangle(function, nullptr, nullptr, &dstatus);
    fprintf(stderr, "%#lx %s\n  %s:%d\n", pc, dstatus == 0 ? demangled_function : function, filename, lineno);
    return 0;
}

void
handler(int sig) noexcept
{
    fprintf(stderr, "Got Signal %d\n", sig);
    backtrace_full(bt_state, 1, bt_full, bt_error, nullptr);
    exit(1);
}

int
main(int argc, char **argv)
{
    bt_state = backtrace_create_state("mpv-ytdl-gui", 0, bt_error, nullptr);
    signal(SIGSEGV, handler);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("mpv-ytdl-gui");
    app.setApplicationName("mpv-ytdl-gui");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/mpvytdlgui/main.qml"_qs);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    MainForm mf;
    engine.rootContext()->setContextProperty("mf", &mf);
    engine.load(url);

    Py_Initialize();
    int code = app.exec();
    Py_Finalize();
    return code;
}
