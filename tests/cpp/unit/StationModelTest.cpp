// tests/unit/StationModelTest.cpp
#include "StationModel.hpp"
#include "Station.hpp"
#include <QTest>
#include <simdjson.h>

class StationModelTest : public QObject {
    Q_OBJECT

  private slots:
    static void testRowCount();
    static void testRowCountWithValidParent();
    static void testDataReturnsCorrectValues();
    static void testDataReturnsEmptyForInvalidIndex();
    static void testDataReturnsEmptyForUnknownRole();
    static void testRoleNames();
};

void StationModelTest::testRowCount() {
  std::vector<Station> stations;

  std::string json1 = R"({"label": "1", "RLOIid": "2321"})";
  std::string json2 = R"({"label": "2", "RLOIid": "3221"})";

  simdjson::dom::parser parser1;
  simdjson::dom::parser parser2;
  simdjson::dom::element s1;
  simdjson::dom::element s2;
  auto error1 = parser1.parse(json1).get(s1);
  QVERIFY(error1 == 0U);
  auto error2 = parser2.parse(json2).get(s2);
  QVERIFY(error2 == 0U);

  stations.push_back(Station::fromJson(s1));
  stations.push_back(Station::fromJson(s2));

  StationModel model(stations);
  QCOMPARE(model.rowCount(), 2);
}

void StationModelTest::testRowCountWithValidParent() {
  std::string jsonStr = R"({"label": "1", "RLOIid": "2321"})";
  simdjson::dom::parser parser;
  simdjson::dom::element s;
  auto error = parser.parse(jsonStr).get(s);
  QVERIFY(error == 0U);

  StationModel model({Station::fromJson(s)});

  // Create a valid parent index
  QModelIndex validParent = model.index(0, 0);

  // Should return 0 for valid parent
  QCOMPARE(model.rowCount(validParent), 0);
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
void StationModelTest::testDataReturnsCorrectValues() {
  std::string stationJson = R"({
    "label": "L1",
    "town": "Ttown",
    "lat": 51.5074,
    "long": -0.1278,
    "RLOIid": "R123",
    "catchmentName": "Catchment X",
    "dateOpened": "2020-01-01",
    "riverName": "River X",
    "notation": "NT-1"
  })";

  std::string measureJson = R"({
    "parameter": "water_level",
    "parameterName": "Water level",
    "qualifier": "above",
    "latestReading": {
      "value": 3.14
    },
    "unitName": "m"
  })";

  simdjson::dom::parser parser1;
  simdjson::dom::parser parser2;
  simdjson::dom::element s;
  simdjson::dom::element mJson;
  auto error1 = parser1.parse(stationJson).get(s);
  QVERIFY(error1 == 0U);
  auto error2 = parser2.parse(measureJson).get(mJson);
  QVERIFY(error2 == 0U);

  Station station = Station::fromJson(s);
  Measure measure = Measure::fromJson(mJson);
  std::vector<Measure> measures{measure};
  station.setMeasures(measures);
  std::vector<Station> stations{station};
  StationModel model(stations);

  QModelIndex idx = model.index(0, 0);

  QCOMPARE(model.data(idx, Qt::UserRole + 1).toString(), QString("L1"));
  QCOMPARE(model.data(idx, Qt::UserRole + 2).toString(), QString("Ttown"));
  QCOMPARE(model.data(idx, Qt::UserRole + 3).toDouble(), 51.5074);
  QCOMPARE(model.data(idx, Qt::UserRole + 4).toDouble(), -0.1278);
  QCOMPARE(model.data(idx, Qt::UserRole + 5).toString(), QString("R123"));
  QCOMPARE(model.data(idx, Qt::UserRole + 6).toString(), QString("Catchment X"));
  QCOMPARE(model.data(idx, Qt::UserRole + 7).toString(), QString("2020-01-01"));
  QCOMPARE(model.data(idx, Qt::UserRole + 8).toString(), QString("River X"));
  QCOMPARE(model.data(idx, Qt::UserRole + 9).toString(), QString("NT-1"));

  QVariant measuresVar = model.data(idx, Qt::UserRole + 10);
  QVERIFY(measuresVar.isValid());
  QVariantList measuresList = measuresVar.toList();
  QCOMPARE(measuresList.size(), 1);

  QVariantMap m = measuresList[0].toMap();
  QCOMPARE(m["parameter"].toString(), QString("water_level"));
  QCOMPARE(m["parameterName"].toString(), QString("Water level"));
  QCOMPARE(m["qualifier"].toString(), QString("above"));
  QCOMPARE(m["unitName"].toString(), QString("m"));
  QCOMPARE(m["latestReading"].toDouble(), 3.14);
}
// NOLINTEND(readability-function-cognitive-complexity)

void StationModelTest::testDataReturnsEmptyForInvalidIndex() {
  StationModel model({});
  QVERIFY(!model.data(QModelIndex(), Qt::UserRole).isValid());
  QVERIFY(!model.data(model.index(10, 0), Qt::UserRole).isValid());
}

void StationModelTest::testDataReturnsEmptyForUnknownRole() {
  std::string jsonStr = R"({"label": "test", "RLOIid": "1234"})";
  simdjson::dom::parser parser;
  simdjson::dom::element s;
  auto error = parser.parse(jsonStr).get(s);
  QVERIFY(error == 0U);

  StationModel model({Station::fromJson(s)});
  QModelIndex idx = model.index(0, 0);

  // Test with a role that's not in WarningRoles enum
  QVERIFY(!model.data(idx, Qt::DisplayRole).isValid());
  QVERIFY(!model.data(idx, 9999).isValid());
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
void StationModelTest::testRoleNames() {
  StationModel model({});
  auto roles = model.roleNames();

  QCOMPARE(roles[Qt::UserRole + 1], QByteArray("label"));
  QCOMPARE(roles[Qt::UserRole + 2], QByteArray("town"));
  QCOMPARE(roles[Qt::UserRole + 3], QByteArray("latitude"));
  QCOMPARE(roles[Qt::UserRole + 4], QByteArray("longitude"));
  QCOMPARE(roles[Qt::UserRole + 5], QByteArray("RLOIid"));
  QCOMPARE(roles[Qt::UserRole + 6], QByteArray("catchmentName"));
  QCOMPARE(roles[Qt::UserRole + 7], QByteArray("dateOpened"));
  QCOMPARE(roles[Qt::UserRole + 8], QByteArray("riverName"));
  QCOMPARE(roles[Qt::UserRole + 9], QByteArray("notation"));
  QCOMPARE(roles[Qt::UserRole + 10], QByteArray("measures"));
}
// NOLINTEND(readability-function-cognitive-complexity)

QTEST_MAIN(StationModelTest)
#include "StationModelTest.moc"