#include "StationModel.hpp"
#include <HttpClient.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

StationModel::StationModel(const std::vector<Station>& stations, QObject* parent)
    : QAbstractListModel(parent), m_stations(stations) {}

int StationModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_stations.size());
}

QVariant StationModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      static_cast<size_t>(index.row()) >= m_stations.size()) {
    return {};
  }

  const Station& station = m_stations[index.row()];

  switch (static_cast<StationRoles>(role)) {
    case StationRoles::LABEL_ROLE:
      return QString::fromStdString(station.getLabel());
    case StationRoles::TOWN_ROLE:
      return QString::fromStdString(station.getTown());
    case StationRoles::LATITUDE_ROLE:
      return station.getLat();
    case StationRoles::LONGITUDE_ROLE:
      return station.getLon();
    case StationRoles::RLOI_ROLE:
      return QString::fromStdString(station.getRLOIid());
    case StationRoles::CATCHMENT_ROLE:
      return QString::fromStdString(station.getCatchmentName());
    case StationRoles::DATE_ROLE:
      return QString::fromStdString(station.getDateOpened());
    case StationRoles::RIVER_ROLE:
      return QString::fromStdString(station.getRiverName());
    case StationRoles::NOTATION_ROLE:
      return QString::fromStdString(station.getNotation());
    case StationRoles::MEASURES_ROLE: {
      const auto& measures = station.getMeasures();
      QVariantList result;
      for (const auto& m : measures) {
        QVariantMap map;
        map["parameter"] = QString::fromStdString(m.getParameter());
        map["parameterName"] = QString::fromStdString(m.getParameterName());
        map["qualifier"] = QString::fromStdString(m.getQualifier());
        map["latestReading"] = m.getLatestReading();
        map["unitName"] = QString::fromStdString(m.getUnitName());
        result.append(map);
      }
      return result;
    }
  }
  return {};
}

QHash<int, QByteArray> StationModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[static_cast<int>(StationRoles::LABEL_ROLE)] = "label";
  roles[static_cast<int>(StationRoles::TOWN_ROLE)] = "town";
  roles[static_cast<int>(StationRoles::LATITUDE_ROLE)] = "latitude";
  roles[static_cast<int>(StationRoles::LONGITUDE_ROLE)] = "longitude";
  roles[static_cast<int>(StationRoles::RLOI_ROLE)] = "RLOIid";
  roles[static_cast<int>(StationRoles::CATCHMENT_ROLE)] = "catchmentName";
  roles[static_cast<int>(StationRoles::DATE_ROLE)] = "dateOpened";
  roles[static_cast<int>(StationRoles::RIVER_ROLE)] = "riverName";
  roles[static_cast<int>(StationRoles::NOTATION_ROLE)] = "notation";
  roles[static_cast<int>(StationRoles::MEASURES_ROLE)] = "measures";
  return roles;
}

bool StationModel::fetchMeasures(int index) {
  if (index < 0 || static_cast<size_t>(index) >= m_stations.size()) {
    return false;
  }

  Station& station = m_stations[index];

  std::string url = "https://environment.data.gov.uk/flood-monitoring/id/stations/" +
                    station.getNotation() + "/measures";

  auto response = HttpClient::getInstance().fetchUrl(url);
  if (!response) {
    std::cerr << "Failed to fetch measures for station " << station.getNotation() << '\n';
    return false;
  }

  try {
    json data = json::parse(*response);

    if (data.contains("items") && data["items"].is_array()) {
      std::vector<Measure> measures;
      for (const auto& measureJson : data["items"]) {
        measures.push_back(Measure::fromJson(measureJson));
      }
      station.setMeasures(measures);

      // Notify QML that data changed
      QModelIndex modelIndex = this->index(index, 0);
      emit dataChanged(modelIndex, modelIndex);

      return true;
    }
  } catch (const json::parse_error& e) {
    std::cerr << "JSON Parse Error for measures: " << e.what() << '\n';
    return false;
  }

  return false;
}
