// test_graph.cpp
#include <iostream>
#include "Flight_graph.h"
#include "graph.h"

using namespace std;

int main() {
    // Test FlightGraph directly
    FlightGraph graph;
    
    graph.addCity("JFK", "John F Kennedy");
    graph.addCity("LAX", "Los Angeles");
    
    cout << "City count: " << graph.getCityCount() << endl;
    cout << "Has JFK: " << graph.hasCity("JFK") << endl;
    
    graph.addRoute("JFK", "LAX", 4000, 299.99, 360, "AA101");
    
    cout << "Route count: " << graph.getRouteCount() << endl;
    cout << "Routes from JFK: " << graph.getRoutesFromCount("JFK") << endl;
    
    // Test RouteService
    RouteService routeService;
    routeService.addCity("JFK", "John F Kennedy");
    routeService.addCity("LAX", "Los Angeles");
    routeService.addRoute("JFK", "LAX", 4000, 299.99, 360, "AA101");
    
    RouteOption route = routeService.findFastestRoute("JFK", "LAX");
    if (route.isValid()) {
        cout << "\nFound route!" << endl;
        route.print();
    }
    
    return 0;
}