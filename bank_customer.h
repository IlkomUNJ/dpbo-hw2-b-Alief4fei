#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

struct BankTransaction {
    int id;
    int customerId;
    double amount;
    std::string datetime;
    std::string note;
    std::string serialize() const {
        std::ostringstream ss;
        ss << id << "|" << customerId << "|" << amount << "|" << datetime << "|" << note;
        return ss.str();
    }
};

class BankCustomer {
private:
    int id;
    std::string name;
    double balance;
    
public:
    BankCustomer() = default;
    BankCustomer(int id_, const std::string &name_, double balance_)
        : id(id_), name(name_), balance(balance_) {}
        
    int getId() const { return id; }
    std::string getName() const { 
        return name; 
    }

    double getBalance() const { 
        return balance; 
    }

    void setBalance(double b) { 
        balance = b; 
    }

    void addBalance(double a) { 
        balance += a; 
    }

    bool withdraw(double a) {
        if (a > balance) {
            return false;
        }
        balance -= a;
        return true;
    }
    std::string serialize() const {
        std::ostringstream ss;
        ss << id << "|" << name << "|" << balance;
        return ss.str();
    }

    static BankCustomer deserialize(const std::string &line) {
        auto p = ::split_pipe(line);
        int id = stoi(p[0]);
        std::string name = p[1];
        double bal = p.size() > 2 ? stod(p[2]) : 0.0;
        return BankCustomer(id, name, bal);
    }
};
