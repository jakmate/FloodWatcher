#include "FloodMonitoringData.hpp"
#include "FloodWarningModel.hpp"
#include "HttpClient.hpp"
#include "StationModel.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>

int main(int argc, char* argv[]) {
  try {
    QGuiApplication app(argc, argv);

    // Fetch data
    FloodMonitoringData monitoringData;

    auto response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/"
                             "id/stations?status=Active");

    if (!response) {
      std::cerr << "Failed to fetch stations data" << '\n';
      return 1;
    }

    std::string readBuffer = *response;

    try {
      json data = json::parse(readBuffer);
      monitoringData.parseStations(data);
      std::cout << "Found " << monitoringData.getStations().size() << " stations\n";
    } catch (const json::parse_error& e) {
      std::cerr << "JSON Parse Error: " << e.what() << "\n";
      std::cerr << "Raw response:\n" << readBuffer << '\n';
      return 1;
    }

    // Create model
    StationModel model(monitoringData.getStations());

    response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");

    if (!response) {
      std::cerr << "Failed to fetch flood warning data" << '\n';
      return 1;
    }

    readBuffer = *response;

    try {
      json data = json::parse(readBuffer);
      monitoringData.parseFloodWarnings(data);
      std::cout << "Found " << monitoringData.getFloodWarnings().size() << " warnings\n";
    } catch (const json::parse_error& e) {
      std::cerr << "JSON Parse Error: " << e.what() << "\n";
      std::cerr << "Raw response:\n" << readBuffer << '\n';
      return 1;
    }

    FloodWarningModel warningModel(monitoringData.getFloodWarnings());

    // Load QML
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("stationModel", &model);
    engine.rootContext()->setContextProperty("floodWarningModel", &warningModel);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Start auto-update after QML is loaded
    warningModel.startAutoUpdate();

    return QGuiApplication::exec();
  } catch (const std::exception& e) {
    qCritical("Unhandled exception: %s", e.what());
    return EXIT_FAILURE;
  } catch (...) {
    qCritical("Unhandled unknown exception");
    return EXIT_FAILURE;
  }
}
