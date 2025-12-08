#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <memory>
#include "../include/FlightServer.h"

using namespace std;

unique_ptr<FlightServer> server;

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    cout << "\n🛑 Interrupt signal (" << signum << ") received." << endl;
    
    if (server) {
        cout << "Stopping flight server..." << endl;
        server->stop();
    }
    
    exit(signum);
}

// Banner display
void printBanner() {
    cout << "==================================================" << endl;
    cout << "       ✈️  FLIGHT MANAGEMENT SYSTEM SERVER       " << endl;
    cout << "==================================================" << endl;
    cout << "   Advanced Data Structures Implementation:" << endl;
    cout << "   • B-Tree for O(log n) time-based queries" << endl;
    cout << "   • Hash Tables for O(1) passenger lookups" << endl;
    cout << "   • Dijkstra's Algorithm for shortest paths" << endl;
    cout << "   • Bitmap for efficient seat management" << endl;
    cout << "==================================================" << endl;
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Parse command line arguments
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    printBanner();
    
    try {
        // Create and start server
        server = make_unique<FlightServer>(port);
        
        cout << "🚀 Starting Flight Server on port " << port << "..." << endl;
        
        if (server->start()) {
            cout << "✅ Server started successfully!" << endl;
            cout << "📡 Endpoints available:" << endl;
            cout << "   GET  /api/health" << endl;
            cout << "   GET  /api/flights" << endl;
            cout << "   GET  /api/flights/range?start=HH:MM&end=HH:MM" << endl;
            cout << "   GET  /api/passengers/{pnr}" << endl;
            cout << "   GET  /api/route/shortest/{from}/{to}" << endl;
            cout << "   GET  /api/gates/range?min=X&max=Y" << endl;
            cout << "   POST /api/checkin/{pnr}" << endl;
            cout << "\n📊 Test with: curl http://localhost:" << port << "/api/health" << endl;
            cout << "💡 Press Ctrl+C to stop the server" << endl;
            
            // Keep main thread alive
            while (true) {
                this_thread::sleep_for(chrono::seconds(10));
                server->printStats();
            }
        } else {
            cerr << "❌ Failed to start server" << endl;
            return 1;
        }
    } catch (const exception& e) {
        cerr << "❌ Server error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
