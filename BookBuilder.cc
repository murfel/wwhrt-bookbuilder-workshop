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
    auto& symbolOrders = orders[symbol];
    std::vector<Order> bestBids;
    for (auto it = symbolOrders.rbegin(); it != symbolOrders.rend(); it++) {
#ifndef ASKBIDSPLIT
        if (it->side != Bid) {
            continue;
        }
#endif
#if (defined(ORDERS_LIST) || defined(ORDERS_VECTOR)) && !defined(SORTED)
        if (bestBids.empty() || it->price > bestBids[0].price) {
            bestBids.clear();
            bestBids.push_back(*it);
        } else if (bestBids[0].price == it->price) {
            bestBids.push_back(*it);
        }
#else  // defined(SORTED) || defined(ORDERS_SET)
        if (bestBids.empty() || it->price == bestBids.front().price) {
            bestBids.push_back(*it);
        } else {
            break;
        }
#endif
    }
    return bestBids;
}

std::vector<BookBuilder::Order> BookBuilder::getBestOffers(Symbol symbol) {
#ifdef ASKBIDSPLIT
    auto& orders = offers;
#endif
    auto& symbolOrders = orders[symbol];
    std::vector<Order> bestOffers;
    for (const Order& order : symbolOrders) {
#ifndef ASKBIDSPLIT
        if (order.side != Ask) {
            continue;
        }
#endif
#if (defined(ORDERS_LIST) || defined(ORDERS_VECTOR)) && !defined(sorted)
        if (bestOffers.empty() || order.price < bestOffers[0].price) {
            bestOffers.clear();
            bestOffers.push_back(order);
        } else if (bestOffers[0].price == order.price) {
            bestOffers.push_back(order);
        }
#else // defined(SORTED) || defined(ORDERS_SET)
        if (bestOffers.empty() || order.price == bestOffers.front().price) {
            bestOffers.push_back(order);
        } else {
            break;
        }
#endif
    }
    return bestOffers;
}

#if defined(SORTED) || defined(ORDERS_SET)
void BookBuilder::insert(OrderContainer & symbolOrders, double price, double size, uint64_t orderId, Side side) {
#ifndef ASKBIDSPLIT
    Order order{price, size, orderId, side};
#else
    (void)side;
    Order order{price, size, orderId};
#endif
#if defined(ORDERS_LIST)
    // Find first >= order.
    auto it = symbolOrders.begin();
    while (it != symbolOrders.end() && *it < order) {
        it++;
    }
    symbolOrders.insert(it, order);
#elif defined(ORDERS_VECTOR)
    auto it = std::lower_bound(symbolOrders.begin(), symbolOrders.end(), order);
    symbolOrders.insert(it, order);
#elif defined(ORDERS_SET)
    symbolOrders.insert(order);
#endif
}
#endif

void BookBuilder::onAdd(const CryptoAdd& add) {
#ifdef ASKBIDSPLIT
    auto& orders = add.side == Bid ? bids : offers;
#endif
    auto& symbolOrders = orders[add.symbol];
#if (defined(ORDERS_LIST) || defined(ORDERS_VECTOR)) && !defined(SORTED)
    symbolOrders.push_back(Order{.price = add.price,
                                 .size = add.size,
                                 .id = add.orderId,
#ifndef ASKBIDSPLIT
                                 .side = add.side
#endif
                            });
#else
    insert(symbolOrders, add.price, add.size, add.orderId, add.side);
#endif
}

// NB: In the dataset, orders never change sides, I checked.
void BookBuilder::onUpdate(const CryptoUpdate& update) {
#ifdef ASKBIDSPLIT
    auto &orders = update.side == Bid ? bids : offers;
#endif
    auto &symbolOrders = orders[update.symbol];

    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); ++it) {
        if (it->id == update.orderId) {
#if (defined(ORDERS_LIST) || defined(ORDERS_VECTOR)) && !defined(SORTED)
            it->price = update.price;
            it->size = update.size;
#else
            // TODO: Hinted insert, since the price is likely changed only a little.
            symbolOrders.erase(it);
            insert(symbolOrders, update.price, update.size, update.orderId, update.side);
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

#if defined(ORDERS_SET) && defined(ATTEMPT_ERASE)
    // Attempt to erase in log time, if the supplied order is an exact match.
    // Otherwise, fallback to linear search by orderId, if price and size do not match.
    {
        auto it = symbolOrders.find({delete_.price, delete_.size, delete_.orderId});
        if (it != symbolOrders.end()) {
            symbolOrders.erase(it);
            return;
        }
    }
#endif

    for (auto it = symbolOrders.begin(); it != symbolOrders.end(); it++) {
        if (it->id == delete_.orderId) {
#if !defined(SWAPREMOVE) || defined(ORDERS_SET)
            symbolOrders.erase(it);
#elif defined(SWAPREMOVE)
            std::swap(*it, symbolOrders.back());
            symbolOrders.pop_back();
#endif
            return;
        }
    }
    // assert(false); // Could not find existing order to delete!
}

} // namespace wwhrt
