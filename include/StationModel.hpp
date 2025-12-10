#pragma once
#include "Station.hpp"
#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>
#include <vector>

class StationModel : public QAbstractListModel {
    Q_OBJECT

  public:
    enum class StationRoles : uint16_t {
      LABEL_ROLE = Qt::UserRole + 1,
      TOWN_ROLE,
      LATITUDE_ROLE,
      LONGITUDE_ROLE,
      RLOI_ROLE,
      CATCHMENT_ROLE,
      DATE_ROLE,
      RIVER_ROLE,
      NOTATION_ROLE,
      MEASURES_ROLE
    };

    explicit StationModel(const std::vector<Station>& stations, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool fetchMeasures(int index);

  private:
    std::vector<Station> m_stations;
};
