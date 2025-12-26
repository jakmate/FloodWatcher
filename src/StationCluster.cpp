#include "StationCluster.hpp"
#include <cmath>
#include <algorithm>

// QuadTreeNode implementation
QuadTreeNode::QuadTreeNode(double minLat, double maxLat, double minLon, double maxLon, int maxDepth)
    : m_minLat(minLat), m_maxLat(maxLat), m_minLon(minLon), m_maxLon(maxLon),
      m_depth(0), m_maxDepth(maxDepth) {}

void QuadTreeNode::insert(const ClusterPoint& point) {
  if (point.lat < m_minLat || point.lat > m_maxLat ||
      point.lon < m_minLon || point.lon > m_maxLon) {
    return;
  }
  
  if (m_nw != nullptr) {
    // Already subdivided
    double midLat = (m_minLat + m_maxLat) / 2.0;
    double midLon = (m_minLon + m_maxLon) / 2.0;
    
    if (point.lat >= midLat && point.lon < midLon) {
      m_nw->insert(point);
    } else if (point.lat >= midLat && point.lon >= midLon) {
      m_ne->insert(point);
    } else if (point.lat < midLat && point.lon < midLon) {
      m_sw->insert(point);
    } else {
      m_se->insert(point);
    }
  } else {
    m_points.push_back(point);
    
    if (shouldSubdivide()) {
      subdivide();
    }
  }
}

bool QuadTreeNode::shouldSubdivide() const {
  return m_points.size() > 10 && m_depth < m_maxDepth;
}

void QuadTreeNode::subdivide() {
  double midLat = (m_minLat + m_maxLat) / 2.0;
  double midLon = (m_minLon + m_maxLon) / 2.0;
  
  m_nw = std::make_unique<QuadTreeNode>(midLat, m_maxLat, m_minLon, midLon, m_maxDepth);
  m_ne = std::make_unique<QuadTreeNode>(midLat, m_maxLat, midLon, m_maxLon, m_maxDepth);
  m_sw = std::make_unique<QuadTreeNode>(m_minLat, midLat, m_minLon, midLon, m_maxDepth);
  m_se = std::make_unique<QuadTreeNode>(m_minLat, midLat, midLon, m_maxLon, m_maxDepth);
  
  m_nw->m_depth = m_depth + 1;
  m_ne->m_depth = m_depth + 1;
  m_sw->m_depth = m_depth + 1;
  m_se->m_depth = m_depth + 1;
  
  for (const auto& point : m_points) {
    if (point.lat >= midLat && point.lon < midLon) {
      m_nw->insert(point);
    } else if (point.lat >= midLat && point.lon >= midLon) {
      m_ne->insert(point);
    } else if (point.lat < midLat && point.lon < midLon) {
      m_sw->insert(point);
    } else {
      m_se->insert(point);
    }
  }
  
  m_points.clear();
}

Cluster QuadTreeNode::aggregatePoints() const {
  Cluster cluster;
  cluster.lat = 0.0;
  cluster.lon = 0.0;
  cluster.count = static_cast<int>(m_points.size());
  
  for (const auto& point : m_points) {
    cluster.lat += point.lat;
    cluster.lon += point.lon;
    cluster.stationIndices.push_back(point.stationIndex);
  }
  
  if (cluster.count > 0) {
    cluster.lat /= cluster.count;
    cluster.lon /= cluster.count;
  }
  
  return cluster;
}

void QuadTreeNode::collectAllPoints(std::vector<ClusterPoint>& points) const {
  if (m_nw == nullptr) {
    points.insert(points.end(), m_points.begin(), m_points.end());
  } else {
    m_nw->collectAllPoints(points);
    m_ne->collectAllPoints(points);
    m_sw->collectAllPoints(points);
    m_se->collectAllPoints(points);
  }
}

std::vector<Cluster> QuadTreeNode::getClusters(double zoomLevel, double minDistance) const {
    std::vector<Cluster> clusters;
    
    // Calculate node size more accurately for geographic clustering
    double latDiff = m_maxLat - m_minLat;
    double lonDiff = m_maxLon - m_minLon;
    
    // Use approximate geographic distance (rough estimate)
    double avgLat = (m_minLat + m_maxLat) / 2.0;
    double metersPerDegreeLat = 111000; // Approximate
    double metersPerDegreeLon = 111000 * std::cos(avgLat * M_PI / 180.0);
    
    double nodeWidthMeters = lonDiff * metersPerDegreeLon;
    double nodeHeightMeters = latDiff * metersPerDegreeLat;
    double nodeSizeMeters = std::sqrt(nodeWidthMeters * nodeWidthMeters + nodeHeightMeters * nodeHeightMeters);
    
    // Convert minDistance from degrees to meters for comparison
    double minDistanceMeters = minDistance * metersPerDegreeLat;
    
    // If this node has no children (leaf node)
    if (m_nw == nullptr) {
        // If the node is small enough or we're at high zoom, show individual points
        if (zoomLevel >= 10.0 || m_points.size() <= 1 || nodeSizeMeters < minDistanceMeters * 0.5) {
            for (const auto& point : m_points) {
                Cluster c;
                c.lat = point.lat;
                c.lon = point.lon;
                c.count = 1;
                c.stationIndices.push_back(point.stationIndex);
                clusters.push_back(c);
            }
        } else {
            // Otherwise, cluster all points in this node
            Cluster cluster = aggregatePoints();
            clusters.push_back(cluster);
        }
    } else {
        // If this node has children (branch node)
        
        // For low zoom levels, we want to see larger clusters but not a single massive cluster
        if (zoomLevel < 8.0) {
            // Aggregate all points in this node, but limit the cluster size
            std::vector<ClusterPoint> allPoints;
            collectAllPoints(allPoints);
            
            // If there are too many points, subdivide further
            if (allPoints.size() > 100) {
                // Instead of aggregating everything, recurse into children
                auto nwClusters = m_nw->getClusters(zoomLevel, minDistance);
                auto neClusters = m_ne->getClusters(zoomLevel, minDistance);
                auto swClusters = m_sw->getClusters(zoomLevel, minDistance);
                auto seClusters = m_se->getClusters(zoomLevel, minDistance);
                
                clusters.insert(clusters.end(), nwClusters.begin(), nwClusters.end());
                clusters.insert(clusters.end(), neClusters.begin(), neClusters.end());
                clusters.insert(clusters.end(), swClusters.begin(), swClusters.end());
                clusters.insert(clusters.end(), seClusters.begin(), seClusters.end());
            } else {
                // If there are not too many points, create a single cluster
                Cluster cluster;
                cluster.lat = 0.0;
                cluster.lon = 0.0;
                cluster.count = static_cast<int>(allPoints.size());
                for (const auto& point : allPoints) {
                    cluster.lat += point.lat;
                    cluster.lon += point.lon;
                    cluster.stationIndices.push_back(point.stationIndex);
                }
                cluster.lat /= cluster.count;
                cluster.lon /= cluster.count;
                clusters.push_back(cluster);
            }
        } else {
            // Otherwise, recurse into children
            auto nwClusters = m_nw->getClusters(zoomLevel, minDistance);
            auto neClusters = m_ne->getClusters(zoomLevel, minDistance);
            auto swClusters = m_sw->getClusters(zoomLevel, minDistance);
            auto seClusters = m_se->getClusters(zoomLevel, minDistance);
            
            clusters.insert(clusters.end(), nwClusters.begin(), nwClusters.end());
            clusters.insert(clusters.end(), neClusters.begin(), neClusters.end());
            clusters.insert(clusters.end(), swClusters.begin(), swClusters.end());
            clusters.insert(clusters.end(), seClusters.begin(), seClusters.end());
        }
    }
    
    return clusters;
}

// ClusterModel implementation
ClusterModel::ClusterModel(QObject* parent)
    : QAbstractListModel(parent) {}

void ClusterModel::setStations(const std::vector<Station>& stations) {
  beginResetModel();
  m_stations = stations;
  buildQuadTree();
  endResetModel();
}

void ClusterModel::buildQuadTree() {
    if (m_stations.empty()) {
        return;
    }
    
    // Calculate bounds from actual station data
    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;
    
    for (const auto& station : m_stations) {
        double lat = station.getLat();
        double lon = station.getLon();
        minLat = std::min(minLat, lat);
        maxLat = std::max(maxLat, lat);
        minLon = std::min(minLon, lon);
        maxLon = std::max(maxLon, lon);
    }
    
    // Add padding to ensure all stations are included
    double latPadding = (maxLat - minLat) * 0.1;
    double lonPadding = (maxLon - minLon) * 0.1;
    
    minLat -= std::max(0.1, latPadding);  // Minimum 0.1 degree padding
    maxLat += std::max(0.1, latPadding);
    minLon -= std::max(0.1, lonPadding);
    maxLon += std::max(0.1, lonPadding);
    
    m_quadTree = std::make_unique<QuadTreeNode>(minLat, maxLat, minLon, maxLon, 10);
    
    // Insert stations with validation
    for (size_t i = 0; i < m_stations.size(); ++i) {
        ClusterPoint point;
        point.lat = m_stations[i].getLat();
        point.lon = m_stations[i].getLon();
        point.stationIndex = static_cast<int>(i);
        
        // Only insert if within reasonable bounds (UK area)
        if (point.lat >= 49.0 && point.lat <= 61.0 && 
            point.lon >= -8.0 && point.lon <= 2.0) {
            m_quadTree->insert(point);
        }
    }
}

double ClusterModel::getMinDistanceForZoom(double zoomLevel) const {
    // More granular and realistic distances for UK scale
    if (zoomLevel >= 14.0) return 0.0005;   // ~50m
    if (zoomLevel >= 12.0) return 0.002;    // ~200m  
    if (zoomLevel >= 10.0) return 0.005;    // ~500m
    if (zoomLevel >= 9.0) return 0.01;      // ~1km
    if (zoomLevel >= 8.0) return 0.02;      // ~2km
    if (zoomLevel >= 7.0) return 0.04;      // ~4km
    return 0.08;                           // ~8km for zoom < 7
}

void ClusterModel::updateClusters(double zoomLevel) {
  if (!m_quadTree) {
    return;
  }
  
  beginResetModel();
  
  double minDistance = getMinDistanceForZoom(zoomLevel);
  auto clusters = m_quadTree->getClusters(zoomLevel, minDistance);
  
  m_displayItems.clear();
  m_displayItems.reserve(clusters.size());
  
  for (const auto& cluster : clusters) {
    ClusterItem item;
    item.lat = cluster.lat;
    item.lon = cluster.lon;
    item.count = cluster.count;
    item.isCluster = cluster.count > 1;
    item.stationIndex = item.isCluster ? -1 : cluster.stationIndices[0];
    m_displayItems.push_back(item);
  }
  
  endResetModel();
}

int ClusterModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_displayItems.size());
}

QVariant ClusterModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      static_cast<size_t>(index.row()) >= m_displayItems.size()) {
    return {};
  }
  
  const ClusterItem& item = m_displayItems[index.row()];
  
  switch (static_cast<ClusterRoles>(role)) {
    case ClusterRoles::LATITUDE_ROLE:
      return item.lat;
    case ClusterRoles::LONGITUDE_ROLE:
      return item.lon;
    case ClusterRoles::COUNT_ROLE:
      return item.count;
    case ClusterRoles::IS_CLUSTER_ROLE:
      return item.isCluster;
    case ClusterRoles::STATION_INDEX_ROLE:
      return item.stationIndex;
  }
  
  return {};
}

QHash<int, QByteArray> ClusterModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[static_cast<int>(ClusterRoles::LATITUDE_ROLE)] = "latitude";
  roles[static_cast<int>(ClusterRoles::LONGITUDE_ROLE)] = "longitude";
  roles[static_cast<int>(ClusterRoles::COUNT_ROLE)] = "count";
  roles[static_cast<int>(ClusterRoles::IS_CLUSTER_ROLE)] = "isCluster";
  roles[static_cast<int>(ClusterRoles::STATION_INDEX_ROLE)] = "stationIndex";
  return roles;
}