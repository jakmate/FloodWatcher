#pragma once
#include "FloodWarning.hpp"
#include <QAbstractListModel>
#include <QTimer>
#include <QVariantList>
#include <vector>

class FloodWarningModel : public QAbstractListModel {
    Q_OBJECT

  public:
    enum class FloodWarningRoles : uint16_t {
      DESCRIPTION_ROLE = Qt::UserRole + 1,
      SEVERITY_ROLE,
      SEVERITY_LEVEL_ROLE,
      EA_AREA_NAME_ROLE,
      POLYGON_PATH_ROLE,
      MESSAGE_ROLE
    };

    explicit FloodWarningModel(const std::vector<FloodWarning>& warnings,
                               QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void startAutoUpdate();
    Q_INVOKABLE void stopAutoUpdate();

  signals:
    void warningsUpdated(int count);

  private:
    void fetchWarnings();

    std::vector<FloodWarning> m_warnings;
    QTimer* m_updateTimer;

    static QVariantList getPolygonPath(const FloodWarning& warning);
    void updateWarnings(const std::vector<FloodWarning>& newWarnings);
    static int calculateNextUpdateMs();
};
