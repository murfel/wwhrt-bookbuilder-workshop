#pragma once

#include "Allocator.h"
#include "Events.h"
#include "Reader.h"

#include <list>
#include <unordered_map>
#include <utility>
#include <vector>

namespace wwhrt {

// BookBuilder Class
//
// This class is responsible for processing events that represent
// updates to the order book, in order to maintain the book state.
// The book state consists of all the active orders on the book, as
// defined by the Order struct below. This class must be able to
// return all orders at the Best Bid (side=Bid, highest price) and
// Best Offer (side=Ask, lowest price), also known as the BBO.
//
// Please modify the below class implementation.
class BookBuilder : public Subscriber {
public:
    struct Order {
        double price;
        double size;
        uint64_t id;

        Order(double price, double size, uint64_t id) : price(price), size(size), id(id) {}

        bool operator<(const Order& ord) const {
            if (price != ord.price) {
                return price < ord.price;
            }
            if (size != ord.size) {
                return size < ord.size;
            }
            return id < ord.id;
        }
    };
    BookBuilder() = default;
    std::vector<Order> getBestBids(Symbol symbol);
    std::vector<Order> getBestOffers(Symbol symbol);

    void onAdd(const CryptoAdd& add) final;
    void onUpdate(const CryptoUpdate& update) final;
    void onDelete(const CryptoDelete& delete_) final;

private:
    // Change me!
    std::unordered_map<Symbol, std::set<Order>> bids;
    std::unordered_map<Symbol, std::set<Order>> offers;
};

} // namespace wwhrt
