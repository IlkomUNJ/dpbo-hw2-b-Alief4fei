#pragma once
#include <string>
#include <sstream>

class Item {
private:
    int id;
    int sellerId;
    std::string name;
    int quantity;
    double price;
public:
    Item() = default;
    Item(int id_, int sellerId_, const std::string &name_, int qty_, double price_)
        : id(id_), sellerId(sellerId_), name(name_), quantity(qty_), price(price_) {}

    int getId() const { 
        return id; 
    }

    int getSellerId() const { 
        return sellerId; 
    }

    std::string getName() const { 
        return name; 
    }

    int getQuantity() const { 
        return quantity; 
    }

    double getPrice() const { 
        return price; 
    }

    void setQuantity(int q) { 
        quantity = q; 
    }

    void setPrice(double p) { 
        price = p; 
    }

    std::string serialize() const {
        std::ostringstream ss;
        ss << id << "|" << sellerId << "|" << name << "|" << quantity << "|" << price;
        return ss.str();
    }
    
    static Item deserialize(const std::string &line) {
        auto p = ::split_pipe(line);
        int id = stoi(p[0]);
        int sellerId = stoi(p[1]);
        std::string name = p[2];
        int qty = stoi(p[3]);
        double price = stod(p[4]);
        return Item(id, sellerId, name, qty, price);
    }
};
