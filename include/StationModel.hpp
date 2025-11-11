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
      LATITUDE_ROLE = Qt::UserRole + 1,
      LONGITUDE_ROLE,
      LABEL_ROLE,
      TOWN_ROLE
    };

    explicit StationModel(const std::vector<Station>& stations, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap getStation(int index) const;
    Q_INVOKABLE QVariantList getMeasures(int index) const;
    Q_INVOKABLE bool fetchMeasures(int index);

  private:
    std::vector<Station> m_stations;
};
