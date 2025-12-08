#ifndef BITMAP_H
#define BITMAP_H

#include <iostream>
#include <vector>
#include <string>
#include <bitset>
using namespace std;

class Bitmap {
private:
    vector<unsigned char> bits;
    int totalSeats;
    int totalRows;
    int seatsPerRow;
    
public:
    Bitmap(int seats = 300, int rows = 50, int seatsPerRow = 6) 
        : totalSeats(seats), totalRows(rows), seatsPerRow(seatsPerRow) {
        
        // Calculate bytes needed: ceil(totalSeats / 8)
        int bytesNeeded = (totalSeats + 7) / 8;
        bits.resize(bytesNeeded, 0);  // All seats available initially
    }
    
    // Mark seat as occupied
    void occupySeat(int seatNumber) {
        if (seatNumber < 0 || seatNumber >= totalSeats) {
            cout << "Invalid seat number!" << endl;
            return;
        }
        
        int byteIndex = seatNumber / 8;
        int bitIndex = seatNumber % 8;
        bits[byteIndex] |= (1 << bitIndex);
    }
    
    // Free a seat
    void freeSeat(int seatNumber) {
        if (seatNumber < 0 || seatNumber >= totalSeats) {
            cout << "Invalid seat number!" << endl;
            return;
        }
        
        int byteIndex = seatNumber / 8;
        int bitIndex = seatNumber % 8;
        bits[byteIndex] &= ~(1 << bitIndex);
    }
    
    // Check if seat is occupied
    bool isOccupied(int seatNumber) const {
        if (seatNumber < 0 || seatNumber >= totalSeats) {
            return false;
        }
        
        int byteIndex = seatNumber / 8;
        int bitIndex = seatNumber % 8;
        return (bits[byteIndex] & (1 << bitIndex)) != 0;
    }
    
    // Find first available seat
    int findFirstAvailable() const {
        for (int i = 0; i < totalSeats; i++) {
            if (!isOccupied(i)) {
                return i;
            }
        }
        return -1;  // No seats available
    }
     int getTotalSeats() const {
        return totalSeats;
    }
    // Count available seats
    int countAvailable() const {
        int count = 0;
        for (int i = 0; i < totalSeats; i++) {
            if (!isOccupied(i)) {
                count++;
            }
        }
        return count;
    }
    
    // Get visual representation
    string getVisualMap() const {
        string visual = "\n=== Seat Map ===\n";
        
        for (int row = 0; row < totalRows; row++) {
            visual += "Row " + to_string(row + 1) + ": ";
            
            for (int seatInRow = 0; seatInRow < seatsPerRow; seatInRow++) {
                int seatNumber = row * seatsPerRow + seatInRow;
                
                if (seatNumber >= totalSeats) break;
                
                if (isOccupied(seatNumber)) {
                    visual += "🔴 ";  // Occupied
                } else {
                    visual += "🟢 ";  // Available
                }
                
                // Add aisle marker
                if (seatInRow == seatsPerRow / 2 - 1) {
                    visual += "| ";
                }
            }
            visual += "\n";
        }
        
        visual += "\n🔴 = Occupied | 🟢 = Available\n";
        return visual;
    }
    
    // Print seat status
    void printSeatStatus() const {
        cout << "\n=== Seat Status ===" << endl;
        cout << "Total Seats: " << totalSeats << endl;
        cout << "Available: " << countAvailable() << endl;
        cout << "Occupied: " << (totalSeats - countAvailable()) << endl;
        
        cout << "\nSeat Map:" << endl;
        cout << getVisualMap();
    }
    
    // Get all available seats
    vector<int> getAvailableSeats() const {
        vector<int> available;
        for (int i = 0; i < totalSeats; i++) {
            if (!isOccupied(i)) {
                available.push_back(i);
            }
        }
        return available;
    }
    
    // Get all occupied seats
    vector<int> getOccupiedSeats() const {
        vector<int> occupied;
        for (int i = 0; i < totalSeats; i++) {
            if (isOccupied(i)) {
                occupied.push_back(i);
            }
        }
        return occupied;
    }
};

#endif