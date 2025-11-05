#include <iostream>

#include "GeometryTypes.hpp"

// Helper function to print a single coordinate
void printCoordinate(const Coordinate& coord) {
    std::cout << "(" << coord.first << ", " << coord.second << ")";
}

// Helper function to print a linear ring
void printLinearRing(const LinearRing& ring, const std::string& indent) {
    std::cout << indent << "Ring with " << ring.size() << " points:\n";
    for (size_t i = 0; i < ring.size(); ++i) {
        std::cout << indent << "  Point " << i << ": ";
        printCoordinate(ring[i]);
        std::cout << "\n";
    }
}

// Helper function to print a polygon
void printPolygon(const MyPolygon& polygon, const std::string& indent) {
    std::cout << indent << "Polygon with " << polygon.size() << " rings:\n";
    for (size_t i = 0; i < polygon.size(); ++i) {
        std::cout << indent << "Ring " << i << " ("
                  << (i == 0 ? "exterior" : "interior hole") << "):\n";
        printLinearRing(polygon[i], indent + "  ");
    }
}

// Helper function to print a MultiPolygon
void printMultiPolygon(const MultiPolygon& multiPolygon, const std::string& indent) {
    std::cout << indent << "MultiPolygon with " << multiPolygon.size() << " polygons:\n";
    for (size_t i = 0; i < multiPolygon.size(); ++i) {
        std::cout << indent << "Polygon " << i << ":\n";
        printPolygon(multiPolygon[i], indent + "  ");
    }
}