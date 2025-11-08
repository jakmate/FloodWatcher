#include "FloodWarningModel.hpp"
#include <algorithm>

FloodWarningModel::FloodWarningModel(const std::vector<FloodWarning>& warnings, QObject* parent)
    : QAbstractListModel(parent), m_warnings(warnings) {
    // Sort by severity level (1 = most severe)
    std::sort(m_warnings.begin(), m_warnings.end(), 
        [](const FloodWarning& a, const FloodWarning& b) {
            return a.getSeverityLevel() < b.getSeverityLevel();
        });
}

FloodWarningModel::~FloodWarningModel() = default;

int FloodWarningModel::rowCount(const QModelIndex &) const
{
    return m_warnings.size();
}

QVariant FloodWarningModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 ||
        static_cast<size_t>(index.row()) >= m_warnings.size())
    {
        return QVariant();
    }

    const auto &warning = m_warnings[index.row()];

    switch (role)
    {
    case DescriptionRole:
        return QString::fromStdString(warning.getDescription());
    case SeverityRole:
        return QString::fromStdString(warning.getSeverity());
    case SeverityLevelRole:
        return warning.getSeverityLevel();
    case EaAreaNameRole:
        return QString::fromStdString(warning.getAreaName());
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> FloodWarningModel::roleNames() const
{
    return {
        {DescriptionRole, "description"},
        {SeverityRole, "severity"},
        {SeverityLevelRole, "severityLevel"},
        {EaAreaNameRole, "eaAreaName"}};
}
