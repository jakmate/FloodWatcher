// tests/unit/WarningModelTest.cpp
#include "WarningModel.hpp"
#include "Warning.hpp"
#include <QGeoCoordinate>
#include <QSignalSpy>
#include <QTest>
#include <simdjson.h>

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

  simdjson::dom::parser parser;
  simdjson::dom::element w1;
  simdjson::dom::element w2;
  std::string json1 = R"({"floodAreaID": "1", "severityLevel": 2})";
  std::string json2 = R"({"floodAreaID": "2", "severityLevel": 1})";
  auto error1 = parser.parse(json1).get(w1);
  QVERIFY(error1 == 0U);
  auto error2 = parser.parse(json2).get(w2);
  QVERIFY(error2 == 0U);

  warnings.push_back(Warning::fromJson(w1));
  warnings.push_back(Warning::fromJson(w2));

  WarningModel model(warnings);
  QCOMPARE(model.rowCount(), 2);
}

void WarningModelTest::testRowCountWithValidParent() {
  simdjson::dom::parser parser;
  simdjson::dom::element w;
  std::string jsonStr = R"({"floodAreaID": "1", "severityLevel": 2})";
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

  WarningModel model({Warning::fromJson(w)});

  // Create a valid parent index
  QModelIndex validParent = model.index(0, 0);

  // Should return 0 for valid parent
  QCOMPARE(model.rowCount(validParent), 0);
}

void WarningModelTest::testDataReturnsCorrectValues() {
  std::string jsonStr = R"({
    "floodAreaID": "test",
    "description": "Test Desc",
    "severity": "Moderate",
    "severityLevel": 2,
    "eaAreaName": "Test Area",
    "message": "Test Message"
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element w;
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

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
  simdjson::dom::parser parser;
  simdjson::dom::element w;
  std::string jsonStr = R"({"floodAreaID": "test", "severityLevel": 2})";
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

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
  std::string json1 = R"({"floodAreaID": "1", "severityLevel": 3})";
  std::string json2 = R"({"floodAreaID": "2", "severityLevel": 1})";
  std::string json3 = R"({"floodAreaID": "3", "severityLevel": 2})";

  simdjson::dom::parser parser1;
  simdjson::dom::parser parser2;
  simdjson::dom::parser parser3;
  simdjson::dom::element w1;
  simdjson::dom::element w2;
  simdjson::dom::element w3;

  auto error1 = parser1.parse(json1).get(w1);
  QVERIFY(error1 == 0U);
  auto error2 = parser2.parse(json2).get(w2);
  QVERIFY(error2 == 0U);
  auto error3 = parser3.parse(json3).get(w3);
  QVERIFY(error3 == 0U);

  std::vector<Warning> warnings{Warning::fromJson(w1), Warning::fromJson(w2),
                                Warning::fromJson(w3)};

  WarningModel model(warnings);

  QCOMPARE(model.data(model.index(0, 0), Qt::UserRole + 3).toInt(), 1);
  QCOMPARE(model.data(model.index(1, 0), Qt::UserRole + 3).toInt(), 2);
  QCOMPARE(model.data(model.index(2, 0), Qt::UserRole + 3).toInt(), 3);
}

void WarningModelTest::testGetPolygonPath() {
  std::string jsonStr = R"({
    "floodAreaID": "test",
    "severityLevel": 1,
    "floodArea": {"polygon": "http://test.com"}
  })";

  simdjson::dom::parser parser;
  simdjson::dom::element w;
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

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
  simdjson::dom::parser parser;
  simdjson::dom::element w;
  std::string jsonStr = R"({"floodAreaID": "test", "severityLevel": 1})";
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

  WarningModel model({Warning::fromJson(w)});
  auto path = model.data(model.index(0, 0), Qt::UserRole + 5).toList();
  QVERIFY(path.isEmpty());
}

void WarningModelTest::testUpdateWarningsWithNewData() {
  simdjson::dom::parser parser;
  simdjson::dom::element w1;
  std::string json1 = R"({"floodAreaID": "1", "description": "Old", "severityLevel": 2})";
  auto error1 = parser.parse(json1).get(w1);
  QVERIFY(error1 == 0U);

  WarningModel model({Warning::fromJson(w1)});
  QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);

  simdjson::dom::element w2;
  std::string json2 = R"({"floodAreaID": "1", "description": "New", "severityLevel": 2})";
  auto error2 = parser.parse(json2).get(w2);
  QVERIFY(error2 == 0U);
  model.updateWarnings({Warning::fromJson(w2)});

  QCOMPARE(model.rowCount(), 1);
  QCOMPARE(dataChangedSpy.count(), 1);
  QCOMPARE(model.data(model.index(0, 0), Qt::UserRole + 1).toString(), QString("New"));
}

void WarningModelTest::testUpdateWarningsWithIdenticalData() {
  simdjson::dom::parser parser;
  simdjson::dom::element w;
  std::string jsonStr = R"({"floodAreaID": "1", "description": "Same", "severityLevel": 2})";
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

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
  simdjson::dom::parser parser;
  simdjson::dom::element w1;
  std::string json1 = R"({"floodAreaID": "1", "severityLevel": 2})";
  auto error1 = parser.parse(json1).get(w1);
  QVERIFY(error1 == 0U);

  WarningModel model({Warning::fromJson(w1)});
  QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);

  simdjson::dom::element w2;
  simdjson::dom::element w3;
  std::string json2 = R"({"floodAreaID": "2", "severityLevel": 1})";
  std::string json3 = R"({"floodAreaID": "3", "severityLevel": 3})";
  auto error2 = parser.parse(json2).get(w2);
  QVERIFY(error2 == 0U);
  auto error3 = parser.parse(json3).get(w3);
  QVERIFY(error3 == 0U);
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
  simdjson::dom::parser parser;
  simdjson::dom::element w;
  std::string jsonStr = R"({"floodAreaID": "1", "severityLevel": 2})";
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

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
  simdjson::dom::parser parser;
  simdjson::dom::element w;
  std::string jsonStr = R"({"floodAreaID": "1", "severityLevel": 2})";
  auto error = parser.parse(jsonStr).get(w);
  QVERIFY(error == 0U);

  WarningModel model({Warning::fromJson(w)});
  model.startAutoUpdate();

  auto* timer = model.findChild<QTimer*>();
  QVERIFY(timer->isActive());

  model.stopAutoUpdate();
  QVERIFY(!timer->isActive());
}

QTEST_MAIN(WarningModelTest)
#include "WarningModelTest.moc"