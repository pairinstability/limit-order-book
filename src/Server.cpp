#include <Book.hpp>
#include <boost/archive/text_iarchive.hpp>
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

std::pair<Command, OrderBook::Order> parse_msg(const std::string& message)
{
    std::pair<Command, OrderBook::Order> result;

    size_t pos = message.find('|');
    if (pos == std::string::npos)
        throw std::invalid_argument("Invalid message format");

    std::string command = message.substr(0, pos);
    if (command == "ADD") {
        result.first = Command::ADD;
    } else if (command == "CANCEL") {
        result.first = Command::CANCEL;
    } else if (command == "EXECUTE") {
        result.first = Command::EXECUTE;
    } else if (command == "GET_BEST_BID") {
        result.first = Command::GET_BEST_BID;
    } else if (command == "GET_BEST_OFFER") {
        result.first = Command::GET_BEST_OFFER;
    } else
        throw std::invalid_argument("Unknown command");

    // deserialization
    std::string order_str = message.substr(pos + 1);
    std::istringstream iss(order_str);
    boost::archive::text_iarchive archive(iss);
    archive >> result.second;

    return result;
}

int main()
{
    OrderBook::Book OrderBook;

    try {
        boost::asio::io_context io_ctx;

        udp::socket socket(io_ctx, udp::endpoint(udp::v4(), 8080));

        while (true) {
            std::array<char, 128> recv_buf;
            udp::endpoint remote_endpoint;
            boost::system::error_code error;

            size_t len = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

            if (error && error != boost::asio::error::message_size)
                throw boost::system::system_error(error);

            std::string message(recv_buf.data(), len);

            auto [command, order] = parse_msg(message);

            switch (command) {
            case Command::ADD:
                std::cout << "Received ADD command with order: " << order.id_number << std::endl;
                OrderBook.addOrder(order);
                break;
            case Command::CANCEL:
                std::cout << "Received CANCEL command with order: " << order.id_number << std::endl;
                OrderBook.cancelOrder(order);
                break;
            case Command::EXECUTE:
                std::cout << "Received EXECUTE command with order: " << order.id_number << std::endl;
                OrderBook.executeOrder(order);
                break;
            case Command::GET_BEST_BID:
                std::cout << "Recieved GET BEST BID command, with best bid: " << OrderBook.getBestBid(order) << std::endl;
                break;
            case Command::GET_BEST_OFFER:
                std::cout << "Recieved GET BEST OFFER command, with best offer: " << OrderBook.getBestOffer(order) << std::endl;
                break;
            default:
                std::cerr << "Unknown command" << std::endl;
                break;
            }

            // send something back just to confirm
            socket.send_to(boost::asio::buffer("Message received"), remote_endpoint);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
