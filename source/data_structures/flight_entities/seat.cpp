#include "seat.h"
#include <sstream>

SeatMap::SeatMap(std::string fid, int total) 
    : flightId(fid), totalSeats(total) {
    seats = new Bitmap(total);
}

SeatMap::~SeatMap() {
    delete seats;
}

bool SeatMap::takeSeat(int seat, std::string pnr) {
    if (seat < 0 || seat >= totalSeats) return false;
    if (seats->isOccupied(seat)) return false;
    seats->occupySeat(seat);
    assignments[seat] = pnr;
    return true;
}

int SeatMap::autoAssign(std::string pnr) {
    int seat = seats->findFirstAvailable();
    if (seat != -1) {
        takeSeat(seat, pnr);
    }
    return seat;
}

void SeatMap::freeSeat(int seat) {
    if (seat < 0 || seat >= totalSeats) return;
    seats->freeSeat(seat);
    assignments.erase(seat);
}

void SeatMap::freeSeatByPNR(std::string pnr) {
    for (auto it = assignments.begin(); it != assignments.end(); ++it) {
        if (it->second == pnr) {
            freeSeat(it->first);
            break;
        }
    }
}

bool SeatMap::isTaken(int seat) const {
    if (seat < 0 || seat >= totalSeats) return true;
    return seats->isOccupied(seat);
}

int SeatMap::countFree() const {
    return seats->countAvailable();
}

int SeatMap::countTaken() const {
    return totalSeats - countFree();
}

std::string SeatMap::getPassengerAtSeat(int seat) const {
    if (seat < 0 || seat >= totalSeats) return "";
    auto it = assignments.find(seat);
    if (it != assignments.end()) return it->second;
    return "";
}

int SeatMap::getSeatOfPassenger(std::string pnr) const {
    for (auto it = assignments.begin(); it != assignments.end(); ++it) {
        if (it->second == pnr) return it->first;
    }
    return -1;
}

std::string SeatMap::showMap() const {
    return seats->getVisualMap();
}