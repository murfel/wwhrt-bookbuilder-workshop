#pragma once

#include "Allocator.h"
#include "Events.h"
#include "Reader.h"

#include <list>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace wwhrt {

#define ORDERS_LIST
#define ASKBIDSPLIT
#define SORTED

//#define ORDERS_VECTOR
//#define ASKBIDSPLIT
//#define SWAPREMOVE  // conflicts with SORTED
//#define SORTED

//#define ORDERS_SET  // must go with ASKBIDSPLIT
//#define ASKBIDSPLIT
//#define ATTEMPT_ERASE

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
#ifndef ASKBIDSPLIT
        Side side;
#endif

        bool operator<(const Order& ord) const {
            if (price != ord.price) {
                return price < ord.price;
            }
            if (size != ord.size) {
                return size < ord.size;
            }
            return id < ord.id;
        }

        struct OrderHasher {
            size_t operator()(const Order &ord) const {
                return std::hash<uint64_t>()(ord.id);
            }
        };
    };
    BookBuilder() = default;
    std::vector<Order> getBestBids(Symbol symbol);
    std::vector<Order> getBestOffers(Symbol symbol);

    void onAdd(const CryptoAdd& add) final;
    void onUpdate(const CryptoUpdate& update) final;
    void onDelete(const CryptoDelete& delete_) final;

  private:
    // Change me!
#ifdef ORDERS_LIST
    using OrderContainer = std::list<Order>;
#elif defined(ORDERS_VECTOR)
    using OrderContainer = std::vector<Order>;
#elif defined(ORDERS_SET)
    using OrderContainer = std::set<Order>;
#endif

static void insert(OrderContainer & symbolOrders, double price, double size, uint64_t orderId, Side side);

#ifndef ASKBIDSPLIT
    std::unordered_map<Symbol, OrderContainer> orders;
#else
    std::unordered_map<Symbol, OrderContainer> bids;
    std::unordered_map<Symbol, OrderContainer> offers;
#endif
    };

} // namespace wwhrt
