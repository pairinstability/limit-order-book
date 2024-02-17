Limit order book datastructures
===

- limit order book.
- basic client/server communication over UDP for adding orders to the order book using boost asio and serialization.
- LOB operations run in O(1) time.
- definitely suboptimal.


```cpp
#include <Book.hpp>

int main()
{
    OrderBook::Book orderBook;
    OrderBook::Order order1;

    order1.id_number = 1;
    order1.order_type = OrderBook::OrderType::BUY;
    order1.shares = 1;
    order1.limit = 10;

    orderBook.addOrder(order1);
    orderBook.cancelOrder(order1);

    return 0;
}
```

Design
---

`Book` class represents an order book for a single symbol. The book has a buy and sell side, which stores the buy and sell orders.

`Order` struct contains order information. `Limit` struct contains the limit price and a doubly-linked list (with std::list) of all orders at that limit price.

The buy and sell side datastructures are red-black binary search trees (with std::map) of `Limit` objects, of unique limit prices. The orders are stored according to their limit price within the `Limit` objects, in order to be executed.

As searching these trees is O(log n) with the size of the container, and searching a doubly-linked list is O(n) with std::find or std::find_if, two hashtables are used to store order information and limit information.

The order hashtable is keyed on order ID number and the values are iterators to Order objects in the doubly-linked list in the Limit objects. This search is O(1) and is used to speed up search for the doubly-linked list.

The limit hashtable is keyed on limit price and the values are iterators to Limit objects in the red-black binary search trees. This search is also O(1) and is used to speed up search for the trees.

Note that pointers are probably better than iterators, unless your own containers are used rather than std lib/third party since theres no guarantees with iterators. But keeping this simple.

The functions added are `addOrder`, `cancelOrder`, `executeOrder`, `getBestBid`, and `getBestOffer`.



```cpp
enum class OrderType {
    BUY,
    SELL,
    CANCEL
};

struct Order {
    // key
    size_t id_number;
    OrderType order_type;
    size_t shares;
    size_t limit;
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

struct Limit {
    size_t limit_price;
    size_t total_volume;

    std::list<Order> orders;
};

std::map<size_t, Limit> m_buy_tree;
std::map<size_t, Limit> m_sell_tree;

// Hashmap of orders keyed on order.id_number.
std::unordered_map<size_t, std::list<Order>::iterator> m_order_hashtable;

// Hashmap of limit rbtree iterators keyed on limit.limit_price.
// The value is a ptr to a Limit into whichever rb binary search tree.
std::unordered_map<size_t, std::map<size_t, Limit>::iterator> m_limit_hashtable;

std::list<Order>::iterator m_lowest_sell;
std::list<Order>::reverse_iterator m_highest_buy;
```









Building
---

Build examples and client/server with:

```sh
./scripts/build.sh
```

For running the client/server, run server in one terminal and client in another.

About
---

A limit order book contains list of buy limit orders and list of sell limit orders. a limit order is an order to buy a security at no more than a specific price, or to sell at no less than a specific price.


To do
---

- clean up.
- implement own datastructures instead of std::list, std::map, std::unordered_map. or just use abseil hashtable since std::unordered_map is not great.
- benchmark, add low latency to the LOB.
- clean up the client/server.
- matching engine.


Reference
---

- Ace the Trading Systems Developer Interview by Dennis Thompson.
- https://web.archive.org/web/20110219163448/http://howtohft.wordpress.com/2011/02/15/how-to-build-a-fast-limit-order-book/