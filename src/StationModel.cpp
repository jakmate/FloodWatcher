#include "StationModel.hpp"
#include "HttpClient.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

StationModel::StationModel(const std::vector<Station> &stations, QObject *parent)
    : QAbstractListModel(parent), m_stations(stations) {}

StationModel::~StationModel() = default;

int StationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_stations.size());
}

QVariant StationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 ||
        static_cast<size_t>(index.row()) >= m_stations.size())
    {
        return QVariant();
    }

    const Station &station = m_stations[index.row()];

    switch (role)
    {
    case LatitudeRole:
        return station.getLat();
    case LongitudeRole:
        return station.getLon();
    case LabelRole:
        return QString::fromStdString(station.getLabel());
    case TownRole:
        return QString::fromStdString(station.getTown());
    }
    return QVariant();
}

QHash<int, QByteArray> StationModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LatitudeRole] = "latitude";
    roles[LongitudeRole] = "longitude";
    roles[LabelRole] = "label";
    roles[TownRole] = "town";
    return roles;
}

QVariantMap StationModel::getStation(int index) const
{
    if (index < 0 || static_cast<size_t>(index) >= m_stations.size())
        return {};

    const Station &station = m_stations[index];
    QVariantMap data;
    data["label"] = QString::fromStdString(station.getLabel());
    data["town"] = QString::fromStdString(station.getTown());
    data["latitude"] = station.getLat();
    data["longitude"] = station.getLon();
    data["RLOIid"] = QString::fromStdString(station.getRLOIid());
    data["catchmentName"] = QString::fromStdString(station.getCatchmentName());
    data["dateOpened"] = QString::fromStdString(station.getDateOpened());
    data["riverName"] = QString::fromStdString(station.getRiverName());
    data["notation"] = QString::fromStdString(station.getNotation());

    return data;
}

QVariantList StationModel::getMeasures(int index) const
{
    if (index < 0 || static_cast<size_t>(index) >= m_stations.size())
        return {};

    const Station &station = m_stations[index];
    const auto &measures = station.getMeasures();

    QVariantList result;
    for (const auto &measure : measures)
    {
        QVariantMap m;
        m["parameter"] = QString::fromStdString(measure.getParameter());
        m["parameterName"] = QString::fromStdString(measure.getParameterName());
        m["unitName"] = QString::fromStdString(measure.getUnitName());
        m["qualifier"] = QString::fromStdString(measure.getQualifier());
        m["latestReading"] = measure.getLatestReading();
        result.append(m);
    }

    return result;
}

bool StationModel::fetchMeasures(int index)
{
    if (index < 0 || static_cast<size_t>(index) >= m_stations.size())
        return false;

    Station &station = m_stations[index];

    std::string url = "https://environment.data.gov.uk/flood-monitoring/id/stations/" +
                      station.getNotation() + "/measures";

    auto response = fetchUrl(url);
    if (!response)
    {
        std::cerr << "Failed to fetch measures for station " << station.getNotation() << std::endl;
        return false;
    }

    try
    {
        json data = json::parse(*response);

        if (data.contains("items") && data["items"].is_array())
        {
            std::vector<Measure> measures;
            for (const auto &measureJson : data["items"])
            {
                measures.push_back(Measure::fromJson(measureJson));
            }
            station.setMeasures(measures);

            // Notify QML that data changed
            QModelIndex modelIndex = this->index(index, 0);
            emit dataChanged(modelIndex, modelIndex);

            return true;
        }
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON Parse Error for measures: " << e.what() << std::endl;
        return false;
    }

    return false;
}