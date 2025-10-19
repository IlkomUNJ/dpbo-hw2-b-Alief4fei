#pragma once
#include "buyer.h"
#include <string>

class Seller {
private:
    int id;
    int buyerLinkId;
    std::string storeName;
    std::string storeAddress;
    std::string storePhone;
    std::string storeEmail;

public:
    Seller() = default;
    Seller(int id_, int buyerId_, const std::string &name_, const std::string &addr_, const std::string &phone_, const std::string &email_)
        : id(id_), buyerLinkId(buyerId_), storeName(name_), storeAddress(addr_), storePhone(phone_), storeEmail(email_) {}

    int getId() const { 
        return id; 
    }

    int getBuyerId() const {
         return buyerLinkId; 
        }

    std::string getStoreName() const {
         return storeName; 
        }

    std::string serialize() const {
        std::ostringstream ss;
        ss << id << "|" << buyerLinkId << "|" << storeName << "|" << storeAddress << "|" << storePhone << "|" << storeEmail;
        return ss.str();
    }

    static Seller deserialize(const std::string &line) {
        auto p = ::split_pipe(line);
        int id = stoi(p[0]);
        int buyerId = stoi(p[1]);
        return Seller(id, buyerId, p[2], p[3], p[4], p[5]);
    }
};
