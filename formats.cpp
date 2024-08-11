#include "formats.h"
#include <qnamespace.h>

FormatData::FormatData(const QString &id, const QString &name, const QString &resolution, const QString &vcodec,
                       const QString &acodec, fps_t fps, filesize_t filesize, bool filesizeEstimate, double tbr)
    : m_id(id), m_name(name), m_resolution(resolution), m_vcodec(vcodec), m_acodec(acodec), m_fps(fps),
      m_filesize(filesize), m_filesizeEstimate(filesizeEstimate), m_tbr(tbr)
{
}

bool
FormatData::selected() const
{
    return m_selected;
}

void
FormatData::selected(bool newValue)
{
    m_selected = newValue;
}

QString
FormatData::id() const
{
    return m_id;
}

QString
FormatData::name() const
{
    return m_name;
}

QString
FormatData::resolution() const
{
    return m_resolution;
}

QString
FormatData::vcodec() const
{
    return m_vcodec;
}

QString
FormatData::acodec() const
{
    return m_acodec;
}

fps_t
FormatData::fps() const
{
    return m_fps;
}

filesize_t
FormatData::filesize() const
{
    return m_filesize;
}

bool
FormatData::filesizeEstimate() const
{
    return m_filesizeEstimate;
}

double
FormatData::tbr() const
{
    return m_tbr;
}

FormatDataModel::FormatDataModel(QObject *parent): QAbstractListModel(parent)
{
}

int
FormatDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_formats.count();
}

QVariant
FormatDataModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_formats.count())
        return QVariant();

    const FormatData &format = m_formats[index.row()];
    switch (role) {
    case SelectedRole:
        return QVariant::fromValue(format.selected());
    case NameRole:
        return format.name();
    case ResolutionRole:
        return format.resolution();
    case VcodecRole:
        return format.vcodec();
    case AcodecRole:
        return format.acodec();
    case FpsRole:
        return QVariant::fromValue(format.fps());
    case FilesizeRole:
        return QVariant::fromValue(format.filesize());
    case FilesizeEstimateRole:
        return QVariant::fromValue(format.filesizeEstimate());
    case TbrRole:
        return QVariant::fromValue(format.tbr());
    default:
        return QVariant();
    }
}

bool
FormatDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != SelectedRole || index.row() < 0 || index.row() >= m_formats.count()) {
        return false;
    }
    FormatData &format = m_formats[index.row()];
    format.selected(value.toBool());
    emit dataChanged(index, index);
    return true;
}

QHash<int, QByteArray>
FormatDataModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SelectedRole] = "selected";
    roles[NameRole] = "name";
    roles[ResolutionRole] = "resolution";
    roles[VcodecRole] = "vcodec";
    roles[AcodecRole] = "acodec";
    roles[FpsRole] = "fps";
    roles[FilesizeRole] = "filesize";
    roles[FilesizeEstimateRole] = "filesize_estimate";
    roles[TbrRole] = "tbr";

    return roles;
}

void
FormatDataModel::setFormatList(const QString &searchUrl, const QList<FormatData> &list)
{
    beginResetModel();
    m_searchUrl = searchUrl;
    m_formats = list;
    endResetModel();
}

const FormatData &
FormatDataModel::operator[](qsizetype idx) const
{
    return m_formats[idx];
}

const QString &
FormatDataModel::searchUrl() const
{
    return m_searchUrl;
}
