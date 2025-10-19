#pragma once
#include <vector>
#include <fstream>
#include <iostream>
#include "buyer.h"
#include "seller.h"
#include "item.h"
#include "transaction.h"
#include "bank_customer.h"
#include "utils.h"

class Database {
public:
    std::vector<Buyer> buyers;
    std::vector<Seller> sellers;
    std::vector<Item> items;
    std::vector<Transaction> transactions;
    std::vector<BankCustomer> bankCustomers;
    int nextBuyerId = 1;
    int nextSellerId = 1;
    int nextItemId = 1;
    int nextTransId = 1;
    int nextBankId = 1;

    Database() {
        ensure_data_folder();
    }

    void loadAll() {
        buyers.clear(); sellers.clear(); items.clear(); transactions.clear(); bankCustomers.clear();
        {
            std::ifstream in("data/buyers.txt");
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty() || line[0]=='#') continue;
                buyers.push_back(Buyer::deserialize(line));
            }
            nextBuyerId = buyers.empty() ? 1 : (buyers.back().getId() + 1);
        }

        {
            std::ifstream in("data/sellers.txt");
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty() || line[0]=='#') continue;
                sellers.push_back(Seller::deserialize(line));
            }
            nextSellerId = sellers.empty() ? 1 : (sellers.back().getId() + 1);
        }

        {
            std::ifstream in("data/items.txt");
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty() || line[0]=='#') continue;
                items.push_back(Item::deserialize(line));
            }
            nextItemId = items.empty() ? 1 : (items.back().getId() + 1);
        }

        {
            std::ifstream in("data/transactions.txt");
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty() || line[0]=='#') continue;
                transactions.push_back(Transaction::deserialize(line));
            }
            nextTransId = transactions.empty() ? 1 : (transactions.back().id + 1);
        }

        {
            std::ifstream in("data/bankcustomers.txt");
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty() || line[0]=='#') continue;
                bankCustomers.push_back(BankCustomer::deserialize(line));
            }
            nextBankId = bankCustomers.empty() ? 1 : (bankCustomers.back().getId() + 1);
        }
    }

    void saveAll() {
        {
            std::ofstream out("data/buyers.txt", std::ios::trunc);
            for (auto &b: buyers) out << b.serialize() << "\n";
        }
        {
            std::ofstream out("data/sellers.txt", std::ios::trunc);
            for (auto &s: sellers) out << s.serialize() << "\n";
        }
        {
            std::ofstream out("data/items.txt", std::ios::trunc);
            for (auto &i: items) out << i.serialize() << "\n";
        }
        {
            std::ofstream out("data/transactions.txt", std::ios::trunc);
            for (auto &t: transactions) out << t.serialize() << "\n";
        }
        {
            std::ofstream out("data/bankcustomers.txt", std::ios::trunc);
            for (auto &bc: bankCustomers) out << bc.serialize() << "\n";
        }
    }

    Buyer* findBuyerById(int id) {
        for (auto &b: buyers) if (b.getId()==id) return &b;
        return nullptr;
    }
    Seller* findSellerById(int id) {
        for (auto &s: sellers) if (s.getId()==id) return &s;
        return nullptr;
    }
    Item* findItemById(int id) {
        for (auto &it: items) if (it.getId()==id) return &it;
        return nullptr;
    }
    BankCustomer* findBankById(int id) {
        for (auto &bc: bankCustomers) if (bc.getId()==id) return &bc;
        return nullptr;
    }
};
