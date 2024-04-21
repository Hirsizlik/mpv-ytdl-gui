#include "mainform.h"
#include <cstring>
#include <exception>
#include <iostream>
#include <unistd.h>
#include "mpv-ytdl-gui-rs-cxx/lib.h"
#include "rust/cxx.h"
#include "formats.h"

MainForm::MainForm(QObject *parent): QObject(parent)
{
    connect(passwordHandler, &PasswordHandler::passwordLoaded, this, &MainForm::passwordLoaded);
    connect(passwordHandler, &PasswordHandler::passwordError, this, &MainForm::passwordError);
    connect(&ytFutureWatcher, &QFutureWatcher<FormatPair>::finished, this, &MainForm::formatsLoaded);
}

MainForm::~MainForm()
{
    delete passwordHandler;
}

void
MainForm::loadPassword(const QString &attribute, const QString &value)
{
    passwordHandler->loadPassword(attribute, value);
}

void
MainForm::passwordLoaded(const QString &password)
{
    this->password = password;
}

FormatData
map(const VideoFormat &data)
{
    QString vcodec;
    if (data.vcodec != "none") {
        vcodec = QString::fromUtf8(data.vcodec);
    }
    QString acodec;
    if (data.acodec != "none") {
        acodec = QString::fromUtf8(data.acodec);
    }
    return FormatData(QString::fromUtf8(data.id), QString::fromUtf8(data.name), QString::fromUtf8(data.resolution),
                      vcodec, acodec, data.fps, data.filesize, data.filesize_is_estimate, data.tbr);
}

QList<FormatData>
convertFromVideoFormat(const rust::cxxbridge1::Vec<VideoFormat> &data)
{
    QList<FormatData> list;
    for (const VideoFormat &vf : data) {
        list.emplaceBack(map(vf));
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

    auto videoToPair = [url](rust::cxxbridge1::Vec<VideoFormat> data) {
        return std::make_pair(url, convertFromVideoFormat(data));
    };

    ytFuture = QtConcurrent::run(load_video_formats, username.toStdString(), password.toStdString(), url.toStdString(),
                                 cookiesFromBrowser.toStdString())
                   .then(videoToPair)
                   .onFailed([](std::runtime_error) { return std::make_pair("", QList<FormatData>()); });
    ytFutureWatcher.setFuture(ytFuture);
}

void
MainForm::formatsLoaded()
{
    FormatPair result = ytFutureWatcher.future().result();
    if (result.second.empty()) {
        emit ytdlpError();
    } else {
        formatsModel.setFormatList(result.first, std::move(result.second));
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
        if (!cookiesBrowser.isEmpty())
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
