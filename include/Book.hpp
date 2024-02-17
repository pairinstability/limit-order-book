#ifndef BOOK_H_
#define BOOK_H_

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <unordered_map>

namespace OrderBook {

typedef size_t OrderHashKey;
typedef size_t PriceType;

enum class OrderType {
    BUY,
    SELL,
    CANCEL
};

/// @brief a single order.
/// @details Serialized for transfer between client and server over UDP.
/// Dont need any symbol information since the data structures represent a single
/// order.
struct Order {
    // key
    OrderHashKey id_number;
    OrderType order_type;
    size_t shares;
    PriceType limit;
    size_t entry_time;
    size_t event_time;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& id_number;
        ar& order_type;
        ar& shares;
        ar& limit;
        ar& entry_time;
        ar& event_time;
    }
};

/// @brief the limit order book.
class Book {
private:
    // doubly-linked list of orders for each limit.
    typedef std::list<Order> OrderList;

    /// @brief a buy/sell limit value
    /// @details Contains a hashtable of orders at that given limit
    struct Limit {
        PriceType limit_price;
        size_t total_volume;

        // doubly-linked list of orders, as they must be in order to be executed later.
        OrderList orders;
    };

    typedef std::map<PriceType, Limit> BuySellSideTree;
    // iterators are not great for this because we're relying on std library implementations
    // and who knows what the iterators will do. ideally, we'd implement our own lists, maps, hashtables etc.
    typedef std::unordered_map<OrderHashKey, OrderList::iterator> OrderHashtable;
    typedef std::unordered_map<PriceType, BuySellSideTree::iterator> LimitHashtable;

    // Buy and sell side represented by two binary search trees of Limit objects.
    BuySellSideTree m_buy_tree;
    BuySellSideTree m_sell_tree;

    // Hashmap of orders keyed on order.id_number.
    OrderHashtable m_order_hashtable;

    // Hashmap of limit rbtree iterators keyed on limit.limit_price.
    // The value is a ptr to a Limit into whichever rb binary search tree.
    LimitHashtable m_limit_hashtable;

    // iterators to ensure that access in the orders list is O(1)
    OrderList::iterator m_lowest_sell;
    OrderList::reverse_iterator m_highest_buy;

    inline void updateBook()
    {
        // update lowest sell
        if (!m_sell_tree.empty()) {
            m_lowest_sell = m_sell_tree.begin()->second.orders.begin();
        }

        // update highest buy
        if (!m_buy_tree.empty()) {
            m_highest_buy = m_buy_tree.rbegin()->second.orders.rbegin();
        }
    };

public:
    Book()
    {
        m_buy_tree = {};
        m_sell_tree = {};
        m_order_hashtable = {};
        m_limit_hashtable = {};
    };

    ~Book() {};

    /// @brief Places an order at the end of a list of orders, to be executed at a particular limit price.
    /// @details Runs O(logn) for the first order at a new price, and O(1) at all other times.
    /// @param order The order.
    void addOrder(Order& order)
    {
        std::cout << "adding order" << std::endl;

        // We first search the m_limit_tre hashtable to see if the limit value exists, as
        // this data structure mirrors what is available in the buy/sell rb trees.
        // We can do this since hashmap lookup is O(1), whereas binary search tree
        // lookup is O(log n).
        //
        // If the limit exists in the hashtable, then we add the order to the existing
        // doubly-linked list of std::list<Order> in Limit.orders.
        //
        // If it isn't in the hashtable, then we need to add the new limit to both
        // the rb tree and the lookup hashtable.

        auto& limit_rbtree = (order.order_type == OrderType::BUY) ? m_buy_tree : m_sell_tree;

        // O(1)
        // find if the order.limit as a key exists in the hashmap of limits.
        auto limit_it = m_limit_hashtable.find(order.limit);

        // if limit exists, which is the more likely/common case
        if (limit_it != m_limit_hashtable.end()) {
            limit_it->second->second.total_volume += order.shares;

            // add the order, in O(1).
            limit_it->second->second.orders.push_back(order);
            // and add the iterator to the order hashtable so we can get fast access later.
            m_order_hashtable.insert({ order.id_number, std::prev(limit_it->second->second.orders.end()) });

        } else {
            // if the limit doesn't exist, then we need to add it to the rb-tree
            // and the hashmap.
            Limit new_limit;
            new_limit.limit_price = order.limit;
            new_limit.total_volume = order.shares;

            // add to rb-tree, in O(log n). This is costly but rare.
            // this transfers ownership of the Limit object into the rb-tree.
            auto limit_tree_it = limit_rbtree.emplace(order.limit, std::move(new_limit)).first;
            // add to the lookup hashtable, in O(1).
            m_limit_hashtable.insert({ order.limit, limit_tree_it });

            // then finally add the order to the Limit linked list, in O(1).
            limit_tree_it->second.orders.push_back(order);

            // and add the order to the order hashtable.
            // the value is the iterator in limit_rbtree for this new limit's orders to the last element.
            m_order_hashtable.insert({ order.id_number, std::prev(limit_tree_it->second.orders.end()) });
        }

        updateBook();
    };

    /// @brief Removes an order from the book.
    /// @details Runs O(1).
    /// @param order The order.
    void cancelOrder(Order& order)
    {
        std::cout << "cancelling order" << std::endl;
        // cancelling an order
        //
        // we do the same method as in addOrder for determining if the order exists in the order book,
        // except if the order doesn't exist then we just return (TODO). so this is near guaranteed O(1).
        //
        // once we find the order, we erase it.

        auto& limit_rbtree = (order.order_type == OrderType::BUY) ? m_buy_tree : m_sell_tree;
        auto limit_it = m_limit_hashtable.find(order.limit);

        // if the limit exists
        if (limit_it != m_limit_hashtable.end()) {
            // then we need to remove the order from the hashtable
            // and the rb tree.
            // if the order just doesnt exist, do nothing.

            auto order_hashtable_it = m_order_hashtable.find(order.id_number);
            (order_hashtable_it != m_order_hashtable.end()) ? (static_cast<void>(m_order_hashtable.erase(order_hashtable_it))) : void();

            updateBook();
        };
    }

    /// @brief Executes an order from the order book.
    /// @details Runs O(1).
    /// A buy limit order will be executed only at the limit price or a lower price.
    /// A sell limit order will be executed only at the limit price or a higher one.
    /// @param order The order.
    void executeOrder(Order& order) {
        // do something
    };

    /// @brief Returns the best bid.
    /// @details Runs O(1).
    /// @returns The best bid..
    size_t getBestBid(Order& order) const
    {
        return m_highest_buy->limit;
    };

    /// @brief Returns the best offer.
    /// @details Runs O(1).
    /// @returns The best offer.
    size_t getBestOffer(Order& order) const
    {
        return m_lowest_sell->limit;
    };
};
}

#endif // BOOK_H_