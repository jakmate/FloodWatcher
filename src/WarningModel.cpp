#include "WarningModel.hpp"
#include "MonitoringData.hpp"
#include <HttpClient.hpp>
#include <QDateTime>
#include <QGeoCoordinate>
#include <algorithm>
#include <iostream>

WarningModel::WarningModel(const std::vector<Warning>& warnings, QObject* parent)
    : QAbstractListModel(parent), m_warnings(warnings), m_updateTimer(new QTimer(this)) {
  // Sort by severity level (1 = most severe)
  std::sort(m_warnings.begin(), m_warnings.end(), [](const Warning& a, const Warning& b) {
    return a.getSeverityLevel() < b.getSeverityLevel();
  });

  connect(m_updateTimer, &QTimer::timeout, this, &WarningModel::fetchWarnings);
}

int WarningModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_warnings.size());
}

QVariant WarningModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      static_cast<size_t>(index.row()) >= m_warnings.size()) {
    return {};
  }

  const auto& warning = m_warnings[index.row()];

  switch (static_cast<WarningRoles>(role)) {
    case WarningRoles::DESCRIPTION_ROLE:
      return QString::fromStdString(warning.getDescription());
    case WarningRoles::SEVERITY_ROLE:
      return QString::fromStdString(warning.getSeverity());
    case WarningRoles::SEVERITY_LEVEL_ROLE:
      return warning.getSeverityLevel();
    case WarningRoles::EA_AREA_NAME_ROLE:
      return QString::fromStdString(warning.getAreaName());
    case WarningRoles::POLYGON_PATH_ROLE:
      return getPolygonPath(warning);
    case WarningRoles::MESSAGE_ROLE:
      return QString::fromStdString(warning.getMessage());
    default:
      return {};
  }
}

QHash<int, QByteArray> WarningModel::roleNames() const {
  return {{static_cast<int>(WarningRoles::DESCRIPTION_ROLE), "description"},
          {static_cast<int>(WarningRoles::SEVERITY_ROLE), "severity"},
          {static_cast<int>(WarningRoles::SEVERITY_LEVEL_ROLE), "severityLevel"},
          {static_cast<int>(WarningRoles::EA_AREA_NAME_ROLE), "eaAreaName"},
          {static_cast<int>(WarningRoles::POLYGON_PATH_ROLE), "polygonPath"},
          {static_cast<int>(WarningRoles::MESSAGE_ROLE), "message"}};
}

void WarningModel::startAutoUpdate() {
  int delayMs = calculateNextUpdateMs();

  std::cout << "Starting auto-update, next fetch in " << (delayMs / 1000.0) << " seconds\n";

  // Schedule first update
  m_updateTimer->setSingleShot(true);
  m_updateTimer->start(delayMs);
}

void WarningModel::stopAutoUpdate() {
  m_updateTimer->stop();
  std::cout << "Auto-update stopped\n";
}

void WarningModel::fetchWarnings() {
  std::cout << "Fetching warnings at "
            << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString() << "\n";

  auto response =
      HttpClient::fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");

  if (!response) {
    std::cerr << "Failed to fetch warnings\n";
    // Retry at next interval
    m_updateTimer->start(calculateNextUpdateMs());
    return;
  }

  try {
    json data = json::parse(*response);
    MonitoringData tempData;
    tempData.parseWarnings(data);

    updateWarnings(tempData.getWarnings());

    std::cout << "Updated: " << m_warnings.size() << " warnings\n";
    emit warningsUpdated(static_cast<int>(m_warnings.size()));

  } catch (const json::parse_error& e) {
    std::cerr << "JSON Parse Error: " << e.what() << "\n";
  }

  // Schedule next update
  m_updateTimer->start(calculateNextUpdateMs());
}

void WarningModel::updateWarnings(const std::vector<Warning>& newWarnings) {
  beginResetModel();
  m_warnings = newWarnings;
  m_warnings.shrink_to_fit();

  // Sort by severity level
  std::sort(m_warnings.begin(), m_warnings.end(), [](const Warning& a, const Warning& b) {
    return a.getSeverityLevel() < b.getSeverityLevel();
  });

  endResetModel();
}

int WarningModel::calculateNextUpdateMs() {
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

QVariantList WarningModel::getPolygonPath(const Warning& warning) {
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
