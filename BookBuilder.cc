#include "BookBuilder.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>
#include <utility>

namespace wwhrt {

std::vector<BookBuilder::Order> BookBuilder::getBestBids(Symbol symbol) {
#ifdef ASKBIDSPLIT
    auto& orders = bids;
#endif
    std::vector<Order> bestBids;
    for (const Order& order : orders[symbol]) {
#ifndef ASKBIDSPLIT
        if (order.side != Bid) {
            continue;
        }
#endif
        if (bestBids.empty() || order.price > bestBids[0].price) {
            bestBids.clear();
            bestBids.push_back(order);
        } else if (bestBids[0].price == order.price) {
            bestBids.push_back(order);
        }
    }
    return bestBids;
}

std::vector<BookBuilder::Order> BookBuilder::getBestOffers(Symbol symbol) {
#ifdef ASKBIDSPLIT
    auto& orders = offers;
#endif
    std::vector<Order> bestOffers;
    for (const Order& order : orders[symbol]) {
#ifndef ASKBIDSPLIT
        if (order.side != Ask) {
            continue;
        }
#endif
        if (bestOffers.empty() || order.price < bestOffers[0].price) {
            bestOffers.clear();
            bestOffers.push_back(order);
        } else if (bestOffers[0].price == order.price) {
            bestOffers.push_back(order);
        }
    }
    return bestOffers;
}

void BookBuilder::onAdd(const CryptoAdd& add) {
#ifdef ASKBIDSPLIT
    auto& orders = add.side == Bid ? bids : offers;
#endif
    auto& symbolOrders = orders[add.symbol];
    symbolOrders.push_back(Order{.price = add.price,
                                 .size = add.size,
                                 .id = add.orderId,
#ifndef ASKBIDSPLIT
                                 .side = add.side
#endif
                            });
}

// NB: In the dataset, orders never change sides, I checked.
void BookBuilder::onUpdate(const CryptoUpdate& update) {
#ifdef ASKBIDSPLIT
    auto& orders = update.side == Bid ? bids : offers;
#endif
    auto& symbolOrders = orders[update.symbol];
    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
        if (it->id == update.orderId) {
            it->price = update.price;
            it->size = update.size;
#ifndef ASKBIDSPLIT
            it->side = update.side;
#endif
            return;
        }
    }
    // assert(false); // Could not find existing order to update!
}

void BookBuilder::onDelete(const CryptoDelete& delete_) {
#ifdef ASKBIDSPLIT
    auto& orders = delete_.side == Bid ? bids : offers;
#endif
    auto& symbolOrders = orders[delete_.symbol];
    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
        if (it->id == delete_.orderId) {
#ifndef SWAPREMOVE
            symbolOrders.erase(it);
#else
            std::swap(*it, symbolOrders.back());
            symbolOrders.pop_back();
#endif
            return;
        }
    }
    // assert(false); // Could not find existing order to delete!
}

} // namespace wwhrt
