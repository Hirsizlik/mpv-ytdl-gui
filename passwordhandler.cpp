#include "passwordhandler.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusVariant>

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

void
PasswordHandler::getSecret(const QDBusObjectPath &pathToUnlockedPassword)
{
    QDBusInterface ifaceService("org.freedesktop.secrets", "/org/freedesktop/secrets",
                                "org.freedesktop.Secret.Service");
    QDBusMessage reply = ifaceService.call("OpenSession", QString("plain"), QVariant::fromValue(QDBusVariant("")));
    auto arguments = reply.arguments();
    if (arguments.size() != 2) {
        qWarning() << "reply to OpenSession invalid";
        emit passwordError();
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
    emit passwordLoaded(value.password);
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

void
PasswordHandler::passwordPromptCompleted(bool dismissed, const QDBusVariant &value) {
    if (dismissed) {
        emit passwordError();
        qWarning("Password prompt was dismissed");
        return;
    }
    auto unlockedPaths = fillPaths(value.variant());
    assert(!unlockedPaths.isEmpty());
    getSecret(unlockedPaths.at(0));
}

void
PasswordHandler::loadPassword(const QString &attribute, const QString &value)
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
    map.insert(attribute, value);
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
        getSecret(unlockedPaths.at(0));
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
        getSecret(unlockedWithoutPrompt.at(0));
        return;
    }

    QDBusObjectPath prompt = unlockReplyArgs.at(1).value<QDBusObjectPath>();

    if (connection.connect("org.freedesktop.secrets", prompt.path(), "org.freedesktop.Secret.Prompt", "Completed", this,
                           SLOT(passwordPromptCompleted(bool, const QDBusVariant &)))) {
        QDBusInterface ifacePrompt("org.freedesktop.secrets", prompt.path(), "org.freedesktop.Secret.Prompt");
        ifacePrompt.call("Prompt", QString("mpv-ytdl-gui"));
    }
}
