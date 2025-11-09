#pragma once
#include <QAbstractListModel>
#include <QVariantList>
#include <vector>
#include "FloodWarning.hpp"

class FloodWarningModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum FloodWarningRoles
    {
        DescriptionRole = Qt::UserRole + 1,
        SeverityRole,
        SeverityLevelRole,
        EaAreaNameRole,
        PolygonPathRole
    };

    explicit FloodWarningModel(const std::vector<FloodWarning> &warnings, QObject *parent = nullptr);
    ~FloodWarningModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<FloodWarning> m_warnings;
    QVariantList getPolygonPath(const FloodWarning &warning) const;
};