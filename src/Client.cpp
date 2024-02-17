#include <Book.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/serialization.hpp>
#include <iostream>

using boost::asio::ip::udp;

enum class Command {
    ADD,
    CANCEL,
    EXECUTE,
    GET_BEST_BID,
    GET_BEST_OFFER
};

std::string send_msg(udp::socket& socket, const udp::endpoint& endpoint, Command command, const OrderBook::Order& order)
{
    std::string serialized_command;
    if (command == Command::ADD) {
        serialized_command = "ADD";
    } else if (command == Command::CANCEL) {
        serialized_command = "CANCEL";
    } else if (command == Command::EXECUTE) {
        serialized_command = "EXECUTE";
    } else if (command == Command::GET_BEST_BID) {
        serialized_command = "GET_BEST_BID";
    } else if (command == Command::GET_BEST_OFFER) {
        serialized_command = "GET_BEST_OFFER";
    }

    std::ostringstream oss;
    boost::archive::text_oarchive archive(oss);
    archive << order;
    std::string serialized_order = oss.str();

    std::string message = serialized_command + "|" + serialized_order;
    socket.send_to(boost::asio::buffer(message), endpoint);

    std::array<char, 128> recv_buf;
    udp::endpoint sender_endpoint;
    size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

    return std::string(recv_buf.data(), len);
}

int main()
{
    try {
        boost::asio::io_context io_ctx;
        udp::socket socket(io_ctx, udp::endpoint(udp::v4(), 0));
        udp::resolver resolver(io_ctx);
        udp::resolver::results_type endpoints = resolver.resolve(udp::v4(), "localhost", "8080");

        OrderBook::Order order;
        order.id_number = 1;
        order.order_type = OrderBook::OrderType::BUY;
        order.shares = 1;
        order.limit = 10;

        std::cout << send_msg(socket, *endpoints.begin(), Command::ADD, order) << std::endl;

        OrderBook::Order order2;
        order2.id_number = 2;
        order2.order_type = OrderBook::OrderType::SELL;
        order2.shares = 1;
        order2.limit = 10;

        std::cout << send_msg(socket, *endpoints.begin(), Command::ADD, order2) << std::endl;
        std::cout << send_msg(socket, *endpoints.begin(), Command::CANCEL, order) << std::endl;

        std::cout << send_msg(socket, *endpoints.begin(), Command::GET_BEST_BID, order2) << std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
