#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

#include "i2pouichannel.h"

namespace ouichannel {
namespace i2p_ouichannel {


class Service {

public:
  using OnConnect = std::function<void(boost::system::error_code)>;

  Service(std::string config_path, boost::asio::io_service&);

  Service(const Service&) = delete;
  Service& operator=(const Service&) = delete;

  void async_setup();

  boost::asio::io_service& get_io_service();

  std::string identity() const;

  /**
     chooses a port and listen on it 
   */
  void listen(OnConnect connect_handler);

  ~Service();

protected:
  void handle_accept(Channel* new_connection,
                     const boost::system::error_code& error);

  boost::asio::io_service& _ios;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
  int _listen2i2p_port;

  OnConnect _connect_handler;

    
};

}
}
