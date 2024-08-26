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
    auto itAndOk = symbolOrders.emplace(add.price, add.size, add.orderId);
    orderIdToIterator[add.orderId] = itAndOk.first;
}

// NB: In the dataset, orders never change sides, I checked.
void BookBuilder::onUpdate(const CryptoUpdate& update) {
    auto& orders = update.side == Bid ? bids : offers;
    auto& symbolOrders = orders[update.symbol];

    auto pairIt = orderIdToIterator.find(update.orderId);
    if (pairIt == orderIdToIterator.end()) {
        return;
    }
    auto it = symbolOrders.emplace_hint(pairIt->second, update.price, update.size, update.orderId);
    symbolOrders.erase(pairIt->second);
    pairIt->second = it;
    // assert(false); // Could not find existing order to update!
}

void BookBuilder::onDelete(const CryptoDelete& delete_) {
    auto& orders = delete_.side == Bid ? bids : offers;
    auto& symbolOrders = orders[delete_.symbol];

    auto pairIt = orderIdToIterator.find(delete_.orderId);
    if (pairIt == orderIdToIterator.end()) {
        return;
    }
    symbolOrders.erase(pairIt->second);
    orderIdToIterator.erase(pairIt);
    // assert(false); // Could not find existing order to delete!
}

} // namespace wwhrt
