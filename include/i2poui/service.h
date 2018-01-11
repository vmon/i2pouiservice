#pragma once

#include <boost/asio.hpp>

#include <i2poui/channel.h>

namespace i2poui {

class Service {

public:
  using OnConnect = Channel::OnConnect;

  Service(const std::string& datadir, boost::asio::io_service&);

  Service(const Service&) = delete;
  Service& operator=(const Service&) = delete;

  boost::asio::io_service& get_io_service();

  std::string public_identity() const;

  /**
     chooses a port and accept on it
   */
  void accept(Channel&, OnConnect connect_handler);

  //access functions
  uint32_t  get_i2p_tunnel_ready_timeout() { return 5*60; /* 5 minutes */ };

protected:
  boost::asio::io_service& _ios;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
  i2p::data::PrivateKeys _private_keys;
};

} // i2poui namespace
