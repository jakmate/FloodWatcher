#include "HttpClient.hpp"
#include "MonitoringData.hpp"
#include "StationModel.hpp"
#include "WarningModel.hpp"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <future>
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

    MonitoringData monitoringData;

    // Fetch stations and warnings concurrently
    auto t1 = std::chrono::steady_clock::now();
    auto stationsFuture = std::async(std::launch::async, []() {
      return HttpClient::getInstance().fetchUrl(
          "https://environment.data.gov.uk/flood-monitoring/id/stations?status=Active");
    });

    auto warningsFuture = std::async(std::launch::async, []() {
      return HttpClient::getInstance().fetchUrl(
          "https://environment.data.gov.uk/flood-monitoring/id/floods");
    });

    auto stationsResponse = stationsFuture.get();
    auto warningsResponse = warningsFuture.get();
    std::cout << "fetch stations: " << msSince(t1) << " ms\n";

    // Stations
    if (!stationsResponse) {
      std::cerr << "Failed to fetch stations data" << '\n';
      return 1;
    }

    std::string readBuffer = *stationsResponse;

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

    auto t3 = std::chrono::steady_clock::now();
    StationModel model(monitoringData.getStations());
    std::cout << "get stations: " << msSince(t3) << " ms\n";

    // Warnings
    if (!warningsResponse) {
      std::cerr << "Failed to fetch warning data" << '\n';
      return 1;
    }

    readBuffer = *warningsResponse;

    auto t5 = std::chrono::steady_clock::now();
    try {
      json data = json::parse(readBuffer);
      monitoringData.parseWarnings(data);
      std::cout << "Found " << monitoringData.getWarnings().size() << " warnings\n";
    } catch (const json::parse_error& e) {
      std::cerr << "JSON Parse Error: " << e.what() << "\n";
      std::cerr << "Raw response:\n" << readBuffer << '\n';
      return 1;
    }
    std::cout << "parse warnings: " << msSince(t5) << " ms\n";

    // Polygons
    auto t6 = std::chrono::steady_clock::now();
    monitoringData.fetchAllPolygonsAsync();
    std::cout << "fetch all polygons async: " << msSince(t6) << " ms\n";

    auto t7 = std::chrono::steady_clock::now();
    WarningModel warningModel(monitoringData.getWarnings());
    std::cout << "get warnings: " << msSince(t7) << " ms\n";

    // Load QML
    auto t8 = std::chrono::steady_clock::now();
    QQmlApplicationEngine engine;
    engine.setInitialProperties({{"stationModel", QVariant::fromValue(&model)},
                                 {"warningModel", QVariant::fromValue(&warningModel)}});
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    std::cout << "load qml: " << msSince(t8) << " ms\n";

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
