#ifndef MPV_YTDL_GUI_FORMATS_H
#define MPV_YTDL_GUI_FORMATS_H

#include <QAbstractListModel>

using fps_t = unsigned long;
using filesize_t = unsigned long long;

class FormatData {
  public:
    explicit FormatData(const QString &id, const QString &name, const QString &resolution, const QString &vcodec,
                        const QString &acodec, fps_t fps, filesize_t filesize, bool filesizeEstimate, double tbr);
    bool selected() const;
    void selected(bool);
    QString id() const;
    QString name() const;
    QString resolution() const;
    QString vcodec() const;
    QString acodec() const;
    fps_t fps() const;
    filesize_t filesize() const;
    bool filesizeEstimate() const;
    double tbr() const;

  private:
    bool m_selected = false;
    QString m_id;
    QString m_name;
    QString m_resolution;
    QString m_vcodec;
    QString m_acodec;
    fps_t m_fps;
    filesize_t m_filesize;
    bool m_filesizeEstimate;
    double m_tbr;
};

class FormatDataModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum FormatDataRoles {
        SelectedRole = Qt::UserRole + 1,
        NameRole,
        ResolutionRole,
        VcodecRole,
        AcodecRole,
        FpsRole,
        FilesizeRole,
        FilesizeEstimateRole,
        TbrRole
    };

    FormatDataModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    void setFormatList(const QString &searchUrl, const QList<FormatData> &);
    const QString &searchUrl() const;
    const FormatData &operator[](qsizetype idx) const;

  protected:
    QHash<int, QByteArray> roleNames() const;

  private:
    QList<FormatData> m_formats;
    QString m_searchUrl;
};

class QVideoData {
  public:
    explicit QVideoData(): duration(0)
    {
    }
    explicit QVideoData(const QString &name, uint64_t duration, QList<FormatData> formats)
        : title(name), duration(duration), formats(formats)
    {
    }
    const QString &getTitle() const noexcept
    {
        return title;
    }
    uint64_t getDuration() const noexcept
    {
        return duration;
    }
    const QList<FormatData> getFormats() const noexcept
    {
        return formats;
    }

  private:
    QString title;
    uint64_t duration;
    QList<FormatData> formats;
};

#endif
