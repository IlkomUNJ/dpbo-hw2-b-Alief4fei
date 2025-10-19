#pragma once
#include <string>
#include <sstream>
#include "bank_customer.h"

class Buyer {
private:
    int id;
    std::string name;
    std::string address;
    std::string phone;
    std::string email;
    int bankAccountId;

public:
    Buyer() : id(0), bankAccountId(0) {}
    Buyer(int id_, const std::string &name_, const std::string &addr_, const std::string &phone_, const std::string &email_, int bankId_)
        : id(id_), name(name_), address(addr_), phone(phone_), email(email_), bankAccountId(bankId_) {}

    int getId() const { 
        return id; 
    }
    std::string getName() const { 
        return name; 
    }
    std::string getAddress() const {
        return address; 
    }
    std::string getPhone() const { 
        return phone; 
    }
    std::string getEmail() const { 
        return email; 
    }
    int getBankAccountId() const {
        return bankAccountId; 
    }

    void setBankAccountId(int bid) { 
        bankAccountId = bid; 
    }

    std::string serialize() const {
        std::ostringstream ss;
        ss << id << "|" << name << "|" << address << "|" << phone << "|" << email << "|" << bankAccountId;
        return ss.str();
    }
    static Buyer deserialize(const std::string &line) {
        auto p = ::split_pipe(line);
        int id = stoi(p[0]);
        std::string name = p[1];
        std::string addr = p[2];
        std::string phone = p[3];
        std::string email = p[4];
        int bankId = p.size()>5 ? stoi(p[5]) : 0;
        return Buyer(id, name, addr, phone, email, bankId);
    }
};
