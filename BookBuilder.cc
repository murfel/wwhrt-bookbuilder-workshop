#include "BookBuilder.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>
#include <utility>

namespace wwhrt {

std::vector<BookBuilder::Order> BookBuilder::getBestBids(Symbol symbol) {
    std::vector<Order> bestBids;
#ifdef BASIC
    for (const Order& order : orders[symbol]) {
        if (order.side == Bid) {
            if (bestBids.empty() || order.price > bestBids[0].price) {
                bestBids.clear();
                bestBids.push_back(order);
            } else if (bestBids[0].price == order.price) {
                bestBids.push_back(order);
            }
        }
    }
#endif
#ifdef ASKBIDSPLIT
    for (const Order& order : bids[symbol]) {
        if (bestBids.empty() || order.price > bestBids[0].price) {
            bestBids.clear();
            bestBids.push_back(order);
        } else if (bestBids[0].price == order.price) {
            bestBids.push_back(order);
        }
    }
#endif
    return bestBids;
}

std::vector<BookBuilder::Order> BookBuilder::getBestOffers(Symbol symbol) {
    std::vector<Order> bestOffers;
#ifdef BASIC
    for (const Order& order : orders[symbol]) {
        if (order.side == Ask) {
            if (bestOffers.empty() || order.price < bestOffers[0].price) {
                bestOffers.clear();
                bestOffers.push_back(order);
            } else if (bestOffers[0].price == order.price) {
                bestOffers.push_back(order);
            }
        }
    }
#endif
#ifdef ASKBIDSPLIT
    for (const Order& order : offers[symbol]) {
        if (bestOffers.empty() || order.price < bestOffers[0].price) {
            bestOffers.clear();
            bestOffers.push_back(order);
        } else if (bestOffers[0].price == order.price) {
            bestOffers.push_back(order);
        }
    }
#endif
    return bestOffers;
}

void BookBuilder::onAdd(const CryptoAdd& add) {
#ifdef BASIC
    orders[add.symbol].push_back(Order{.price = add.price,
                                       .size = add.size,
                                       .id = add.orderId,
                                       .side = add.side});
#endif
#ifdef ASKBIDSPLIT
    if (add.side == Bid) {
        bids[add.symbol].push_back(Order{.price = add.price,
                .size = add.size,
                .id = add.orderId});
    } else {
        offers[add.symbol].push_back(Order{.price = add.price,
                .size = add.size,
                .id = add.orderId});
    }
#endif
}

// NB: In the dataset, orders never change sides, I checked.
void BookBuilder::onUpdate(const CryptoUpdate& update) {
#ifdef BASIC
    auto& symbolOrders = orders[update.symbol];
    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
        if (it->id == update.orderId) {
            it->price = update.price;
            it->size = update.size;
            it->side = update.side;
            return;
        }
    }
#endif
#ifdef ASKBIDSPLIT
    if (update.side == Bid) {
        auto& symbolOrders = bids[update.symbol];
        for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
            if (it->id == update.orderId) {
                it->price = update.price;
                it->size = update.size;
                return;
            }
        }
    } else {
        auto& symbolOrders = offers[update.symbol];
        for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
            if (it->id == update.orderId) {
                it->price = update.price;
                it->size = update.size;
                return;
            }
        }
    }

#endif
    // assert(false); // Could not find existing order to update!
}

void BookBuilder::onDelete(const CryptoDelete& delete_) {
#ifdef BASIC
    auto& symbolOrders = orders[delete_.symbol];
    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
        if (it->id == delete_.orderId) {
            symbolOrders.erase(it);
            return;
        }
    }
#endif
#ifdef ASKBIDSPLIT
    if (delete_.side == Bid) {
        auto& symbolOrders = bids[delete_.symbol];
        for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
            if (it->id == delete_.orderId) {
                symbolOrders.erase(it);
                return;
            }
        }
    } else {
        auto& symbolOrders = offers[delete_.symbol];
        for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
            if (it->id == delete_.orderId) {
                symbolOrders.erase(it);
                return;
            }
        }
    }
#endif
    // assert(false); // Could not find existing order to delete!
}

} // namespace wwhrt
