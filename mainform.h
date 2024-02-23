#ifndef MPV_YTDL_GUI_MAINFORM_H
#define MPV_YTDL_GUI_MAINFORM_H

#include <QDBusVariant>
#include <QFutureWatcher>
#include <QObject>
#include <QStringList>
#include <QStringListModel>
#include <QtConcurrent/QtConcurrent>
#include "formats.h"
#include "passwordhandler.h"

class MainForm : public QObject {
    Q_OBJECT
    Q_PROPERTY(FormatDataModel *formatsModel READ getFormatsModel NOTIFY formatsChanged)
    Q_PROPERTY(bool hasPassword READ hasPassword)

  public:
    explicit MainForm(QObject *parent = nullptr);
    ~MainForm();
    FormatDataModel *getFormatsModel()
    {
        return &formatsModel;
    }
    bool hasPassword() const
    {
        return !password.isEmpty();
    };

  public slots:
    void loadPassword(const QString &p1, const QString &p2);
    void passwordLoaded(const QString &password);
    void loadFormats(const QString &username, const QString &url, const QString &cookiesFromBrowser);
    void start(const QString &username, const QString &sublang, const QString &cookiesFromBrowser);

  signals:
    void formatsChanged();
    void ytdlpError();
    void passwordError();
    void noSelectionError();

  private:
    QString password;
    PasswordHandler *passwordHandler = new PasswordHandler();
    FormatDataModel formatsModel;
    using FormatPair = std::pair<QString, QList<FormatData>>;
    QFuture<FormatPair> ytFuture;
    QFutureWatcher<FormatPair> ytFutureWatcher;

  private slots:
    void formatsLoaded();
};

#endif
