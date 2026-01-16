#include "StationCluster.hpp"

// QuadTreeNode implementation
QuadTreeNode::QuadTreeNode(const Bounds& bounds, int maxDepth)
    : m_minLat(bounds.minLat), m_maxLat(bounds.maxLat), m_minLon(bounds.minLon),
      m_maxLon(bounds.maxLon), m_depth(0), m_maxDepth(maxDepth) {}

Quadrant getQuadrant(const QuadrantStruct& quadrant) {
  if (quadrant.lat >= quadrant.midLat) {
    return (quadrant.lon >= quadrant.midLon) ? NE : NW;
  }
  return (quadrant.lon >= quadrant.midLon) ? SE : SW;
}

void QuadTreeNode::insert(const ClusterPoint& point) {
  if (point.lat < m_minLat || point.lat > m_maxLat || point.lon < m_minLon ||
      point.lon > m_maxLon) {
    return;
  }

  if (m_nw != nullptr) {
    // Already subdivided
    double midLat = (m_minLat + m_maxLat) / 2.0;
    double midLon = (m_minLon + m_maxLon) / 2.0;

    Quadrant quad = getQuadrant(QuadrantStruct{point.lat, point.lon, midLat, midLon});
    switch (quad) {
      case NW:
        m_nw->insert(point);
        break;
      case NE:
        m_ne->insert(point);
        break;
      case SW:
        m_sw->insert(point);
        break;
      case SE:
        m_se->insert(point);
        break;
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

  m_nw = std::make_unique<QuadTreeNode>(Bounds{midLat, m_maxLat, m_minLon, midLon}, m_maxDepth);
  m_ne = std::make_unique<QuadTreeNode>(Bounds{midLat, m_maxLat, midLon, m_maxLon}, m_maxDepth);
  m_sw = std::make_unique<QuadTreeNode>(Bounds{m_minLat, midLat, m_minLon, midLon}, m_maxDepth);
  m_se = std::make_unique<QuadTreeNode>(Bounds{m_minLat, midLat, midLon, m_maxLon}, m_maxDepth);

  int newDepth = m_depth + 1;
  m_nw->m_depth = newDepth;
  m_ne->m_depth = newDepth;
  m_sw->m_depth = newDepth;
  m_se->m_depth = newDepth;

  m_children[NW] = m_nw.get();
  m_children[NE] = m_ne.get();
  m_children[SW] = m_sw.get();
  m_children[SE] = m_se.get();

  // Direct insert to avoid recursion overhead
  for (const auto& point : m_points) {
    Quadrant quad = getQuadrant(QuadrantStruct{point.lat, point.lon, midLat, midLon});
    switch (quad) {
      case NW:
        m_nw->m_points.push_back(point);
        break;
      case NE:
        m_ne->m_points.push_back(point);
        break;
      case SW:
        m_sw->m_points.push_back(point);
        break;
      case SE:
        m_se->m_points.push_back(point);
        break;
    }
  }

  m_points.clear();
  m_points.shrink_to_fit();
}

Cluster QuadTreeNode::aggregatePointsInline() const {
  Cluster cluster;
  cluster.lat = 0.0;
  cluster.lon = 0.0;
  cluster.count = static_cast<int>(m_points.size());
  cluster.stationIndices.reserve(m_points.size());

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

Cluster QuadTreeNode::aggregateSubtree() const {
  Cluster cluster;
  cluster.lat = 0.0;
  cluster.lon = 0.0;
  cluster.count = 0;

  std::function<void(const QuadTreeNode*)> aggregate = [&](const QuadTreeNode* node) {
    if (node->m_nw == nullptr) {
      for (const auto& point : node->m_points) {
        cluster.lat += point.lat;
        cluster.lon += point.lon;
        cluster.stationIndices.push_back(point.stationIndex);
        cluster.count++;
      }
    } else {
      aggregate(node->m_nw.get());
      aggregate(node->m_ne.get());
      aggregate(node->m_sw.get());
      aggregate(node->m_se.get());
    }
  };

  aggregate(this);

  if (cluster.count > 0) {
    cluster.lat /= cluster.count;
    cluster.lon /= cluster.count;
  }

  return cluster;
}

std::vector<Cluster> QuadTreeNode::getClusters(double zoomLevel, double minDistance) const {
  double avgLat = (m_minLat + m_maxLat) / 2.0;
  ClusterContext ctx{zoomLevel, minDistance * 111000.0, // Convert to meters
                     111000.0, 111000.0 * std::cos(avgLat * M_PI / 180.0)};

  std::vector<Cluster> clusters;
  getClustersImpl(ctx, clusters);
  return clusters;
}

void QuadTreeNode::getClustersImpl(const ClusterContext& ctx,
                                   std::vector<Cluster>& clusters) const {
  double latDiff = m_maxLat - m_minLat;
  double lonDiff = m_maxLon - m_minLon;

  double nodeWidthMeters = lonDiff * ctx.metersPerDegreeLon;
  double nodeHeightMeters = latDiff * ctx.metersPerDegreeLat;
  double nodeSizeMeters =
      std::sqrt((nodeWidthMeters * nodeWidthMeters) + (nodeHeightMeters * nodeHeightMeters));

  // Leaf node
  if (m_nw == nullptr) {
    if (ctx.zoomLevel >= 10.0 || m_points.size() <= 1 ||
        nodeSizeMeters < ctx.minDistanceMeters * 0.5) {
      // Show individual points
      for (const auto& point : m_points) {
        Cluster c;
        c.lat = point.lat;
        c.lon = point.lon;
        c.count = 1;
        c.stationIndices.push_back(point.stationIndex);
        clusters.push_back(c);
      }
    } else {
      // Cluster all points in leaf
      clusters.push_back(aggregatePointsInline());
    }
    return;
  }

  // Branch node
  if (ctx.zoomLevel < 8.0) {
    int totalPoints = 0;
    std::function<int(const QuadTreeNode*)> countPoints = [&](const QuadTreeNode* node) {
      if (node->m_nw == nullptr) {
        return static_cast<int>(node->m_points.size());
      }
      return countPoints(node->m_nw.get()) + countPoints(node->m_ne.get()) +
             countPoints(node->m_sw.get()) + countPoints(node->m_se.get());
    };
    totalPoints = countPoints(this);

    if (totalPoints > 100) {
      // Recurse into children
      m_nw->getClustersImpl(ctx, clusters);
      m_ne->getClustersImpl(ctx, clusters);
      m_sw->getClustersImpl(ctx, clusters);
      m_se->getClustersImpl(ctx, clusters);
    } else {
      // Aggregate entire subtree
      clusters.push_back(aggregateSubtree());
    }
  } else {
    // Normal recursion
    m_nw->getClustersImpl(ctx, clusters);
    m_ne->getClustersImpl(ctx, clusters);
    m_sw->getClustersImpl(ctx, clusters);
    m_se->getClustersImpl(ctx, clusters);
  }
}

// ClusterModel implementation
ClusterModel::ClusterModel(QObject* parent) : QAbstractListModel(parent) {}

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
  double minLat = 90.0;
  double maxLat = -90.0;
  double minLon = 180.0;
  double maxLon = -180.0;

  for (const auto& station : m_stations) {
    double lat = station.getLat();
    double lon = station.getLon();
    minLat = std::min(minLat, lat);
    maxLat = std::max(maxLat, lat);
    minLon = std::min(minLon, lon);
    maxLon = std::max(maxLon, lon);
  }

  double latPadding = std::max(0.1, (maxLat - minLat) * 0.1);
  double lonPadding = std::max(0.1, (maxLon - minLon) * 0.1);

  minLat -= latPadding;
  maxLat += latPadding;
  minLon -= lonPadding;
  maxLon += lonPadding;

  m_quadTree = std::make_unique<QuadTreeNode>(Bounds{minLat, maxLat, minLon, maxLon}, 10);

  // Insert stations with validation
  for (size_t i = 0; i < m_stations.size(); ++i) {
    ClusterPoint point = {};
    point.lat = m_stations[i].getLat();
    point.lon = m_stations[i].getLon();
    point.stationIndex = static_cast<int>(i);

    // Only insert if within bounds
    if (point.lat >= 49.0 && point.lat <= 61.0 && point.lon >= -8.0 && point.lon <= 2.0) {
      m_quadTree->insert(point);
    }
  }
}

double ClusterModel::getMinDistanceForZoom(double zoomLevel) {
  if (zoomLevel >= 14.0) {
    return 0.0005; // ~50m
  }
  if (zoomLevel >= 12.0) {
    return 0.002; // ~200m
  }
  if (zoomLevel >= 10.0) {
    return 0.005; // ~500m
  }
  if (zoomLevel >= 9.0) {
    return 0.01; // ~1km
  }
  if (zoomLevel >= 8.0) {
    return 0.02; // ~2km
  }
  if (zoomLevel >= 7.0) {
    return 0.04; // ~4km
  }
  return 0.08; // ~8km for zoom < 7
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
    ClusterItem item = {};
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
