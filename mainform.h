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
    Q_PROPERTY(QString videoTitle READ getVideoTitle NOTIFY videoChanged)
    Q_PROPERTY(QString videoDuration READ getVideoDuration NOTIFY videoChanged)

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
    const QString &getVideoTitle() const
    {
        return videoTitle;
    }
    const QString &getVideoDuration() const
    {
        return videoDuration;
    }

  public slots:
    void loadPassword(const QString &p1, const QString &p2);
    void passwordLoaded(const QString &password);
    void loadFormats(const QString &username, const QString &url, const QString &cookiesFromBrowser,
                     const QString &userAgent);
    void start(const QString &username, const QString &sublang, const QString &cookiesFromBrowser,
               const QString &userAgent);

  signals:
    void formatsChanged();
    void videoChanged();
    void ytdlpError();
    void passwordError();
    void noSelectionError();

  private:
    QString password;
    PasswordHandler *passwordHandler = new PasswordHandler();
    QString videoTitle;
    QString videoDuration;
    FormatDataModel formatsModel;
    using FormatPair = std::pair<QString, QVideoData>;
    QFuture<FormatPair> ytFuture;
    QFutureWatcher<FormatPair> ytFutureWatcher;

  private slots:
    void formatsLoaded();
};

#endif
