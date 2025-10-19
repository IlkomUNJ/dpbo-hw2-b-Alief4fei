#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <iomanip>
#include <ctime>

#include "utils.h"
#include "database.h"

using namespace std;

enum MainMenu { MM_LOGIN = 1, MM_REGISTER = 2, MM_EXIT = 3 };
enum PostLogin { PL_ACCOUNT_INFO = 1, PL_SHOP = 2, PL_BANK = 3, PL_SELLER = 4, PL_UPGRADE = 5, PL_LOGOUT = 6 };

enum ShopMenu { SH_BROWSE = 1, SH_ORDERS = 2, SH_SPENDING = 3, SH_BACK = 4 };

enum BankMenu {
    BM_TOPUP = 1, BM_WITHDRAW = 2, BM_CASHFLOW_TODAY = 3, BM_CASHFLOW_MONTH = 4,
    BM_LIST_TRANSACTIONS_WEEK = 5, BM_LIST_CUSTOMERS = 6, BM_LIST_DORMANT = 7,
    BM_LIST_TOP_N = 8, BM_BACK = 9
};

enum SellerMenu { SMM_MANAGE_ITEMS = 1, SMM_VIEW_TX = 2, SMM_UPDATE_TX_STATUS = 3, SMM_ANALYTICS = 4, SMM_BACK = 5 };
enum SellerItemMenu { SIM_LIST = 1, SIM_ADD = 2, SIM_UPDATE = 3, SIM_REMOVE = 4, SIM_BACK = 5 };
enum TxFilter { TF_ALL = 1, TF_PAID = 2, TF_COMPLETED = 3, TF_CANCELLED = 4, TF_BACK = 5 };


int safe_int_input() {
    int x;
    while (true) {
        if (!(cin >> x)) {
            cin.clear();
            string garbage; getline(cin, garbage);
            cout << "Input invalid. Coba lagi: ";
            continue;
        }
        string rest; getline(cin, rest);
        return x;
    }
}

double safe_double_input() {
    double x;
    while (true) {
        if (!(cin >> x)) {
            cin.clear();
            string garbage; getline(cin, garbage);
            cout << "Input invalid. Coba lagi: ";
            continue;
        }
        string rest; getline(cin, rest);
        return x;
    }
}

void header_line() { cout << "==============================\n"; }
void sep_line() { cout << "------------------------------\n"; }

void press_enter_to_continue() {
    cout << "Tekan Enter untuk lanjut...";
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

time_t parse_datetime_local(const string &dt) {
    std::tm tm{};
    std::istringstream ss(dt);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) return 0;
    return std::mktime(&tm);
}
bool same_day_local(const string &dt, time_t now) {
    time_t t = parse_datetime_local(dt);
    if (t == 0) return false;
    std::tm tm_now{}, tm_t{};
#if defined(_MSC_VER)
    localtime_s(&tm_now, &now);
    localtime_s(&tm_t, &t);
#elif defined(__unix__) || defined(__APPLE__) || defined(_POSIX_VERSION)
    localtime_r(&now, &tm_now);
    localtime_r(&t, &tm_t);
#else
    std::tm *pnow = std::localtime(&now);
    if (pnow) tm_now = *pnow;
    std::tm *pt = std::localtime(&t);
    if (pt) tm_t = *pt;
#endif
    return (tm_now.tm_year==tm_t.tm_year && tm_now.tm_yday==tm_t.tm_yday);
}

Item* findItemBySellerLocalId(Database &db, int sellerId, int localItemId) {
    for (auto &it : db.items) {
        if (it.getSellerId() == sellerId && it.getId() == localItemId) return &it;
    }
    return nullptr;
}

Transaction* findTransactionBySellerLocalId(Database &db, int sellerId, int localTxId) {
    for (auto &t : db.transactions) {
        if (t.sellerId == sellerId && t.id == localTxId) return &t;
    }
    return nullptr;
}

void print_transaction_readable(Database &db, const Transaction &t) {
    sep_line();
    Item* it = findItemBySellerLocalId(db, t.sellerId, t.itemId);
    string itemName = it ? it->getName() : ("(unknown item, localID:" + to_string(t.itemId) + ")");
    cout << "Transaction ID: " << t.id << "\n";
    cout << "Item     : " << itemName << " (localID:" << t.itemId << ")\n";
    cout << "Buyer ID : " << t.buyerId << "\n";
    cout << "Seller ID: " << t.sellerId << "\n";
    cout << "Quantity : " << t.quantity << "\n";
    cout << "Total    : Rp" << fixed << setprecision(0) << t.total << "\n";
    cout << "Status   : " << status_to_string(t.status) << "\n";
    cout << "Date     : " << t.datetime << "\n";
    sep_line();
}

Item* find_seller_item_by_name(Database &db, int sellerId, const string &name) {
    for (auto &it : db.items) {
        if (it.getSellerId() == sellerId && it.getName() == name) return &it;
    }
    return nullptr;
}
bool print_seller_inventory(Database &db, int sellerId) {
    header_line();
    cout << "        YOUR INVENTORY\n";
    sep_line();
    cout << left << setw(6) << "LID" << setw(20) << "NAME" << setw(8) << "STOCK" << setw(14) << "PRICE\n";
    sep_line();
    bool any = false;
    for (auto &it : db.items) {
        if (it.getSellerId() == sellerId) {
            cout << left << setw(6) << it.getId()
                 << setw(20) << it.getName()
                 << setw(8) << it.getQuantity()
                 << "Rp" << fixed << setprecision(0) << it.getPrice() << "\n";
            any = true;
        }
    }
    sep_line();
    return any;
}

void bank_list_transactions_week(Database &db) {
    header_line(); cout << "Bank: Transactions (last 7 days)\n"; sep_line();
    time_t now = time(nullptr);
    bool any = false;
    for (auto &t: db.transactions) {
        time_t tt = parse_datetime_local(t.datetime);
        if (tt==0) continue;
        double diff = difftime(now, tt);
        if (diff >= 0 && diff <= 7.0*24*3600) {
            print_transaction_readable(db, t);
            any = true;
        }
    }
    if (!any) cout << "No transactions in the last 7 days.\n";
    press_enter_to_continue();
}

void bank_list_customers(Database &db) {
    header_line(); cout << "Bank Customers\n"; sep_line();
    if (db.bankCustomers.empty()) cout << "No bank customers.\n";
    else {
        for (auto &bc : db.bankCustomers) {
            cout << "BankID: " << bc.getId() << " | Name: " << bc.getName() << " | Balance: Rp"
                 << fixed << setprecision(0) << bc.getBalance() << "\n";
        }
    }
    sep_line();
    press_enter_to_continue();
}

void bank_list_dormant(Database &db) {
    header_line(); cout << "Bank: Dormant accounts (no tx 30 days)\n"; sep_line();
    time_t now = time(nullptr);
    bool any=false;
    for (auto &bc : db.bankCustomers) {
        bool active=false;
        for (auto &buyer : db.buyers) {
            if (buyer.getBankAccountId() != bc.getId()) continue;
            for (auto &t : db.transactions) {
                if (t.buyerId != buyer.getId()) continue;
                time_t tt = parse_datetime_local(t.datetime);
                if (tt==0) continue;
                double diff = difftime(now, tt);
                if (diff >=0 && diff <= 30.0*24*3600) { 
                    active = true; 
                    break; 
                }
            }
            if (active) break;
        }
        if (!active) {
            cout << "BankID: " << bc.getId() << " | Name: " << bc.getName() << " | Balance: Rp"
                 << fixed << setprecision(0) << bc.getBalance() << "\n";
            any=true;
        }
    }
    if (!any) cout << "No dormant accounts.\n";
    sep_line();
    press_enter_to_continue();
}

void bank_list_top_n_today(Database &db) {
    cout << "Enter N (0 cancel): ";
    int n = safe_int_input();
    if (n==0) { cout << "Cancelled\n"; return; }
    time_t now = time(nullptr);
    map<int,int> cnt;
    for (auto &t : db.transactions) if (same_day_local(t.datetime, now)) cnt[t.buyerId]++;
    vector<pair<int,int>> arr(cnt.begin(), cnt.end());
    sort(arr.begin(), arr.end(), [](auto &a, auto &b){ return a.second > b.second; });
    header_line(); cout << "Top " << n << " users today\n"; sep_line();
    for (int i=0;i<(int)arr.size() && i<n;i++) {
        Buyer* b = db.findBuyerById(arr[i].first);
        string name = b? b->getName() : "Unknown";
        cout << (i+1) << ". BuyerID: " << arr[i].first << " | Name: " << name << " | TxCount: " << arr[i].second << "\n";
    }
    if (arr.empty()) cout << "No transactions today.\n";
    sep_line();
    press_enter_to_continue();
}

void seller_add_item(Database &db, int sellerId) {
    header_line(); cout << "Add Item (enter 0 at name to cancel)\n"; sep_line();
    cout << "Item name: ";
    string name; 
    getline(cin, name); 
    if (name == "0") { 
        cout << "Cancelled\n"; 
        return; 
    }

    cout << "Quantity: "; 
    int q = safe_int_input(); 
    if (q==0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    cout << "Price (Rp): ";
    double p = safe_double_input(); 
    if (p==0.0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    Item* existing = find_seller_item_by_name(db, sellerId, name);
    if (existing) {
        existing->setQuantity(existing->getQuantity() + q);
        existing->setPrice(p);
        cout << "Item exists - updated quantity and price.\n";
    } else {
        int maxLocalId = 0;
        for (auto &it : db.items) {
            if (it.getSellerId() == sellerId && it.getId() > maxLocalId) maxLocalId = it.getId();
        }
        int newLocalId = maxLocalId + 1;
        Item it(newLocalId, sellerId, name, q, p);
        db.items.push_back(it);
        cout << "Item added. Local ID: " << newLocalId << "\n";
    }
    db.saveAll();
    sep_line();
    press_enter_to_continue();
}

void seller_update_item(Database &db, int sellerId) {
    header_line(); cout << "Update Item (your inventory)\n"; sep_line();
    bool any = print_seller_inventory(db, sellerId);

    if (!any) { 
        cout << "No items to update.\n"; 
        press_enter_to_continue(); 
        return; 
    }

    cout << "Enter Local ItemID to update (0 cancel): ";
    int iid = safe_int_input(); 
    if (iid==0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    Item* it = findItemBySellerLocalId(db, sellerId, iid);
    if (!it) { 
        cout << "Item not found or not yours.\n"; 
        return; 
    }

    cout << "New quantity: "; 
    int q = safe_int_input(); 
    if (q==0) { cout << "Cancelled\n"; 
        return; 
    }

    cout << "New price (Rp): "; 
    double p = safe_double_input(); 
    if (p==0.0) { cout << "Cancelled\n"; 
        return; 
    }

    it->setQuantity(q);
    it->setPrice(p);
    db.saveAll();
    cout << "Item updated.\n";
    press_enter_to_continue();
}

void seller_remove_item(Database &db, int sellerId) {
    header_line(); cout << "Remove Item (your inventory)\n"; sep_line();
    bool any = print_seller_inventory(db, sellerId);
    if (!any) { 
        cout << "No items to remove.\n"; 
        press_enter_to_continue(); 
        return; 
    }

    cout << "Enter Local ItemID to remove (0 cancel): ";
    int iid = safe_int_input(); 
    if (iid==0) { 
        cout << "Cancelled\n"; 
        return; 
    }
    auto it = find_if(db.items.begin(), db.items.end(), [&](const Item &x){ 
        return x.getSellerId()==sellerId && x.getId()==iid; });
    if (it == db.items.end()) { 
        cout << "Item not found or not yours.\n"; 
        return; 
    }

    db.items.erase(it);
    db.saveAll();
    cout << "Item removed.\n";
    press_enter_to_continue();
}

void buyer_browse_and_buy(Database &db, Buyer &buyer) {
    header_line(); cout << "Available Stores\n"; sep_line();
    if (db.sellers.empty()) { 
        cout << "No stores available.\n"; 
        press_enter_to_continue(); 
        return; 
    }

    for (auto &s : db.sellers) {
        cout << "SellerID: " << s.getId() << " | Store: " << s.getStoreName() << "\n";
    }
    sep_line();
    cout << "Choose SellerID to browse (0 cancel): ";
    int sid = safe_int_input(); 
    if (sid==0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    Seller* sel = db.findSellerById(sid);
    if (!sel) { 
        cout << "Seller not found.\n"; 
        return; 
    }

    header_line(); cout << "Items in store: " << sel->getStoreName() << "\n"; sep_line();
    bool any = false;
    for (auto &it : db.items) {
        if (it.getSellerId() == sel->getId()) {
            cout << "LocalID:" << it.getId() << " | " << setw(20) << left << it.getName()
                 << " | Qty:" << setw(4) << it.getQuantity()
                 << " | Price: Rp" << fixed << setprecision(0) << it.getPrice() << "\n";
            any = true;
        }
    }
    if (!any) cout << "(No items in this store)\n";
    sep_line();

    if (!any) { press_enter_to_continue(); return; }

    cout << "Enter Local ItemID to buy (0 cancel): ";
    int iid = safe_int_input(); 
    if (iid==0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    Item* it = findItemBySellerLocalId(db, sel->getId(), iid);
    if (!it) { 
        cout << "Item not found in this store.\n"; 
        return; 
    }

    cout << "Quantity (0 cancel): "; 
    int q = safe_int_input(); 
    if (q==0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    if (q > it->getQuantity()) { 
        cout << "Not enough stock.\n"; 
        return; 
    }

    if (buyer.getBankAccountId() == 0) { 
        cout << "You have no bank account. Please topup/create first.\n"; 
        return; 
    }

    BankCustomer* bc = db.findBankById(buyer.getBankAccountId());
    if (!bc) { 
        cout << "Buyer bank record missing.\n"; 
        return; 
    }

    double total = q * it->getPrice();
    if (bc->getBalance() < total) { 
        cout << "Insufficient funds. Total: Rp" << fixed << setprecision(0) << total << "\n"; 
        return; 
    }

    bc->withdraw(total);
    Buyer* sellerBuyerLink = db.findBuyerById(sel->getBuyerId());
    if (sellerBuyerLink && sellerBuyerLink->getBankAccountId() != 0) {
        BankCustomer* sbc = db.findBankById(sellerBuyerLink->getBankAccountId());
        if (sbc) sbc->addBalance(total);
    }
    it->setQuantity(it->getQuantity() - q);

    int maxLocalTxId = 0;
    for (auto &tx : db.transactions) {
        if (tx.sellerId == sel->getId() && tx.id > maxLocalTxId)
            maxLocalTxId = tx.id;
    }
    int newLocalTxId = maxLocalTxId + 1;

    Transaction tr;
    tr.id = newLocalTxId;        
    tr.buyerId = buyer.getId();
    tr.sellerId = sel->getId();
    tr.itemId = it->getId();  
    tr.quantity = q;
    tr.total = total;
    tr.status = TransStatus::PAID;
    tr.datetime = now_datetime_string();
    db.transactions.push_back(tr);
    db.saveAll();

    cout << "\nPembelian berhasil! Detail:\n";
    print_transaction_readable(db, tr);
    press_enter_to_continue();
}

void buyer_view_orders(Database &db, Buyer &buyer) {
    header_line(); cout << "Your Orders\n"; sep_line();
    cout << "Filter:\n";
    cout << "1) Show All\n";
    cout << "2) Only PAID\n";
    cout << "3) Only COMPLETED\n";
    cout << "4) Only CANCELLED\n";
    cout << "5) Back\n";
    cout << "Choose: ";
    int f = safe_int_input(); if (f==0) { 
        cout << "Cancelled\n"; 
        return; 
    }
    bool any=false;
    for (auto &t : db.transactions) {
        if (t.buyerId != buyer.getId()) continue;
        if (f == TF_ALL) { 
            print_transaction_readable(db, t); 
            any=true; 
        }
        else if (f == TF_PAID && t.status == TransStatus::PAID) { 
            print_transaction_readable(db, t); 
            any=true; 
        }
        else if (f == TF_COMPLETED && t.status == TransStatus::COMPLETED) { 
            print_transaction_readable(db, t); 
            any=true; 
        }
        else if (f == TF_CANCELLED && t.status == TransStatus::CANCELLED) { 
            print_transaction_readable(db, t); 
            any=true; 
        }
    }
    if (!any) cout << "No orders match filter.\n";
    press_enter_to_continue();
}

void buyer_check_spending_k_days(Database &db, Buyer &buyer) {
    cout << "Enter k (days) (0 cancel): ";
    int k = safe_int_input(); 
    if (k==0) { 
        cout << "Cancelled\n"; 
        return; 
    }
    time_t now = time(nullptr);
    double sum = 0;
    for (auto &t : db.transactions) {
        if (t.buyerId != buyer.getId()) continue;
        time_t tt = parse_datetime_local(t.datetime);
        if (tt==0) continue;
        double diff = difftime(now, tt);
        if (diff >=0 && diff <= (double)k*24*3600) sum += t.total;
    }
    header_line();
    cout << "Spending last " << k << " days: Rp" << fixed << setprecision(0) << sum << "\n";
    sep_line();
    press_enter_to_continue();
}

void seller_view_transactions(Database &db, int sellerId) {
    header_line(); cout << "Seller Transactions (filtered)\n"; sep_line();
    while (true) {
        cout << "Filter:\n";
        cout << "1) Show All\n";
        cout << "2) Only PAID\n";
        cout << "3) Only COMPLETED\n";
        cout << "4) Only CANCELLED\n";
        cout << "5) Back\n";
        cout << "Choose: ";
        int f = safe_int_input();
        if (f == TF_BACK) break;
        bool any=false;
        for (auto &t : db.transactions) {
            if (t.sellerId != sellerId) continue;
            if (f == TF_ALL) { 
                print_transaction_readable(db, t); 
                any=true; 
            }
            else if (f == TF_PAID && t.status == TransStatus::PAID) { 
                print_transaction_readable(db, t); 
                any=true; 
            }
            else if (f == TF_COMPLETED && t.status == TransStatus::COMPLETED) { 
                print_transaction_readable(db, t); 
                any=true; 
            }
            else if (f == TF_CANCELLED && t.status == TransStatus::CANCELLED) { 
                print_transaction_readable(db, t); 
                any=true; 
            }
        }
        if (!any) cout << "No transactions match filter.\n";
        press_enter_to_continue();
    }
}

void seller_update_transaction_status(Database &db, int sellerId) {
    header_line(); cout << "PAID Transactions for your store\n"; sep_line();
    vector<int> paidIds;
    for (auto &t : db.transactions) {
        if (t.sellerId == sellerId && t.status == TransStatus::PAID) {
            print_transaction_readable(db, t);
            paidIds.push_back(t.id);
        }
    }
    if (paidIds.empty()) {
        header_line(); cout << "   NO PAID TRANSACTIONS\n"; sep_line();
        cout << "Press 0 to go back: ";
        int z = safe_int_input();
        if (z==0) return;
        else { cout << "Returning.\n"; return; }
    }

    cout << "Enter Local Transaction ID to update (0 cancel): ";
    int tid = safe_int_input(); 
    if (tid==0) { 
        cout << "Cancelled\n"; 
        return; 
    }
    if (find(paidIds.begin(), paidIds.end(), tid) == paidIds.end()) {
        cout << "Transaction ID not in PAID list or not yours.\n"; 
        return;
    }

    Transaction* tr = findTransactionBySellerLocalId(db, sellerId, tid);
    if (!tr) { cout << "Transaction not found (race condition).\n"; 
        return; 
    }

    cout << "Choose:\n";
    cout << "1) Mark as COMPLETED\n";
    cout << "2) Mark as CANCELLED (refund)\n";
    cout << "3) Cancel\n";
    cout << "Choose: ";
    int ch = safe_int_input();
    if (ch == 3) { 
        cout << "Cancelled\n"; 
        return; 
    }
    if (ch == 1) {
        tr->status = TransStatus::COMPLETED;
        db.saveAll();
        cout << "Transaction marked COMPLETED.\n";
        return;
    }
    if (ch == 2) {
        Buyer* buyer = db.findBuyerById(tr->buyerId);
        if (!buyer) { 
            cout << "Buyer missing; marking CANCELLED only.\n"; 
            tr->status = TransStatus::CANCELLED; 
            db.saveAll(); 
            return; 
        }

        double refund = tr->total;
        if (buyer->getBankAccountId() == 0) {
            cout << "Buyer has no bank account; cannot refund automatically. Marking CANCELLED only.\n";
            tr->status = TransStatus::CANCELLED; 
            db.saveAll(); 
            return;
        }

        BankCustomer* buyerBank = db.findBankById(buyer->getBankAccountId());
        if (!buyerBank) { 
            cout << "Buyer bank missing; mark CANCELLED only.\n"; 
            tr->status = TransStatus::CANCELLED; 
            db.saveAll(); 
            return; 
        }

        Seller* seller = db.findSellerById(sellerId);
        int sellerBankId = 0;
        if (seller) {
            Buyer* link = db.findBuyerById(seller->getBuyerId());
            if (link) sellerBankId = link->getBankAccountId();
        }

        if (sellerBankId != 0) {
            BankCustomer* sbc = db.findBankById(sellerBankId);
            if (sbc) {
                sbc->addBalance(-refund);
                cout << "Seller bank (BankID " << sbc->getId() << ") debited. New seller balance: Rp" << fixed << setprecision(0) << sbc->getBalance() << "\n";
                if (sbc->getBalance() < 0.0) cout << "Warning: seller balance negative after refund.\n";
            } else {
                cout << "Seller bank record referenced but not found.\n";
            }
        } else {
            cout << "Seller has no bank account. System will credit buyer but seller not debited.\n";
        }

        buyerBank->addBalance(refund);
        cout << "Buyer refunded Rp" << fixed << setprecision(0) << refund << " (BankID " << buyerBank->getId() << ").\n";

        Item* item = findItemBySellerLocalId(db, tr->sellerId, tr->itemId);
        if (item) {
            item->setQuantity(item->getQuantity() + tr->quantity);
            cout << "Item stock restored: +" << tr->quantity << " (" << item->getName() << ")\n";
        } else {
            cout << "Warning: item not found, cannot restore stock.\n";
        }

        tr->status = TransStatus::CANCELLED;
        db.saveAll();

        cout << "Transaction marked as CANCELLED.\n";
        return;
    }
    cout << "Invalid option.\n";
}

void store_list_latest_k_days(Database &db, int sellerId) {
    cout << "Enter k (days) (0 cancel): ";
    int k = safe_int_input(); 
    if (k==0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    time_t now = time(nullptr);
    bool any = false;
    header_line(); cout << "Transactions for last " << k << " days\n"; sep_line();
    for (auto &t : db.transactions) {
        if (t.sellerId != sellerId) continue;
        time_t tt = parse_datetime_local(t.datetime);
        if (tt==0) continue;
        double diff = difftime(now, tt);
        if (diff >= 0 && diff <= (double)k*24*3600) {
            print_transaction_readable(db, t);
            any = true;
        }
    }
    if (!any) cout << "No transactions in the given period.\n";
    sep_line();
    press_enter_to_continue();
}

void store_list_paid_not_completed(Database &db, int sellerId) {
    header_line(); cout << "Paid but not completed transactions\n"; sep_line();
    bool any = false;
    for (auto &t : db.transactions) {
        if (t.sellerId == sellerId && t.status == TransStatus::PAID) {
            print_transaction_readable(db, t);
            any = true;
        }
    }
    if (!any) cout << "No PAID transactions found.\n";
    sep_line();
    press_enter_to_continue();
}

void store_most_frequent_items(Database &db, int sellerId) {
    cout << "Enter m (top items, 0 cancel): ";
    int m = safe_int_input(); 
    if (m==0) { 
        cout << "Cancelled\n"; 
        return; 
    }

    map<int,int> cnt;
    for (auto &t : db.transactions) {
        if (t.sellerId != sellerId) continue;
        cnt[t.itemId] += t.quantity;
    }
    vector<pair<int,int>> arr(cnt.begin(), cnt.end());
    sort(arr.begin(), arr.end(), [](const pair<int,int> &a, const pair<int,int> &b){ return a.second > b.second; });
    header_line(); cout << "Top " << m << " frequent items\n"; sep_line();
    for (int i = 0; i < (int)arr.size() && i < m; ++i) {
        Item* it = findItemBySellerLocalId(db, sellerId, arr[i].first);
        string name = it ? it->getName() : ("(localID:" + to_string(arr[i].first) + ")");
        cout << (i+1) << ". Item: " << name << " | QtySold: " << arr[i].second << "\n";
    }
    if (arr.empty()) cout << "No items sold yet.\n";
    sep_line();
    press_enter_to_continue();
}

void store_most_active_buyers(Database &db, int sellerId) {
    map<int,int> cnt;
    for (auto &t : db.transactions) {
        if (t.sellerId != sellerId) continue;
        cnt[t.buyerId] += 1;
    }
    vector<pair<int,int>> arr(cnt.begin(), cnt.end());
    sort(arr.begin(), arr.end(), [](const pair<int,int> &a, const pair<int,int> &b){ return a.second > b.second; });
    header_line(); cout << "Most active buyers for your store\n"; sep_line();
    for (int i = 0; i < (int)arr.size() && i < 10; ++i) {
        Buyer* b = db.findBuyerById(arr[i].first);
        string name = b ? b->getName() : ("(ID:" + to_string(arr[i].first) + ")");
        cout << (i+1) << ". BuyerID: " << arr[i].first << " | Name: " << name << " | TxCount: " << arr[i].second << "\n";
    }
    if (arr.empty()) cout << "No buyers yet.\n";
    sep_line();
    press_enter_to_continue();
}

void store_most_active_sellers(Database &db) {
    map<int,int> cnt;
    for (auto &t : db.transactions) {
        cnt[t.sellerId] += 1;
    }
    vector<pair<int,int>> arr(cnt.begin(), cnt.end());
    sort(arr.begin(), arr.end(), [](const pair<int,int> &a, const pair<int,int> &b){ return a.second > b.second; });
    header_line(); cout << "Most active sellers (by tx count)\n"; sep_line();
    for (int i = 0; i < (int)arr.size() && i < 10; ++i) {
        Seller* s = db.findSellerById(arr[i].first);
        string name = s ? s->getStoreName() : ("(SellerID:" + to_string(arr[i].first) + ")");
        cout << (i+1) << ". SellerID: " << arr[i].first << " | Store: " << name << " | TxCount: " << arr[i].second << "\n";
    }
    if (arr.empty()) cout << "No sellers with transactions yet.\n";
    sep_line();
    press_enter_to_continue();
}

void show_account_info(Database &db, Buyer &buyer) {
    header_line(); cout << "ACCOUNT INFO\n"; sep_line();
    cout << "ID      : " << buyer.getId() << "\n";
    cout << "Name    : " << buyer.getName() << "\n";
    cout << "Address : " << buyer.getAddress() << "\n";
    cout << "Phone   : " << buyer.getPhone() << "\n";
    cout << "Email   : " << buyer.getEmail() << "\n";
    if (buyer.getBankAccountId() == 0) {
        cout << "Bank    : (none)\n";
    } else {
        BankCustomer* bc = db.findBankById(buyer.getBankAccountId());
        if (bc) cout << "BankID  : " << bc->getId() << " | Balance: Rp" << fixed << setprecision(0) << bc->getBalance() << "\n";
        else cout << "Bank linked but record not found.\n";
    }
    Seller* s = nullptr;
    for (auto &ss : db.sellers) if (ss.getBuyerId() == buyer.getId()) { s = &ss; break; }
    if (s) {
        sep_line();
        cout << "Seller info:\n";
        cout << "SellerID: " << s->getId() << "\n";
        cout << "Store   : " << s->getStoreName() << "\n";
    }
    sep_line();
    press_enter_to_continue();
}

int main() {
    Database db;
    db.loadAll();

    header_line(); cout << "TERMINAL STORE APP (LOCAL IDS) - FINAL\n"; sep_line();

    while (true) {
        cout << "\nMain Menu:\n";
        cout << "1) Login\n";
        cout << "2) Register\n";
        cout << "3) Exit\n";
        cout << "Choose: ";
        int opt = safe_int_input();

        if (opt == MM_LOGIN) {
            cout << "Enter ID (0 cancel): ";
            int id = safe_int_input(); if (id==0) { 
                cout << "Cancelled\n"; 
                continue; 
            }
            cout << "Enter Name: ";
            string name; getline(cin, name);
            Buyer* buyer = db.findBuyerById(id);
            if (!buyer) { 
                cout << "Buyer not found.\n"; 
                continue; 
            }
            if (buyer->getName() != name) { 
                cout << "Name does not match ID. Login failed.\n"; 
                continue; 
            }

            Seller* selfSeller = nullptr;
            for (auto &s : db.sellers) if (s.getBuyerId() == buyer->getId()) { selfSeller = &s; break; }

            bool logged_in = true;
            while (logged_in) {
                header_line(); cout << "USER MENU\n"; sep_line();
                cout << "1) Account Info/n";
                cout << "2) Shop Menu\n";
                cout << "3) Bank Menu\n";
                cout << "4) Seller Menu\n";
                cout << "5) Upgrade to Seller\n";
                cout << "6) Logout\nChoose: ";
                int choice = safe_int_input();

                if (choice == PL_ACCOUNT_INFO) {
                    show_account_info(db, *buyer);
                }
                else if (choice == PL_SHOP) {
                    bool inShop = true;
                    while (inShop) {
                        cout << "\nShop Menu:\n"
                             << "1) Browse & Buy\n"
                             << "2) View Orders\n"
                             << "3) Check Spending (k days)\n"
                             << "4) Back\n"
                             << "Choose: ";
                        int sc = safe_int_input();
                        if (sc == SH_BROWSE) buyer_browse_and_buy(db, *buyer);
                        else if (sc == SH_ORDERS) buyer_view_orders(db, *buyer);
                        else if (sc == SH_SPENDING) buyer_check_spending_k_days(db, *buyer);
                        else if (sc == SH_BACK) inShop = false;
                        else cout << "Invalid option\n";
                    }
                }
                else if (choice == PL_BANK) {
                    bool inBank = true;
                    while (inBank) {
                        cout << "\nBank Menu:\n"
                             << "1) Topup\n"
                             << "2) Withdraw\n"
                             << "3) View Cashflow Today\n"
                             << "4) View Cashflow Month\n"
                             << "5) List Transactions (last week)\n"
                             << "6) List Bank Customers\n"
                             << "7) List Dormant Accounts (1 month)\n"
                             << "8) List Top-N Users Today\n"
                             << "9) Back\n"
                             << "Choose: ";
                        int bm = safe_int_input();
                        if (bm == BM_TOPUP) {
                            cout << "Topup amount (0 cancel): ";
                            double amt = safe_double_input(); 
                            if (amt==0.0) { 
                                cout << "Cancelled\n"; 
                                continue; 
                            }

                            if (buyer->getBankAccountId() == 0) {
                                BankCustomer bc(db.nextBankId++, buyer->getName(), amt);
                                db.bankCustomers.push_back(bc);
                                buyer->setBankAccountId(bc.getId());
                                db.saveAll();
                                cout << "Bank account created and topped up. ID=" << bc.getId() << "\n";
                            } else {
                                BankCustomer* bc = db.findBankById(buyer->getBankAccountId());
                                if (!bc) { 
                                    cout << "Bank missing\n"; 
                                    continue; 
                                }
                                bc->addBalance(amt);
                                db.saveAll();
                                cout << "Topup success. New bal: Rp" << fixed << setprecision(0) << bc->getBalance() << "\n";
                            }
                        }
                        else if (bm == BM_WITHDRAW) {
                            if (buyer->getBankAccountId() == 0) { cout << "No bank account\n"; continue; }
                            BankCustomer* bc = db.findBankById(buyer->getBankAccountId());
                            if (!bc) { 
                                cout << "Bank missing\n"; 
                                continue; 
                            }
                            cout << "Withdraw amount (0 cancel): ";
                            double amt = safe_double_input(); 
                            if (amt==0.0) { 
                                cout << "Cancelled\n"; 
                                continue; 
                            }
                            if (!bc->withdraw(amt)) { 
                                cout << "Insufficient funds\n"; 
                                continue; 
                            }
                            db.saveAll();
                            cout << "Withdraw success. New bal: Rp" << fixed << setprecision(0) << bc->getBalance() << "\n";
                        }
                        else if (bm == BM_CASHFLOW_TODAY) {
                            if (buyer->getBankAccountId()==0) { 
                                cout << "No bank account\n"; 
                                continue; 
                            }
                            time_t now = time(nullptr);
                            double debit = 0;
                            for (auto &t : db.transactions) if (t.buyerId == buyer->getId() && same_day_local(t.datetime, now)) debit += t.total;
                            header_line(); cout << "Cashflow Today (approx)\n"; sep_line();
                            cout << "Debit (spent): Rp" << fixed << setprecision(0) << debit << " | Credit: Rp0\n"; sep_line();
                            press_enter_to_continue();
                        }
                        else if (bm == BM_CASHFLOW_MONTH) {
                            if (buyer->getBankAccountId()==0) { 
                                cout << "No bank account\n"; 
                                continue; 
                            }
                            time_t now = time(nullptr);
                            double debit = 0;
                            for (auto &t : db.transactions) {
                                if (t.buyerId != buyer->getId()) continue;
                                time_t tt = parse_datetime_local(t.datetime);
                                if (tt==0) continue;
                                double diff = difftime(now, tt);
                                if (diff >=0 && diff <= 30.0*24*3600) debit += t.total;
                            }
                            header_line(); cout << "Cashflow Last 30 days (approx)\n"; sep_line();
                            cout << "Debit (spent): Rp" << fixed << setprecision(0) << debit << " | Credit: Rp0\n"; sep_line();
                            press_enter_to_continue();
                        }
                        else if (bm == BM_LIST_TRANSACTIONS_WEEK) bank_list_transactions_week(db);
                        else if (bm == BM_LIST_CUSTOMERS) bank_list_customers(db);
                        else if (bm == BM_LIST_DORMANT) bank_list_dormant(db);
                        else if (bm == BM_LIST_TOP_N) bank_list_top_n_today(db);
                        else if (bm == BM_BACK) inBank = false;
                        else cout << "Invalid option\n";
                    }
                }
                else if (choice == PL_SELLER) {
                    if (!selfSeller) { cout << "You are not a seller. Use Upgrade to Seller first.\n"; continue; }
                    bool inSeller = true;
                    while (inSeller) {
                        cout << "\nSeller Menu:\n"
                             << "1) Manage Items\n"
                             << "2) View Transactions\n"
                             << "3) Update Transaction Status\n"
                             << "4) Analytics\n"
                             << "5) Back\n"
                             << "Choose: ";
                        int so = safe_int_input();
                        if (so == SMM_MANAGE_ITEMS) {
                            bool inManage = true;
                            while (inManage) {
                                cout << "\nManage Items:\n"
                                     << "1) List Items\n"
                                     << "2) Add Item\n"
                                     << "3) Update Item\n"
                                     << "4) Remove Item\n"
                                     << "5) Back\n"
                                     << "Choose: ";
                                int mm = safe_int_input();
                                if (mm == SIM_LIST) {
                                    bool any = print_seller_inventory(db, selfSeller->getId());
                                    if (!any) cout << "(No items in inventory)\n";
                                    press_enter_to_continue();
                                }
                                else if (mm == SIM_ADD) seller_add_item(db, selfSeller->getId());
                                else if (mm == SIM_UPDATE) seller_update_item(db, selfSeller->getId());
                                else if (mm == SIM_REMOVE) seller_remove_item(db, selfSeller->getId());
                                else if (mm == SIM_BACK) inManage = false;
                                else cout << "Invalid option\n";
                            }
                        }
                        else if (so == SMM_VIEW_TX) seller_view_transactions(db, selfSeller->getId());
                        else if (so == SMM_UPDATE_TX_STATUS) seller_update_transaction_status(db, selfSeller->getId());
                        else if (so == SMM_ANALYTICS) {
                            bool inA = true;
                            while (inA) {
                                cout << "\nAnalytics:\n1) Latest K days\n2) Paid but not completed\n3) Most frequent items (m)\n4) Most active buyers\n5) Most active sellers\n6) Back\nChoose: ";
                                int a = safe_int_input();
                                if (a==1) store_list_latest_k_days(db, selfSeller->getId());
                                else if (a==2) store_list_paid_not_completed(db, selfSeller->getId());
                                else if (a==3) store_most_frequent_items(db, selfSeller->getId());
                                else if (a==4) store_most_active_buyers(db, selfSeller->getId());
                                else if (a==5) store_most_active_sellers(db);
                                else if (a==6) inA = false;
                                else cout << "Invalid option\n";
                            }
                        }
                        else if (so == SMM_BACK) inSeller = false;
                        else cout << "Invalid option\n";
                    }
                }
                else if (choice == PL_UPGRADE) {
                    bool already = false;
                    for (auto &s : db.sellers) if (s.getBuyerId() == buyer->getId()) { already=true; break; }
                    if (already) { 
                        cout << "You are already a seller.\n"; 
                        continue; 
                    }
                    cout << "Upgrade to Seller - enter store info (enter 0 at any prompt to cancel)\n";
                    cout << "Store name: "; string sname; getline(cin, sname); 
                    if (sname=="0") { 
                        cout<<"Cancelled\n"; 
                        continue; 
                    }
                    cout << "Store address: "; 
                    string saddr; 
                    getline(cin, saddr); 
                    if (saddr=="0") { 
                        cout<<"Cancelled\n"; 
                        continue; 
                    }
                    cout << "Store phone: "; 
                    string sphone; 
                    getline(cin, sphone); 
                    if (sphone=="0") { cout<<"Cancelled\n"; 
                        continue; 
                    }
                    cout << "Store email: "; 
                    string semail; 
                    getline(cin, semail); 
                    if (semail=="0") { 
                        cout<<"Cancelled\n"; 
                        continue; 
                    }
                    Seller ns(db.nextSellerId++, buyer->getId(), sname, saddr, sphone, semail);
                    db.sellers.push_back(ns);
                    db.saveAll();
                    cout << "Seller created with ID: " << ns.getId() << "\n";
                    selfSeller = db.findSellerById(ns.getId());
                }
                else if (choice == PL_LOGOUT) {
                    cout << "Logged out.\n";
                    logged_in = false;
                }
                else cout << "Invalid option\n";
            }

        } else if (opt == MM_REGISTER) {
            cout << "== Register Buyer (auto-creates bank account with balance 0) ==\n";
            cout << "Name: "; 
            string name; 
            getline(cin, name); 
            if (name=="0") { 
                cout << "Cancelled\n"; 
                continue; 
            }
            cout << "Address: "; 
            string addr; 
            getline(cin, addr); 
            if (addr=="0") { 
                cout << "Cancelled\n"; 
                continue; 
            }
            cout << "Phone: "; 
            string phone; 
            getline(cin, phone); 
            if (phone=="0") { 
                cout << "Cancelled\n";
                continue; 
            }
            cout << "Email: "; 
            string email; 
            getline(cin, email); 
            if (email=="0") { 
                cout << "Cancelled\n"; 
                continue; 
            }

            Buyer b(db.nextBuyerId++, name, addr, phone, email, 0);
            BankCustomer bc(db.nextBankId++, name, 0.0);
            db.bankCustomers.push_back(bc);
            b.setBankAccountId(bc.getId());
            db.buyers.push_back(b);
            db.saveAll();

            header_line(); cout << "Registered Successfully\n"; sep_line();
            cout << "Buyer ID: " << b.getId() << "\n";
            cout << "Bank account created (ID: " << bc.getId() << ") with balance Rp0\n";
            sep_line();
        }
        else if (opt == MM_EXIT) {
            cout << "Saving & exit...\n";
            db.saveAll();
            break;
        }
        else {
            cout << "Invalid option\n";
        }
    }

    return 0;
}
