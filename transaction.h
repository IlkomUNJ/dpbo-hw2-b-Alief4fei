#pragma once
#include <string>
#include <sstream>

enum class TransStatus { CREATED, PAID, COMPLETED, CANCELLED };

inline std::string status_to_string(TransStatus s) {
    switch (s) {
        case TransStatus::CREATED: return "CREATED";
        case TransStatus::PAID: return "PAID";
        case TransStatus::COMPLETED: return "COMPLETED";
        case TransStatus::CANCELLED: return "CANCELLED";
    }
    return "UNKNOWN";
}
inline TransStatus string_to_status(const std::string &s) {
    if (s=="PAID") return TransStatus::PAID;
    if (s=="COMPLETED") return TransStatus::COMPLETED;
    if (s=="CANCELLED") return TransStatus::CANCELLED;
    return TransStatus::CREATED;
}

struct Transaction {
    int id;
    int buyerId;
    int sellerId;
    int itemId;
    int quantity;
    double total;
    TransStatus status;
    std::string datetime;
    std::string serialize() const {
        std::ostringstream ss;
        ss << id << "|" << buyerId << "|" << sellerId << "|" << itemId << "|" << quantity << "|" << total << "|" << status_to_string(status) << "|" << datetime;
        return ss.str();
    }
    static Transaction deserialize(const std::string &line) {
        auto p = ::split_pipe(line);
        Transaction t;
        t.id = stoi(p[0]);
        t.buyerId = stoi(p[1]);
        t.sellerId = stoi(p[2]);
        t.itemId = stoi(p[3]);
        t.quantity = stoi(p[4]);
        t.total = stod(p[5]);
        t.status = string_to_status(p[6]);
        t.datetime = p[7];
        return t;
    }
};
