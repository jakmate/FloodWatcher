#include "FloodMonitoringData.hpp"
#include "FloodWarningModel.hpp"
#include "HttpClient.hpp"
#include "StationModel.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>

static inline auto msSince(std::chrono::steady_clock::time_point t) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t)
      .count();
}

int main(int argc, char* argv[]) {
  auto t0 = std::chrono::steady_clock::now();
  std::cout << "startup: " << msSince(t0) << " ms\n";

  try {
    QGuiApplication app(argc, argv);

    // Fetch data
    FloodMonitoringData monitoringData;

    auto t1 = std::chrono::steady_clock::now();
    auto response = HttpClient::fetchUrl("https://environment.data.gov.uk/flood-monitoring/"
                                         "id/stations?status=Active");
    std::cout << "fetch stations: " << msSince(t1) << " ms\n";

    if (!response) {
      std::cerr << "Failed to fetch stations data" << '\n';
      return 1;
    }

    std::string readBuffer = *response;

    auto t2 = std::chrono::steady_clock::now();
    try {
      json data = json::parse(readBuffer);
      monitoringData.parseStations(data);
      std::cout << "Found " << monitoringData.getStations().size() << " stations\n";
    } catch (const json::parse_error& e) {
      std::cerr << "JSON Parse Error: " << e.what() << "\n";
      std::cerr << "Raw response:\n" << readBuffer << '\n';
      return 1;
    }
    std::cout << "parse stations: " << msSince(t2) << " ms\n";

    // Create model
    StationModel model(monitoringData.getStations());
    auto t3 = std::chrono::steady_clock::now();
    response = HttpClient::fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");
    std::cout << "fetch warnings: " << msSince(t3) << " ms\n";

    if (!response) {
      std::cerr << "Failed to fetch flood warning data" << '\n';
      return 1;
    }

    readBuffer = *response;

    auto t4 = std::chrono::steady_clock::now();
    try {
      json data = json::parse(readBuffer);
      monitoringData.parseFloodWarnings(data);
      std::cout << "Found " << monitoringData.getFloodWarnings().size() << " warnings\n";
    } catch (const json::parse_error& e) {
      std::cerr << "JSON Parse Error: " << e.what() << "\n";
      std::cerr << "Raw response:\n" << readBuffer << '\n';
      return 1;
    }
    std::cout << "parse warnings: " << msSince(t4) << " ms\n";

    auto t5 = std::chrono::steady_clock::now();
    monitoringData.fetchAllPolygonsAsync();
    std::cout << "fetch all polygons async: " << msSince(t5) << " ms\n";

    FloodWarningModel warningModel(monitoringData.getFloodWarnings());

    // Load QML
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"stationModel", QVariant::fromValue(&model)},
                                 {"floodWarningModel", QVariant::fromValue(&warningModel)}});
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Start auto-update after QML is loaded
    warningModel.startAutoUpdate();
    std::cout << "total: " << msSince(t0) << " ms\n";
#ifdef ENABLE_PROFILING_EXIT
    return 0;
#else
    return QGuiApplication::exec();
#endif
  } catch (const std::exception& e) {
    qCritical("Unhandled exception: %s", e.what());
    return EXIT_FAILURE;
  } catch (...) {
    qCritical("Unhandled unknown exception");
    return EXIT_FAILURE;
  }
}
