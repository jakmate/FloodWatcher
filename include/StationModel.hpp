#pragma once
#include <QAbstractListModel>
#include <vector>
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
    ~StationModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariantMap getStation(int index) const;

private:
    std::vector<Station> m_stations;
};