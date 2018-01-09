#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

#include "i2pouichannel.h"

namespace ouichannel {
namespace i2p_ouichannel {

class Service {

public:
  using OnConnect = std::function<void(boost::system::error_code, Channel*)>;

  Service(boost::asio::io_service&);

  Service(const Service&) = delete;
  Service& operator=(const Service&) = delete;

  void async_setup();

  boost::asio::io_service& get_io_service();

  std::string identity() const;

  /**
     chooses a port and listen on it 
   */
  void listen(OnConnect connect_handler, std::string private_key_str = "");

  ~Service();

  //access functions
  uint32_t  get_i2p_tunnel_ready_timeout() { return _i2p_tunnel_ready_timeout;};
protected:
  void handle_accept(Channel* new_connection,
                     const boost::system::error_code& error);

  uint32_t _i2p_tunnel_ready_timeout;
  boost::asio::io_service& _ios;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
  int _listen2i2p_port;

  OnConnect _connect_handler;
  std::string _private_key_str;
    
};

}
}
