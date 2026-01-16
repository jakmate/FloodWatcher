#pragma once
#include "Station.hpp"
#include <QAbstractListModel>
#include <cmath>
#include <memory>
#include <vector>

struct ClusterPoint {
    double lat;
    double lon;
    int stationIndex;
};

struct Cluster {
    double lat;
    double lon;
    int count;
    std::vector<int> stationIndices;
};

struct Bounds {
    double minLat;
    double maxLat;
    double minLon;
    double maxLon;
};

struct QuadrantStruct {
    double lat;
    double lon;
    double midLat;
    double midLon;
};

enum Quadrant { NW = 0, NE = 1, SW = 2, SE = 3 };

struct ClusterContext {
    double zoomLevel;
    double minDistanceMeters;
    double metersPerDegreeLat;
    double metersPerDegreeLon;
};

class QuadTreeNode {
  public:
    QuadTreeNode(const Bounds& bounds, int maxDepth);
    void insert(const ClusterPoint& point);
    std::vector<Cluster> getClusters(double zoomLevel, double minDistance) const;

  private:
    double m_minLat;
    double m_maxLat;
    double m_minLon;
    double m_maxLon;
    int m_depth;
    int m_maxDepth;

    std::vector<ClusterPoint> m_points;
    std::unique_ptr<QuadTreeNode> m_nw;
    std::unique_ptr<QuadTreeNode> m_ne;
    std::unique_ptr<QuadTreeNode> m_sw;
    std::unique_ptr<QuadTreeNode> m_se;
    QuadTreeNode* m_children[4] = {nullptr, nullptr, nullptr, nullptr};

    bool shouldSubdivide() const;
    void subdivide();
    Cluster aggregatePointsInline() const;
    Cluster aggregateSubtree() const;
    void getClustersImpl(const ClusterContext& ctx, std::vector<Cluster>& clusters) const;
};

struct ClusterItem {
    double lat;
    double lon;
    int count;
    bool isCluster;
    int stationIndex;
};

class ClusterModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit ClusterModel(QObject* parent = nullptr);

    enum class ClusterRoles {
      LATITUDE_ROLE = Qt::UserRole + 1,
      LONGITUDE_ROLE,
      COUNT_ROLE,
      IS_CLUSTER_ROLE,
      STATION_INDEX_ROLE
    };

    void setStations(const std::vector<Station>& stations);
    Q_INVOKABLE void updateClusters(double zoomLevel);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

  private:
    std::vector<Station> m_stations;
    std::unique_ptr<QuadTreeNode> m_quadTree;
    std::vector<ClusterItem> m_displayItems;

    void buildQuadTree();
    static double getMinDistanceForZoom(double zoomLevel);
};