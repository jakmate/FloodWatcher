#pragma once
#include <QAbstractListModel>
#include <QVariantMap>
#include <QVariantList>
#include "Station.hpp"

class StationModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum StationRoles
    {
        LatitudeRole = Qt::UserRole + 1,
        LongitudeRole,
        LabelRole,
        TownRole
    };

    explicit StationModel(const std::vector<Station> &stations, QObject *parent = nullptr);
    ~StationModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap getStation(int index) const;
    Q_INVOKABLE QVariantList getMeasures(int index) const;
    Q_INVOKABLE bool fetchMeasures(int index);

private:
    std::vector<Station> m_stations;
};