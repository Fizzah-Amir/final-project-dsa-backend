#ifndef ROUTESERVICE_H
#define ROUTESERVICE_H

#include "../data_structures/Flight_graph.h"
#include "../data_structures/linked_list.h"
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <iostream>
#include<iomanip>

struct RouteOption {
    std::vector<std::string> cities;
    std::vector<std::string> flights;
    int totalDistance;
    double totalPrice;
    int totalTime;
    
    RouteOption() : totalDistance(0), totalPrice(0.0), totalTime(0) {}
    
    bool isValid() const {
        return !cities.empty() && !flights.empty();
    }
    
    void print() const {
        std::cout << "\n=== Route Option ===" << std::endl;
        std::cout << "Route: ";
        for (size_t i = 0; i < cities.size(); i++) {
            std::cout << cities[i];
            if (i < cities.size() - 1) std::cout << " -> ";
        }
        std::cout << "\nFlights: ";
        for (const auto& flight : flights) {
            std::cout << flight << " ";
        }
        std::cout << "\nTotal Distance: " << totalDistance << " km" << std::endl;
        std::cout << "Total Price: $" << std::fixed << std::setprecision(2) << totalPrice << std::endl;
        std::cout << "Total Time: " << totalTime << " minutes (" 
                  << totalTime/60 << "h " << totalTime%60 << "m)" << std::endl;
    }
};

class RouteService {
private:
    FlightGraph routeGraph;
    
    RouteOption convertToRouteOption(const FlightGraph::DijkstraResult& graphResult);
    RouteOption calculateRouteForPath(const std::vector<std::string>& path, 
                                     const std::vector<FlightGraph::Edge*>& edges);
    
public:
    // Constructor
    RouteService() = default;
    
    // Graph setup
    void addCity(const std::string& code, const std::string& name);
    void addRoute(const std::string& from, const std::string& to, 
                  int distance, double price, int duration, 
                  const std::string& flightNumber);
    
    // Route finding
    RouteOption findShortestRoute(const std::string& from, const std::string& to);
    RouteOption findCheapestRoute(const std::string& from, const std::string& to);
    RouteOption findFastestRoute(const std::string& from, const std::string& to);
    
    // Advanced route finding
    std::vector<RouteOption> findAllRoutes(const std::string& from, 
                                          const std::string& to, 
                                          int maxStops = 3);
    
    // Queries
    std::vector<std::string> getConnectedCities(const std::string& city);
    std::vector<std::string> getAllCities();
    bool hasDirectFlight(const std::string& from, const std::string& to);
    int getRoutesFromCount(const std::string& city);
    
    // Display
    void printRouteMap();
    void printCityConnections(const std::string& city);
    
    // Graph statistics
    int getCityCount() const;
    int getRouteCount() const;
    
    // Get city name
    std::string getCityName(const std::string& code) const;
    
    // Find connecting flights between two cities
    std::vector<std::string> findConnectingFlights(const std::string& from, 
                                                  const std::string& to);
    
    // Get all routes between two cities (direct and indirect)
    std::vector<RouteOption> getRoutesBetween(const std::string& from, 
                                             const std::string& to, 
                                             int maxStops = 2);
    
    // Find routes with constraints
    std::vector<RouteOption> findRoutesUnderPrice(const std::string& from, 
                                                 const std::string& to, 
                                                 double maxPrice);
    std::vector<RouteOption> findRoutesUnderTime(const std::string& from, 
                                                const std::string& to, 
                                                int maxMinutes);
};

#endif