#include "graph.h"
#include <iostream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <limits>
#include <set>

using namespace std;

// Convert FlightGraph result to RouteOption
RouteOption RouteService::convertToRouteOption(const FlightGraph::DijkstraResult& graphResult) {
    RouteOption option;
    
    if (!graphResult.isValid()) {
        return option; // Empty option
    }
    
    option.cities = graphResult.path;
    option.flights = graphResult.flights;
    option.totalDistance = graphResult.totalDistance;
    option.totalPrice = graphResult.totalPrice;
    option.totalTime = graphResult.totalTime;
    
    return option;
}

// Graph setup methods
void RouteService::addCity(const std::string& code, const std::string& name) {
    routeGraph.addCity(code, name);
}

void RouteService::addRoute(const std::string& from, const std::string& to, 
                           int distance, double price, int duration, 
                           const std::string& flightNumber) {
    routeGraph.addRoute(from, to, distance, price, duration, flightNumber);
}

// Route finding
RouteOption RouteService::findShortestRoute(const std::string& from, const std::string& to) {
    FlightGraph::DijkstraResult graphResult = routeGraph.findShortestPath(from, to);
    return convertToRouteOption(graphResult);
}

RouteOption RouteService::findCheapestRoute(const std::string& from, const std::string& to) {
    FlightGraph::DijkstraResult graphResult = routeGraph.findCheapestPath(from, to);
    return convertToRouteOption(graphResult);
}

RouteOption RouteService::findFastestRoute(const std::string& from, const std::string& to) {
    FlightGraph::DijkstraResult graphResult = routeGraph.findFastestPath(from, to);
    return convertToRouteOption(graphResult);
}

// Find all possible routes using BFS
vector<RouteOption> RouteService::findAllRoutes(const std::string& from, 
                                               const std::string& to, 
                                               int maxStops) {
    vector<RouteOption> allRoutes;
    
    // Check if cities exist
    vector<string> allCities = getAllCities();
    if (find(allCities.begin(), allCities.end(), from) == allCities.end() ||
        find(allCities.begin(), allCities.end(), to) == allCities.end()) {
        return allRoutes;
    }
    
    // Use BFS for all paths
    struct BFSNode {
        string city;
        vector<string> path;
        vector<FlightGraph::Edge*> edges;
        int stops;
        int totalDistance;
        double totalPrice;
        int totalTime;
        
        BFSNode(string c, vector<string> p, vector<FlightGraph::Edge*> e, int s, 
                int d = 0, double pr = 0.0, int t = 0)
            : city(c), path(p), edges(e), stops(s), 
              totalDistance(d), totalPrice(pr), totalTime(t) {}
    };
    
    queue<BFSNode> q;
    q.push(BFSNode(from, {from}, {}, 0));
    
    while (!q.empty()) {
        BFSNode current = q.front();
        q.pop();
        
        // If reached destination and not just the starting point
        if (current.city == to && current.path.size() > 1) {
            RouteOption route;
            route.cities = current.path;
            route.totalDistance = current.totalDistance;
            route.totalPrice = current.totalPrice;
            route.totalTime = current.totalTime;
            
            // Collect flight numbers
            for (const auto& edge : current.edges) {
                route.flights.push_back(edge->flightNumber);
            }
            
            allRoutes.push_back(route);
        }
        
        // Stop if too many stops
        if (current.stops >= maxStops) {
            continue;
        }
        
        // Get all edges from current city
        LinkedList<FlightGraph::Edge*> edgeList = routeGraph.getRoutesFrom(current.city);
        ListNode<FlightGraph::Edge*>* edgeNode = edgeList.begin();
        
        while (edgeNode != nullptr) {
            FlightGraph::Edge* edge = edgeNode->data;
            string nextCity = edge->to;
            
            // Avoid cycles (simple check)
            if (find(current.path.begin(), current.path.end(), nextCity) == current.path.end()) {
                vector<string> newPath = current.path;
                newPath.push_back(nextCity);
                
                vector<FlightGraph::Edge*> newEdges = current.edges;
                newEdges.push_back(edge);
                
                // Update totals
                int newDistance = current.totalDistance + edge->distance;
                double newPrice = current.totalPrice + edge->price;
                int newTime = current.totalTime + edge->duration;
                
                q.push(BFSNode(nextCity, newPath, newEdges, current.stops + 1,
                              newDistance, newPrice, newTime));
            }
            
            edgeNode = edgeNode->next;
        }
    }
    
    return allRoutes;
}

// Calculate totals for a path (alternative method)
RouteOption RouteService::calculateRouteForPath(const vector<string>& path, 
                                               const vector<FlightGraph::Edge*>& edges) {
    RouteOption route;
    route.cities = path;
    
    int totalDist = 0;
    double totalPrice = 0;
    int totalTime = 0;
    
    for (size_t i = 0; i < edges.size(); i++) {
        FlightGraph::Edge* edge = edges[i];
        route.flights.push_back(edge->flightNumber);
        
        totalDist += edge->distance;
        totalPrice += edge->price;
        totalTime += edge->duration;
    }
    
    route.totalDistance = totalDist;
    route.totalPrice = totalPrice;
    route.totalTime = totalTime;
    
    return route;
}

// Get cities connected to a city
vector<string> RouteService::getConnectedCities(const std::string& city) {
    vector<string> connected;
    
    LinkedList<FlightGraph::Edge*> edges = routeGraph.getRoutesFrom(city);
    ListNode<FlightGraph::Edge*>* current = edges.begin();
    
    while (current != nullptr) {
        FlightGraph::Edge* edge = current->data;
        connected.push_back(edge->to);
        current = current->next;
    }
    
    return connected;
}

// Get all cities in network
vector<string> RouteService::getAllCities() {
    return routeGraph.getAllCities();
}

// Check if direct flight exists
bool RouteService::hasDirectFlight(const std::string& from, const std::string& to) {
    vector<string> connected = getConnectedCities(from);
    return find(connected.begin(), connected.end(), to) != connected.end();
}

// Get number of routes from a city
int RouteService::getRoutesFromCount(const std::string& city) {
    return routeGraph.getRoutesFromCount(city);
}

// Print route map
void RouteService::printRouteMap() {
    routeGraph.printGraph();
}

// Print specific city connections
void RouteService::printCityConnections(const std::string& city) {
    vector<string> connected = getConnectedCities(city);
    
    cout << "\n=== Connections from " << city << " (" << getCityName(city) << ") ===" << endl;
    cout << "Direct flights: " << connected.size() << endl;
    
    if (connected.empty()) {
        cout << "No direct flights from " << city << endl;
        return;
    }
    
    LinkedList<FlightGraph::Edge*> edges = routeGraph.getRoutesFrom(city);
    ListNode<FlightGraph::Edge*>* current = edges.begin();
    
    while (current != nullptr) {
        FlightGraph::Edge* edge = current->data;
        cout << "  → " << edge->to << " (" << getCityName(edge->to) << ")" << endl;
        cout << "     Flight: " << edge->flightNumber 
             << ", Distance: " << edge->distance << "km"
             << ", Price: $" << fixed << setprecision(2) << edge->price
             << ", Time: " << edge->duration << "min" << endl;
        current = current->next;
    }
}

// Graph statistics
int RouteService::getCityCount() const {
    return routeGraph.getCityCount();
}

int RouteService::getRouteCount() const {
    return routeGraph.getRouteCount();
}

// Get city name
std::string RouteService::getCityName(const std::string& code) const {
    return routeGraph.getCityName(code);
}

// Find connecting flights between two cities
vector<string> RouteService::findConnectingFlights(const std::string& from, 
                                                  const std::string& to) {
    vector<string> connections;
    
    // Get all cities connected from source
    vector<string> intermediateCities = getConnectedCities(from);
    
    for (const auto& intermediate : intermediateCities) {
        // Check if intermediate connects to destination
        if (hasDirectFlight(intermediate, to)) {
            connections.push_back(intermediate);
        }
    }
    
    return connections;
}

// Get all routes between two cities
vector<RouteOption> RouteService::getRoutesBetween(const std::string& from, 
                                                  const std::string& to, 
                                                  int maxStops) {
    return findAllRoutes(from, to, maxStops);
}

// Find routes under maximum price
vector<RouteOption> RouteService::findRoutesUnderPrice(const std::string& from, 
                                                      const std::string& to, 
                                                      double maxPrice) {
    vector<RouteOption> allRoutes = findAllRoutes(from, to, 3);
    vector<RouteOption> filteredRoutes;
    
    for (const auto& route : allRoutes) {
        if (route.totalPrice <= maxPrice) {
            filteredRoutes.push_back(route);
        }
    }
    
    return filteredRoutes;
}

// Find routes under maximum time
vector<RouteOption> RouteService::findRoutesUnderTime(const std::string& from, 
                                                     const std::string& to, 
                                                     int maxMinutes) {
    vector<RouteOption> allRoutes = findAllRoutes(from, to, 3);
    vector<RouteOption> filteredRoutes;
    
    for (const auto& route : allRoutes) {
        if (route.totalTime <= maxMinutes) {
            filteredRoutes.push_back(route);
        }
    }
    
    return filteredRoutes;
}