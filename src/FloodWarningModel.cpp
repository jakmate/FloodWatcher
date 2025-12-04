#include "FloodWarningModel.hpp"
#include "FloodMonitoringData.hpp"
#include <HttpClient.hpp>
#include <QDateTime>
#include <QGeoCoordinate>
#include <algorithm>
#include <iostream>

FloodWarningModel::FloodWarningModel(const std::vector<FloodWarning>& warnings, QObject* parent)
    : QAbstractListModel(parent), m_warnings(warnings), m_updateTimer(new QTimer(this)) {
  // Sort by severity level (1 = most severe)
  std::sort(m_warnings.begin(), m_warnings.end(), [](const FloodWarning& a, const FloodWarning& b) {
    return a.getSeverityLevel() < b.getSeverityLevel();
  });

  connect(m_updateTimer, &QTimer::timeout, this, &FloodWarningModel::fetchWarnings);
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
    case FloodWarningRoles::MESSAGE_ROLE:
      return QString::fromStdString(warning.getMessage());
    default:
      return {};
  }
}

QHash<int, QByteArray> FloodWarningModel::roleNames() const {
  return {{static_cast<int>(FloodWarningRoles::DESCRIPTION_ROLE), "description"},
          {static_cast<int>(FloodWarningRoles::SEVERITY_ROLE), "severity"},
          {static_cast<int>(FloodWarningRoles::SEVERITY_LEVEL_ROLE), "severityLevel"},
          {static_cast<int>(FloodWarningRoles::EA_AREA_NAME_ROLE), "eaAreaName"},
          {static_cast<int>(FloodWarningRoles::POLYGON_PATH_ROLE), "polygonPath"},
          {static_cast<int>(FloodWarningRoles::MESSAGE_ROLE), "message"}};
}

void FloodWarningModel::startAutoUpdate() {
  int delayMs = calculateNextUpdateMs();

  std::cout << "Starting auto-update, next fetch in " << (delayMs / 1000.0) << " seconds\n";

  // Schedule first update
  m_updateTimer->setSingleShot(true);
  m_updateTimer->start(delayMs);
}

void FloodWarningModel::stopAutoUpdate() {
  m_updateTimer->stop();
  std::cout << "Auto-update stopped\n";
}

void FloodWarningModel::fetchWarnings() {
  std::cout << "Fetching flood warnings at "
            << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString() << "\n";

  auto response =
      HttpClient::fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");

  if (!response) {
    std::cerr << "Failed to fetch flood warnings\n";
    // Retry at next interval
    m_updateTimer->start(calculateNextUpdateMs());
    return;
  }

  try {
    json data = json::parse(*response);
    FloodMonitoringData tempData;
    tempData.parseFloodWarnings(data);

    updateWarnings(tempData.getFloodWarnings());

    std::cout << "Updated: " << m_warnings.size() << " warnings\n";
    emit warningsUpdated(static_cast<int>(m_warnings.size()));

  } catch (const json::parse_error& e) {
    std::cerr << "JSON Parse Error: " << e.what() << "\n";
  }

  // Schedule next update
  m_updateTimer->start(calculateNextUpdateMs());
}

void FloodWarningModel::updateWarnings(const std::vector<FloodWarning>& newWarnings) {
  beginResetModel();
  m_warnings = newWarnings;
  m_warnings.shrink_to_fit();

  // Sort by severity level
  std::sort(m_warnings.begin(), m_warnings.end(), [](const FloodWarning& a, const FloodWarning& b) {
    return a.getSeverityLevel() < b.getSeverityLevel();
  });

  endResetModel();
}

int FloodWarningModel::calculateNextUpdateMs() {
  QDateTime now = QDateTime::currentDateTime();
  int currentMinute = now.time().minute();
  int currentSecond = now.time().second();
  int currentMs = now.time().msec();

  // Find next quarter hour (0, 15, 30, 45)
  int nextQuarter = ((currentMinute / 15) + 1) * 15;
  if (nextQuarter >= 60) {
    nextQuarter = 0;
  }

  int minutesToNext = nextQuarter - currentMinute;
  if (minutesToNext <= 0) {
    minutesToNext += 60;
  }

  // Convert to milliseconds and subtract elapsed time in current minute
  int delayMs = ((minutesToNext * 60 - currentSecond) * 1000) - currentMs;

  return delayMs;
}

QVariantList FloodWarningModel::getPolygonPath(const FloodWarning& warning) {
  const auto& multiPolygon = warning.getFloodAreaPolygon();
  if (!multiPolygon.has_value() || multiPolygon->empty() || (*multiPolygon)[0].empty()) {
    return {};
  }

  const auto& exteriorRing = (*multiPolygon)[0][0];
  QVariantList coordinates;
  coordinates.reserve(static_cast<qsizetype>(exteriorRing.size()));

  for (const auto& coord : exteriorRing) {
    // coord.first = longitude, coord.second = latitude
    coordinates.append(QVariant::fromValue(QGeoCoordinate(coord.second, coord.first)));
  }

  return coordinates;
}
