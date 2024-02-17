limit order book datastructures
===

single threaded limit order book with client/server communication over UDP for adding orders to the order book.

a limit order book contains list of buy limit orders and list of sell limit orders. a limit order is an order to buy a security at no more than a specific price, or to sell at no less than a specific price.

LOB operations run in O(1) time.