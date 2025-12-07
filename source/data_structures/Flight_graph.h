 #ifndef FLIGHTGRAPH_H
#define FLIGHTGRAPH_H
#include "linked_list.h"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <limits>
#include <algorithm>
using namespace std;

class FlightGraph {
public:  // MOVE PUBLIC SECTION UP
    // City structure - MADE PUBLIC
    struct City {
        string code;    // "JFK", "LAX"
        string name;    // "John F. Kennedy"
        
        City(string c = "", string n = "") : code(c), name(n) {}
    };
    
    // Edge structure for adjacency list - MADE PUBLIC
    struct Edge {
        string from;
        string to;
        int distance;    // km
        double price;    // USD
        int duration;    // minutes
        string flightNumber;
        
        Edge(string f = "", string t = "", int d = 0, double p = 0.0, 
             int dur = 0, string fn = "")
            : from(f), to(t), distance(d), price(p), duration(dur), flightNumber(fn) {}
    };
    
    // For Dijkstra's algorithm - MADE PUBLIC
    struct DijkstraResult {
        vector<string> path;
        vector<string> flights;
        int totalDistance;
        double totalPrice;
        int totalTime;
        
        DijkstraResult() : totalDistance(0), totalPrice(0.0), totalTime(0) {}
        
        bool isValid() const { return !path.empty(); }
    };

private:
    // Adjacency list: city -> linked list of edges
    unordered_map<string, LinkedList<Edge*>> adjacencyList;
    
    // City information
    unordered_map<string, City> cities;

public:
    // Add city to graph
    void addCity(const string& code, const string& name) {
        cities[code] = City(code, name);
        adjacencyList[code] = LinkedList<Edge*>();  // Initialize empty list
    }
    
    // Add flight route
    void addRoute(const string& from, const string& to, 
                  int distance, double price, int duration, 
                  const string& flightNumber) {
        
        if (cities.find(from) == cities.end() || cities.find(to) == cities.end()) {
            cout << "Error: City not found!" << endl;
            return;
        }
        
        Edge* newEdge = new Edge(from, to, distance, price, duration, flightNumber);
        adjacencyList[from].add(newEdge);
    }
    
    // Get routes from city
    LinkedList<Edge*> getRoutesFrom(const string& city) {
        if (adjacencyList.find(city) != adjacencyList.end()) {
            return adjacencyList[city];
        }
        return LinkedList<Edge*>();
    }
    
    // Find shortest path (by distance) - RETURNS FULL RESULT
    DijkstraResult findShortestPath(const string& start, const string& end) {
        return findPathWithTotals(start, end, "distance");
    }
    
    // Find cheapest path (by price) - RETURNS FULL RESULT
    DijkstraResult findCheapestPath(const string& start, const string& end) {
        return findPathWithTotals(start, end, "price");
    }
    
    // Find fastest path (by duration) - RETURNS FULL RESULT
    DijkstraResult findFastestPath(const string& start, const string& end) {
        return findPathWithTotals(start, end, "duration");
    }
    
    // Get all cities
    vector<string> getAllCities() const {
        vector<string> result;
        for (const auto& pair : cities) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    // Get number of cities in graph
    int getCityCount() const {
        return static_cast<int>(cities.size());
    }
    
    // Get total number of routes/flights in graph
    int getRouteCount() const {
        int count = 0;
        for (const auto& pair : adjacencyList) {
            ListNode<Edge*>* current = pair.second.begin();
            while (current != nullptr) {
                count++;
                current = current->next;
            }
        }
        return count;
    }
    
    // Get number of routes from a specific city
    int getRoutesFromCount(const string& city) const {
        if (adjacencyList.find(city) == adjacencyList.end()) {
            return 0;
        }
        
        int count = 0;
        ListNode<Edge*>* current = adjacencyList.at(city).begin();
        while (current != nullptr) {
            count++;
            current = current->next;
        }
        return count;
    }
    
    // Check if city exists in graph
    bool hasCity(const string& city) const {
        return cities.find(city) != cities.end();
    }
    
    // Check if direct route exists
    bool hasDirectRoute(const string& from, const string& to) const {
        if (adjacencyList.find(from) == adjacencyList.end()) {
            return false;
        }
        
        ListNode<Edge*>* current = adjacencyList.at(from).begin();
        while (current != nullptr) {
            if (current->data->to == to) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
    
    // Print graph
    void printGraph() const {
        cout << "\n=== Flight Network ===" << endl;
        cout << "Cities: " << getCityCount() << ", Routes: " << getRouteCount() << endl;
        
        for (const auto& cityPair : cities) {
            string city = cityPair.first;
            cout << "\n" << city << " (" << cityPair.second.name << "):" << endl;
            
            if (adjacencyList.find(city) != adjacencyList.end()) {
                ListNode<Edge*>* current = adjacencyList.at(city).begin();
                int routeCount = 0;
                while (current != nullptr) {
                    Edge* edge = current->data;
                    cout << "  -> " << edge->to << " (Flight: " << edge->flightNumber 
                         << ", Dist: " << edge->distance << "km, "
                         << "Price: $" << edge->price << ", "
                         << "Time: " << edge->duration << "min)" << endl;
                    current = current->next;
                    routeCount++;
                }
                
                if (routeCount == 0) {
                    cout << "  [No outgoing flights]" << endl;
                }
            } else {
                cout << "  [No adjacency list entry]" << endl;
            }
        }
    }
    
    // Get city name by code
    string getCityName(const string& code) const {
        auto it = cities.find(code);
        if (it != cities.end()) {
            return it->second.name;
        }
        return "Unknown";
    }
    
private:
    // Dijkstra's algorithm that returns totals
    DijkstraResult findPathWithTotals(const string& start, const string& end, 
                                     const string& criterion) {
        DijkstraResult result;
        
        // Check if cities exist
        if (cities.find(start) == cities.end() || cities.find(end) == cities.end()) {
            return result;
        }
        
        // Priority queue for Dijkstra: (cost, city, path, flights)
        struct QueueNode {
            double cost;
            string city;
            vector<string> path;
            vector<string> flights;
            
            bool operator>(const QueueNode& other) const {
                return cost > other.cost;
            }
        };
        
        priority_queue<QueueNode, vector<QueueNode>, greater<QueueNode>> pq;
        
        // Distance map
        unordered_map<string, double> dist;
        for (const auto& cityPair : cities) {
            dist[cityPair.first] = numeric_limits<double>::max();
        }
        dist[start] = 0;
        
        // Start node
        pq.push({0, start, {start}, {}});
        
        while (!pq.empty()) {
            QueueNode current = pq.top();
            pq.pop();
            
            if (current.city == end) {
                result.path = current.path;
                result.flights = current.flights;
                calculateRouteTotals(result);
                return result;
            }
            
            // If we found a better path already, skip
            if (current.cost > dist[current.city]) {
                continue;
            }
            
            // Explore neighbors
            if (adjacencyList.find(current.city) != adjacencyList.end()) {
                ListNode<Edge*>* edgeNode = adjacencyList[current.city].begin();
                while (edgeNode != nullptr) {
                    Edge* edge = edgeNode->data;
                    string neighbor = edge->to;
                    
                    // Calculate new cost based on criterion
                    double edgeCost;
                    if (criterion == "distance") {
                        edgeCost = edge->distance;
                    } else if (criterion == "price") {
                        edgeCost = edge->price;
                    } else { // duration
                        edgeCost = edge->duration;
                    }
                    
                    double newCost = current.cost + edgeCost;
                    
                    if (newCost < dist[neighbor]) {
                        dist[neighbor] = newCost;
                        
                        // Create new path
                        vector<string> newPath = current.path;
                        newPath.push_back(neighbor);
                        
                        // Create new flights list
                        vector<string> newFlights = current.flights;
                        newFlights.push_back(edge->flightNumber);
                        
                        pq.push({newCost, neighbor, newPath, newFlights});
                    }
                    
                    edgeNode = edgeNode->next;
                }
            }
        }
        
        return result;  // No path found
    }
    
    // Calculate totals for a route
    void calculateRouteTotals(DijkstraResult& result) {
        result.totalDistance = 0;
        result.totalPrice = 0;
        result.totalTime = 0;
        
        for (size_t i = 0; i < result.flights.size(); i++) {
            string from = result.path[i];
            string to = result.path[i + 1];
            string flight = result.flights[i];
            
            // Find this edge and get its values
            ListNode<Edge*>* edgeNode = adjacencyList[from].begin();
            while (edgeNode != nullptr) {
                Edge* edge = edgeNode->data;
                if (edge->to == to && edge->flightNumber == flight) {
                    result.totalDistance += edge->distance;
                    result.totalPrice += edge->price;
                    result.totalTime += edge->duration;
                    break;
                }
                edgeNode = edgeNode->next;
            }
        }
    }
};

#endif