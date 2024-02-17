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