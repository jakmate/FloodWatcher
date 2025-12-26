#pragma once
#include "Station.hpp"
#include <QAbstractListModel>
#include <QGeoCoordinate>
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

class QuadTreeNode {
  public:
    QuadTreeNode(const Bounds& bounds, int maxDepth);

    void insert(const ClusterPoint& point);
    std::vector<Cluster> getClusters(double zoomLevel, double minDistance) const;

  private:
    double m_minLat, m_maxLat, m_minLon, m_maxLon;
    int m_depth;
    int m_maxDepth;
    std::vector<ClusterPoint> m_points;
    std::unique_ptr<QuadTreeNode> m_nw, m_ne, m_sw, m_se;

    bool shouldSubdivide() const;
    void subdivide();
    Cluster aggregatePoints() const;
    void collectAllPoints(std::vector<ClusterPoint>& points) const;
};

class ClusterModel : public QAbstractListModel {
    Q_OBJECT

  public:
    enum class ClusterRoles : uint16_t {
      LATITUDE_ROLE = Qt::UserRole + 1,
      LONGITUDE_ROLE,
      COUNT_ROLE,
      IS_CLUSTER_ROLE,
      STATION_INDEX_ROLE
    };

    explicit ClusterModel(QObject* parent = nullptr);

    void setStations(const std::vector<Station>& stations);
    Q_INVOKABLE void updateClusters(double zoomLevel);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

  private:
    struct ClusterItem {
        double lat;
        double lon;
        int count;
        bool isCluster;
        int stationIndex; // -1 if cluster, otherwise station index
    };

    std::vector<Station> m_stations;
    std::unique_ptr<QuadTreeNode> m_quadTree;
    std::vector<ClusterItem> m_displayItems;

    void buildQuadTree();
    static double getMinDistanceForZoom(double zoomLevel);
};