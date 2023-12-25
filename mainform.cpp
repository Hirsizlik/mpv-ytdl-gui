#include "mainform.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusVariant>
#include <cstring>
#include <exception>
#include <iostream>
#include <unistd.h>
#include "formats.h"
#include "ytdlp.h"

MainForm::MainForm(QObject *parent): QObject(parent)
{
}

class WarnOnExit {
  public:
    explicit WarnOnExit(const QDBusConnection &connection): connection(connection)
    {
    }
    ~WarnOnExit()
    {
        if (connection.lastError().isValid()) {
            qWarning("%s", qPrintable(connection.lastError().message()));
        }
    }

  private:
    const QDBusConnection &connection;
};

struct SecretStructure {
    QDBusObjectPath secretPath;
    QByteArray password;
    QString passwordMime;
};

const QDBusArgument &
operator>>(const QDBusArgument &argument, SecretStructure &secret)
{
    argument.beginStructure();
    argument >> secret.secretPath;
    argument.beginArray();
    argument.endArray();
    argument >> secret.password >> secret.passwordMime;
    argument.endStructure();
    return argument;
}

static QList<QDBusObjectPath>
fillPaths(const QVariant &var)
{
    const QDBusArgument &arg = var.value<QDBusArgument>();
    QList<QDBusObjectPath> paths;

    arg.beginArray();
    while (!arg.atEnd()) {
        QDBusObjectPath current;
        arg >> current;
        paths.emplaceBack(std::move(current));
    }
    arg.endArray();
    return paths;
}

QString
MainForm::getSecret(const QDBusObjectPath &pathToUnlockedPassword)
{
    QDBusInterface ifaceService("org.freedesktop.secrets", "/org/freedesktop/secrets",
                                "org.freedesktop.Secret.Service");
    QDBusMessage reply = ifaceService.call("OpenSession", QString("plain"), QVariant::fromValue(QDBusVariant("")));
    auto arguments = reply.arguments();
    if (arguments.size() != 2) {
        emit passwordError();
        qWarning() << "reply to OpenSession invalid";
        return "";
    }
    // first argument only relevant for encrypted communication, not done here.
    QDBusMessage secretReply = ifaceService.call(
        "GetSecrets", QVariant::fromValue(QList<QDBusObjectPath>{pathToUnlockedPassword}), arguments.at(1));
    const QDBusArgument &arg = secretReply.arguments()[0].value<QDBusArgument>();
    arg.beginMap();
    // expecting only one map entry, ignore the rest
    arg.beginMapEntry();
    QDBusObjectPath key;
    SecretStructure value;
    arg >> key >> value;
    arg.endMapEntry();
    arg.endMap();
    return value.password;
}

void
MainForm::passwordPromptCompleted(const bool dismissed, const QDBusVariant &value)
{
    if (dismissed) {
        emit passwordError();
        qWarning("Password prompt was dismissed");
        return;
    }
    auto unlockedPaths = fillPaths(value.variant());
    assert(!unlockedPaths.isEmpty());
    password = getSecret(unlockedPaths.at(0));
}

void
MainForm::loadPassword(const QString &p1, const QString &p2)
{
    auto connection = QDBusConnection::sessionBus();
    WarnOnExit wox(connection);
    if (!connection.isConnected()) {
        emit passwordError();
        qWarning("DBus is not running");
        return;
    }
    QDBusInterface ifaceService("org.freedesktop.secrets", "/org/freedesktop/secrets",
                                "org.freedesktop.Secret.Service");
    if (!ifaceService.isValid()) {
        emit passwordError();
        qWarning("interface invalid");
        return;
    }

    QMap<QString, QString> map;
    map.insert(p1, p2);
    QDBusArgument arg1;
    arg1 << map;

    QDBusMessage searchItemReply = ifaceService.call("SearchItems", QVariant::fromValue(arg1));
    auto reply_arguments = searchItemReply.arguments();
    if (reply_arguments.size() != 2) {
        emit passwordError();
        qWarning() << "reply to SearchItems invalid";
        return;
    }

    QList<QDBusObjectPath> unlockedPaths = fillPaths(reply_arguments.at(0));
    QList<QDBusObjectPath> lockedPaths = fillPaths(reply_arguments.at(1));

    if (unlockedPaths.isEmpty() && lockedPaths.isEmpty()) {
        emit passwordError();
        qWarning("No password found");
        return;
    }

    if (!unlockedPaths.isEmpty()) {
        password = getSecret(unlockedPaths.at(0));
        return;
    }

    QDBusMessage unlockReply =
        ifaceService.call("Unlock", QVariant::fromValue(QList<QDBusObjectPath>{lockedPaths.at(0)}));
    auto unlockReplyArgs = unlockReply.arguments();
    if (unlockReplyArgs.size() != 2) {
        emit passwordError();
        qWarning() << "reply to Unlock invalid";
        return;
    }

    QList<QDBusObjectPath> unlockedWithoutPrompt = fillPaths(unlockReplyArgs.at(0));
    if (!unlockedWithoutPrompt.isEmpty()) {
        password = getSecret(unlockedWithoutPrompt.at(0));
        return;
    }

    QDBusObjectPath prompt = unlockReplyArgs.at(1).value<QDBusObjectPath>();

    if (connection.connect("org.freedesktop.secrets", prompt.path(), "org.freedesktop.Secret.Prompt", "Completed", this,
                           SLOT(passwordPromptCompleted(const bool, const QDBusVariant &)))) {
        QDBusInterface ifacePrompt("org.freedesktop.secrets", prompt.path(), "org.freedesktop.Secret.Prompt");
        ifacePrompt.call("Prompt", QString("mpv-ytdl-gui"));
    }
}

FormatData
map(const yt::YtdlpData &data)
{
    QString vcodec;
    if (data.vcodec() != "none") {
        vcodec = QString::fromStdString(data.vcodec());
    }
    QString acodec;
    if (data.acodec() != "none") {
        acodec = QString::fromStdString(data.acodec());
    }
    return FormatData(QString::fromStdString(data.id()), QString::fromStdString(data.name()),
                      QString::fromStdString(data.resolution()), vcodec, acodec, data.fps(), data.filesize(),
                      data.filesize_estimate(), data.tbr());
}

QList<FormatData>
convertFromYtdlp(const std::vector<yt::YtdlpData> &data)
{
    QList<FormatData> list;
    for (const yt::YtdlpData &y : data) {
        list.emplaceBack(map(y));
    }
    return list;
}

void
MainForm::loadFormats(const QString &username, const QString &url, const QString &cookiesFromBrowser)
{
    if (url.isEmpty()) {
        emit ytdlpError();
        return;
    }

    try {
        auto data = yt::loadFormats(username.toUtf8(), password.toUtf8(), url.toUtf8(), cookiesFromBrowser.toUtf8());
        formatsModel.setFormatList(url, convertFromYtdlp(data));
    } catch (const std::runtime_error &) {
        emit ytdlpError();
    }
}

void
MainForm::start(const QString &username, const QString &sublang, const QString &cookiesBrowser)
{
    std::vector<FormatData> selectedFormats;
    for (qsizetype i = 0; i < formatsModel.rowCount(); ++i) {
        if (formatsModel[i].selected()) {
            selectedFormats.emplace_back(formatsModel[i]);
        }
    }

    if (selectedFormats.empty()) {
        emit noSelectionError();
        return;
    }

    QString formats = std::transform_reduce(
        selectedFormats.begin(), selectedFormats.end(), QString(),
        [](const QString &f1, const QString &f2) -> QString { return f1.isEmpty() ? f2 : f1 + "," + f2; },
        [](const FormatData &f) -> QString { return f.id(); });

    if (0 == fork()) {
        QList<std::pair<QString, QString>> ytdlRawOptionList;
        ytdlRawOptionList.emplaceBack("all-subs", "");
        ytdlRawOptionList.emplaceBack("cookies-from-browser", cookiesBrowser);

        if (username != "" && password != "") {
            ytdlRawOptionList.emplaceBack("username", username);
            ytdlRawOptionList.emplaceBack("password", password);
        }

        QString ytdl_raw_options = "--ytdl-raw-options=";
        for (const auto &[k, v] : ytdlRawOptionList) {
            ytdl_raw_options += k + "=" + v + ",";
        }
        ytdl_raw_options.removeLast();

        auto ytdl_format = QString(R"(--ytdl-format=%1)").arg(formats);
        QString slang_param;
        if (!sublang.isEmpty()) {
            slang_param = "--slang=" + sublang;
        } else {
            slang_param = "";
        }

        execlp("mpv", "mpv", qPrintable(ytdl_raw_options), qPrintable(ytdl_format), qPrintable(slang_param),
               qPrintable(formatsModel.searchUrl()), "--player-operation-mode=pseudo-gui", (char *)NULL);
    }
}
