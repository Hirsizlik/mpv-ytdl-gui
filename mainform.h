#ifndef MPV_GUI_MAINFORM_HPP
#define MPV_GUI_MAINFORM_HPP

#include <QObject>
#include <QDBusVariant>
#include <QStringList>
#include <QStringListModel>
#include "formats.h"

class MainForm : public QObject {
    Q_OBJECT
    Q_PROPERTY(FormatDataModel* formatsModel READ getFormatsModel NOTIFY formatsChanged)
    Q_PROPERTY(bool hasPassword READ hasPassword)

public:
    explicit MainForm(QObject *parent = nullptr);
    FormatDataModel* getFormatsModel() { return &formatsModel; }
    bool hasPassword() const { return !password.isEmpty(); };

public slots:
    void loadPassword(const QString& p1, const QString& p2);
    void passwordPromptCompleted(const bool dismissed, const QDBusVariant& value);
    void loadFormats(const QString& username, const QString& url, const QString& cookiesFromBrowser);
    void start(const QString& username, const QString& sublang, const QString& cookiesFromBrowser);

signals:
    void formatsChanged();
    void ytdlpError();
    void passwordError();
    void noSelectionError();

private:
    QString password;
    FormatDataModel formatsModel;
    QString getSecret(const QDBusObjectPath &pathToUnlockedPassword);
};

#endif
