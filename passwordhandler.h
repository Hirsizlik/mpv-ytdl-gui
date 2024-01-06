#ifndef MPV_YTDL_GUI_PASSWORDHANDLER_H
#define MPV_YTDL_GUI_PASSWORDHANDLER_H

#include <QObject>

class QDBusVariant;
class QDBusObjectPath;

class PasswordHandler : public QObject {
    Q_OBJECT

  public:
    void loadPassword(const QString &attribute, const QString &value);

  private:
    void getSecret(const QDBusObjectPath &pathToUnlockedPassword);

  private slots:
    void passwordPromptCompleted(bool dismissed, const QDBusVariant &result);

  signals:
    void passwordLoaded(QString password);
    void passwordError();
};

#endif
