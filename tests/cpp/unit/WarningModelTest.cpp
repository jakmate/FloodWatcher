// tests/unit/WarningModelTest.cpp
#include "WarningModel.hpp"
#include "Warning.hpp"
#include <QGeoCoordinate>
#include <QSignalSpy>
#include <QTest>

class WarningModelTest : public QObject {
    Q_OBJECT

  private slots:
    static void testRowCount();
    static void testRowCountWithValidParent();
    static void testDataReturnsCorrectValues();
    static void testDataReturnsEmptyForInvalidIndex();
    static void testDataReturnsEmptyForUnknownRole();
    static void testRoleNames();
    static void testSortsBySeverityLevel();
    static void testGetPolygonPath();
    static void testGetPolygonPathEmpty();
    static void testUpdateWarningsWithNewData();
    static void testUpdateWarningsWithIdenticalData();
    static void testUpdateWarningsWithDifferentSize();
    static void testCalculateNextUpdateMs();
    static void testStartAutoUpdate();
    static void testStopAutoUpdate();
};

void WarningModelTest::testRowCount() {
  std::vector<Warning> warnings;
  json w1 = R"({"floodAreaID": "1", "severityLevel": 2})"_json;
  json w2 = R"({"floodAreaID": "2", "severityLevel": 1})"_json;
  warnings.push_back(Warning::fromJson(w1));
  warnings.push_back(Warning::fromJson(w2));

  WarningModel model(warnings);
  QCOMPARE(model.rowCount(), 2);
}

void WarningModelTest::testRowCountWithValidParent() {
  json w = R"({"floodAreaID": "1", "severityLevel": 2})"_json;
  WarningModel model({Warning::fromJson(w)});

  // Create a valid parent index
  QModelIndex validParent = model.index(0, 0);

  // Should return 0 for valid parent (flat list model)
  QCOMPARE(model.rowCount(validParent), 0);
}

void WarningModelTest::testDataReturnsCorrectValues() {
  json w = R"({
        "floodAreaID": "test",
        "description": "Test Desc",
        "severity": "Moderate",
        "severityLevel": 2,
        "eaAreaName": "Test Area",
        "message": "Test Message"
    })"_json;

  std::vector<Warning> warnings{Warning::fromJson(w)};
  WarningModel model(warnings);

  QModelIndex idx = model.index(0, 0);

  QCOMPARE(model.data(idx, Qt::UserRole + 1).toString(), QString("Test Desc"));
  QCOMPARE(model.data(idx, Qt::UserRole + 2).toString(), QString("Moderate"));
  QCOMPARE(model.data(idx, Qt::UserRole + 3).toInt(), 2);
  QCOMPARE(model.data(idx, Qt::UserRole + 4).toString(), QString("Test Area"));
  QCOMPARE(model.data(idx, Qt::UserRole + 6).toString(), QString("Test Message"));
}

void WarningModelTest::testDataReturnsEmptyForInvalidIndex() {
  WarningModel model({});

  QVERIFY(!model.data(QModelIndex(), Qt::UserRole).isValid());
  QVERIFY(!model.data(model.index(10, 0), Qt::UserRole).isValid());
}

void WarningModelTest::testDataReturnsEmptyForUnknownRole() {
  json w = R"({"floodAreaID": "test", "severityLevel": 2})"_json;
  WarningModel model({Warning::fromJson(w)});

  QModelIndex idx = model.index(0, 0);

  // Test with a role that's not in WarningRoles enum
  QVERIFY(!model.data(idx, Qt::DisplayRole).isValid());
  QVERIFY(!model.data(idx, 9999).isValid());
}

void WarningModelTest::testRoleNames() {
  WarningModel model({});
  auto roles = model.roleNames();

  QCOMPARE(roles[Qt::UserRole + 1], QByteArray("description"));
  QCOMPARE(roles[Qt::UserRole + 2], QByteArray("severity"));
  QCOMPARE(roles[Qt::UserRole + 3], QByteArray("severityLevel"));
  QCOMPARE(roles[Qt::UserRole + 4], QByteArray("eaAreaName"));
  QCOMPARE(roles[Qt::UserRole + 5], QByteArray("polygonPath"));
  QCOMPARE(roles[Qt::UserRole + 6], QByteArray("message"));
}

void WarningModelTest::testSortsBySeverityLevel() {
  json w1 = R"({"floodAreaID": "1", "severityLevel": 3})"_json;
  json w2 = R"({"floodAreaID": "2", "severityLevel": 1})"_json;
  json w3 = R"({"floodAreaID": "3", "severityLevel": 2})"_json;

  std::vector<Warning> warnings{Warning::fromJson(w1), Warning::fromJson(w2),
                                Warning::fromJson(w3)};

  WarningModel model(warnings);

  QCOMPARE(model.data(model.index(0, 0), Qt::UserRole + 3).toInt(), 1);
  QCOMPARE(model.data(model.index(1, 0), Qt::UserRole + 3).toInt(), 2);
  QCOMPARE(model.data(model.index(2, 0), Qt::UserRole + 3).toInt(), 3);
}

void WarningModelTest::testGetPolygonPath() {
  json w = R"({
        "floodAreaID": "test",
        "severityLevel": 1,
        "floodArea": {"polygon": "http://test.com"}
    })"_json;

  auto warning = Warning::fromJson(w);

  MultiPolygon mp = {{{{0.0, 51.0}, {1.0, 51.0}, {1.0, 52.0}, {0.0, 51.0}}}};
  warning.setFloodAreaPolygon(mp);

  WarningModel model({warning});
  auto path = model.data(model.index(0, 0), Qt::UserRole + 5).toList();

  QCOMPARE(path.size(), 4);
  auto coord = path[0].value<QGeoCoordinate>();
  QCOMPARE(coord.latitude(), 51.0);
  QCOMPARE(coord.longitude(), 0.0);
}

void WarningModelTest::testGetPolygonPathEmpty() {
  json w = R"({"floodAreaID": "test", "severityLevel": 1})"_json;
  WarningModel model({Warning::fromJson(w)});

  auto path = model.data(model.index(0, 0), Qt::UserRole + 5).toList();
  QVERIFY(path.isEmpty());
}

void WarningModelTest::testUpdateWarningsWithNewData() {
  json w1 = R"({"floodAreaID": "1", "description": "Old", "severityLevel": 2})"_json;
  WarningModel model({Warning::fromJson(w1)});

  QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);

  json w2 = R"({"floodAreaID": "1", "description": "New", "severityLevel": 2})"_json;
  model.updateWarnings({Warning::fromJson(w2)});

  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(dataChangedSpy.count(), 1);
  QCOMPARE(model.data(model.index(0, 0), Qt::UserRole + 1).toString(), QString("New"));
}

void WarningModelTest::testUpdateWarningsWithIdenticalData() {
  json w = R"({"floodAreaID": "1", "description": "Same", "severityLevel": 2})"_json;
  WarningModel model({Warning::fromJson(w)});

  QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);
  QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);

  // Update with identical data
  model.updateWarnings({Warning::fromJson(w)});

  // Should not trigger any signals
  QCOMPARE(dataChangedSpy.count(), 0);
  QCOMPARE(resetSpy.count(), 0);
}

void WarningModelTest::testUpdateWarningsWithDifferentSize() {
  json w1 = R"({"floodAreaID": "1", "severityLevel": 2})"_json;
  WarningModel model({Warning::fromJson(w1)});

  QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);

  json w2 = R"({"floodAreaID": "2", "severityLevel": 1})"_json;
  json w3 = R"({"floodAreaID": "3", "severityLevel": 3})"_json;
  model.updateWarnings({Warning::fromJson(w2), Warning::fromJson(w3)});

  QCOMPARE(model.rowCount(), 2);
  QCOMPARE(resetSpy.count(), 1);
}

void WarningModelTest::testCalculateNextUpdateMs() {
  int delayMs = WarningModel::calculateNextUpdateMs();

  // Should be between 0 and 15 minutes
  QVERIFY(delayMs > 0);
  QVERIFY(delayMs <= 15 * 60 * 1000);
}

void WarningModelTest::testStartAutoUpdate() {
  json w = R"({"floodAreaID": "1", "severityLevel": 2})"_json;
  WarningModel model({Warning::fromJson(w)});

  model.startAutoUpdate();

  auto* timer = model.findChild<QTimer*>();
  if (timer == nullptr) {
    QFAIL("Timer not found");
    return;
  }

  QVERIFY(timer->isActive());
  QVERIFY(timer->isSingleShot());

  model.stopAutoUpdate();
}

void WarningModelTest::testStopAutoUpdate() {
  json w = R"({"floodAreaID": "1", "severityLevel": 2})"_json;
  WarningModel model({Warning::fromJson(w)});

  model.startAutoUpdate();
  auto* timer = model.findChild<QTimer*>();
  QVERIFY(timer->isActive());

  model.stopAutoUpdate();
  QVERIFY(!timer->isActive());
}

QTEST_MAIN(WarningModelTest)
#include "WarningModelTest.moc"