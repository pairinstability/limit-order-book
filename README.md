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