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

namespace i2p_ouiservice {

class Service;

class Channel {
public:
  using OnConnect = std::function<void(boost::system::error_code, Channel*)>;
  using OnReceive = std::function<void(boost::system::error_code, size_t)>;
  using OnWrite   = std::function<void(boost::system::error_code, size_t)>;

public:
    Channel(Service&);

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    Channel(Channel&&) = default;
    Channel& operator=(Channel&&) = default;

    boost::asio::io_service& get_io_service();

    void connect(std::string target_id, uint32_t connect_timeout, OnConnect connect_handler);

    void listen(int listen_port, OnConnect connect_handler, uint32_t connect_timeout = 0, std::string private_key_str = "");
    
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
    bool _server_mode = false;
    std::string localhost = "127.0.0.1";
    boost::asio::io_service& _ios;
    //boost::asio::steady_timer _status_timer;
    boost::asio::ip::tcp::socket socket_;
    
    std::unique_ptr<i2p::client::I2PService> i2p_oui_tunnel;
    
    OnConnect _connect_handler;

    void handle_tunnel_ready(const boost::system::error_code& err);
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
    socket_.async_write_some(bufs, h);
}

} // i2p_ouiservice namespace
