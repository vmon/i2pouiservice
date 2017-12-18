#pragma once

#include <memory>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>

#include "I2PTunnel.h"

namespace ouichannel {
namespace i2p_ouichannel {

class Service;

class Channel {
public:
  using OnConnect = std::function<void(boost::system::error_code)>;
  using OnReceive = std::function<void(boost::system::error_code, size_t)>;
  using OnWrite   = std::function<void(boost::system::error_code, size_t)>;

public:
    Channel(Service&);
    Channel(Service& service, boost::asio::ip::tcp::socket& server_socket);

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    Channel(Channel&&) = default;
    Channel& operator=(Channel&&) = default;

    boost::asio::io_service& get_io_service();

    void
    connect( std::string target_id
             , const std::string& shared_secret, OnConnect connect_handler);

    void listen(const std::string& shared_secret, int listen_port, OnConnect connect_handler, std::string private_key_str = "");
    
    template< class MutableBufferSequence
            , class ReadHandler>
    void async_read_some(const MutableBufferSequence&, ReadHandler&&);

    template< class ConstBufferSequence
            , class WriteHandler>
    void async_write_some(const ConstBufferSequence&, WriteHandler&&);

    ~Channel();

protected:
    friend class Service;
    int _tunnel_port;
    std::string localhost = "127.0.0.1";
    boost::asio::io_service& _ios;
    boost::asio::steady_timer _status_timer;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket client_socket_;
    boost::asio::ip::tcp::socket& socket_;
    
    boost::asio::streambuf request_;
    boost::asio::streambuf response_;

    std::unique_ptr<i2p::client::I2PService> i2p_oui_tunnel;
    
    OnConnect _connect_handler;

    void handle_tunnel_ready(const boost::system::error_code& err);
    void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void handle_connect(const boost::system::error_code& err);

    
};

template< class MutableBufferSequence
        , class ReadHandler>
void Channel::async_read_some( const MutableBufferSequence& bufs
                             , ReadHandler&& h)
{
    socket_.async_read_some(bufs, h);
}


template< class ConstBufferSequence
        , class WriteHandler>
void Channel::async_write_some( const ConstBufferSequence& bufs
                              , WriteHandler&& h)
{
    using namespace std;

    socket_.async_write_some(bufs, h);
    // get_io_service().post(
    //                       );
}

} // i2p_ouichannel
} // ouichannel
