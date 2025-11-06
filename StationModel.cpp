#include "StationModel.hpp"

StationModel::StationModel(const std::vector<Station>& stations, QObject *parent)
    : QAbstractListModel(parent), m_stations(stations) {}

int StationModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_stations.size());
}

QVariant StationModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || 
        static_cast<size_t>(index.row()) >= m_stations.size()) {
        return QVariant();
    }
    
    const Station& station = m_stations[index.row()];
    
    switch (role) {
        case LatitudeRole: return station.getLat();
        case LongitudeRole: return station.getLon();
        case LabelRole: return QString::fromStdString(station.getLabel());
        case TownRole: return QString::fromStdString(station.getTown());
    }
    return QVariant();
}

QHash<int, QByteArray> StationModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[LatitudeRole] = "latitude";
    roles[LongitudeRole] = "longitude";
    roles[LabelRole] = "label";
    roles[TownRole] = "town";
    return roles;
}

QVariantMap StationModel::getStation(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= m_stations.size()) 
        return {};
    
    const Station& station = m_stations[index];
    QVariantMap data;
    data["label"]          = QString::fromStdString(station.getLabel());
    data["town"]           = QString::fromStdString(station.getTown());
    data["latitude"]       = station.getLat();
    data["longitude"]      = station.getLon();
    data["RLOIid"]         = QString::fromStdString(station.getRLOIid());
    data["catchmentName"]  = QString::fromStdString(station.getCatchmentName());
    data["dateOpened"]     = QString::fromStdString(station.getDateOpened());
    data["riverName"]      = QString::fromStdString(station.getRiverName());

    return data;
}