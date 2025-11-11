#include "FloodWarningModel.hpp"
#include <QGeoCoordinate>
#include <algorithm>
#include <iostream>

FloodWarningModel::FloodWarningModel(const std::vector<FloodWarning>& warnings, QObject* parent)
    : QAbstractListModel(parent), m_warnings(warnings) {
  // Sort by severity level (1 = most severe)
  std::sort(m_warnings.begin(), m_warnings.end(), [](const FloodWarning& a, const FloodWarning& b) {
    return a.getSeverityLevel() < b.getSeverityLevel();
  });
}

int FloodWarningModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_warnings.size());
}

QVariant FloodWarningModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      static_cast<size_t>(index.row()) >= m_warnings.size()) {
    return {};
  }

  const auto& warning = m_warnings[index.row()];

  switch (static_cast<FloodWarningRoles>(role)) {
    case FloodWarningRoles::DESCRIPTION_ROLE:
      return QString::fromStdString(warning.getDescription());
    case FloodWarningRoles::SEVERITY_ROLE:
      return QString::fromStdString(warning.getSeverity());
    case FloodWarningRoles::SEVERITY_LEVEL_ROLE:
      return warning.getSeverityLevel();
    case FloodWarningRoles::EA_AREA_NAME_ROLE:
      return QString::fromStdString(warning.getAreaName());
    case FloodWarningRoles::POLYGON_PATH_ROLE:
      return getPolygonPath(warning);
    default:
      return {};
  }
}

QHash<int, QByteArray> FloodWarningModel::roleNames() const {
  return {{static_cast<int>(FloodWarningRoles::DESCRIPTION_ROLE), "description"},
          {static_cast<int>(FloodWarningRoles::SEVERITY_ROLE), "severity"},
          {static_cast<int>(FloodWarningRoles::SEVERITY_LEVEL_ROLE), "severityLevel"},
          {static_cast<int>(FloodWarningRoles::EA_AREA_NAME_ROLE), "eaAreaName"},
          {static_cast<int>(FloodWarningRoles::POLYGON_PATH_ROLE), "polygonPath"}};
}

QVariantList FloodWarningModel::getPolygonPath(const FloodWarning& warning) {
  QVariantList paths;

  const auto& multiPolygon = warning.getFloodAreaPolygon();
  if (!multiPolygon.has_value() || multiPolygon->empty()) {
    return paths;
  }

  // For each polygon in the multipolygon
  for (const auto& polygon : *multiPolygon) {
    if (polygon.empty()) {
      continue;
    }

    // Get the exterior ring
    const auto& exteriorRing = polygon[0];

    QVariantList coordinates;
    for (const auto& coord : exteriorRing) {
      // coord.first = longitude, coord.second = latitude
      QGeoCoordinate geoCoord(coord.second, coord.first);
      coordinates.append(QVariant::fromValue(geoCoord));
    }

    if (!coordinates.isEmpty()) {
      paths.append(QVariant::fromValue(coordinates));
    }
  }

  // QML MapPolygon can only handle one path at a time
  if (!paths.isEmpty()) {
    return paths[0].toList();
  }

  return {};
}
