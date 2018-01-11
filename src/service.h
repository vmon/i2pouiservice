#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

#include "i2pouichannel.h"

namespace i2p_ouiservice {

class Service {

public:
  using OnConnect = Channel::OnConnect;

  Service(const std::string& datadir, boost::asio::io_service&);

  Service(const Service&) = delete;
  Service& operator=(const Service&) = delete;

  boost::asio::io_service& get_io_service();

  std::string identity() const;

  /**
     chooses a port and accept on it
   */
  void accept(std::string private_key_str, Channel&, OnConnect connect_handler);

  ~Service();

  //access functions
  uint32_t  get_i2p_tunnel_ready_timeout() { return _i2p_tunnel_ready_timeout;};

protected:
  uint32_t _i2p_tunnel_ready_timeout;
  boost::asio::io_service& _ios;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
  int _listen2i2p_port;

  OnConnect _connect_handler;
  std::string _private_key_str;
};

} // i2p_ouiservice namespace
