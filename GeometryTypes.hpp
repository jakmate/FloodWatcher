#pragma once
#include <utility>
#include <vector>

// Single coordinate pair (longitude, latitude)
using Coordinate = std::pair<double, double>;

// A linear ring (closed polygon ring - first and last points are the same)
using LinearRing = std::vector<Coordinate>;

// A polygon can have multiple rings (exterior ring + interior holes)
using MyPolygon = std::vector<LinearRing>;

// MultiPolygon for complex flood areas with multiple disconnected polygons
using MultiPolygon = std::vector<MyPolygon>;

void printCoordinate(const Coordinate& coord);
void printLinearRing(const LinearRing& ring, const std::string& indent = "    ");
void printPolygon(const MyPolygon& polygon, const std::string& indent = "  ");
void printMultiPolygon(const MultiPolygon& multiPolygon, const std::string& indent = "");
