#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>
#include "StationModel.hpp"
#include "FloodMonitoringData.hpp"
#include "HttpClient.hpp"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    
    // Fetch data
    FloodMonitoringData monitoringData;

    auto response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/floods");
    
    if (!response) {
        std::cerr << "Failed to fetch flood warning data" << std::endl;
        return 1;
    }
    
    std::string readBuffer = *response;

    /*try {
        json data = json::parse(readBuffer);
        monitoringData.parseFloodWarnings(data);
        std::cout << "Found " << monitoringData.getFloodWarnings().size() << " warnings\n";
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
        std::cerr << "Raw response:\n" << readBuffer << std::endl;
        return 1;
    }*/

    response = fetchUrl("https://environment.data.gov.uk/flood-monitoring/id/stations?status=Active");
    
    if (!response) {
        std::cerr << "Failed to fetch stations data" << std::endl;
        return 1;
    }
    
    readBuffer = *response;

    try {
        json data = json::parse(readBuffer);
        monitoringData.parseStations(data);
        std::cout << "Found " << monitoringData.getStations().size() << " stations\n";
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
        std::cerr << "Raw response:\n" << readBuffer << std::endl;
        return 1;
    }
    
    // Create model
    StationModel model(monitoringData.getStations());
    
    // Load QML
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("stationModel", &model);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    return app.exec();
}