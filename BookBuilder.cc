#include "BookBuilder.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>
#include <utility>

namespace wwhrt {

std::vector<BookBuilder::Order> BookBuilder::getBestBids(Symbol symbol) {
    auto& symbolOrders = bids[symbol];
    std::vector<Order> bestBids;
    for (auto it = symbolOrders.rbegin(); it != symbolOrders.rend(); it++) {
        if (bestBids.empty() || it->price == bestBids.front().price) {
            bestBids.push_back(*it);
        } else {
            break;
        }
    }
    return bestBids;
}

std::vector<BookBuilder::Order> BookBuilder::getBestOffers(Symbol symbol) {
    auto& symbolOrders = offers[symbol];
    std::vector<Order> bestOffers;
    for (const Order& order : symbolOrders) {
        if (bestOffers.empty() || order.price == bestOffers.front().price) {
            bestOffers.push_back(order);
        } else {
            break;
        }
    }
    return bestOffers;
}

void BookBuilder::onAdd(const CryptoAdd& add) {
    auto& orders = add.side == Bid ? bids : offers;
    auto& symbolOrders = orders[add.symbol];
    symbolOrders.insert({add.price, add.size, add.orderId});
}

// NB: In the dataset, orders never change sides, I checked.
void BookBuilder::onUpdate(const CryptoUpdate& update) {
    auto& orders = update.side == Bid ? bids : offers;
    auto& symbolOrders = orders[update.symbol];
    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
        if (it->id == update.orderId) {
            symbolOrders.erase(it);
            symbolOrders.insert({update.price, update.size, update.orderId});
            return;
        }
    }
}

void BookBuilder::onDelete(const CryptoDelete& delete_) {
    auto& orders = delete_.side == Bid ? bids : offers;
    auto& symbolOrders = orders[delete_.symbol];

    {
        auto it = symbolOrders.find({delete_.price, delete_.size, delete_.orderId});
        if (it != symbolOrders.end()) {
            symbolOrders.erase(it);
            return;
        }
    }

    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); it++) {
        if (it->id == delete_.orderId) {
            symbolOrders.erase(it);
            return;
        }
    }
}

} // namespace wwhrt
