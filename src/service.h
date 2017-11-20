#pragma once

#include <boost/asio/io_service.hpp>
namespace ouichannel {
namespace i2p_ouichannel {


class Service {

public:
  Service(std::string config_path, boost::asio::io_service&);

  Service(const Service&) = delete;
  Service& operator=(const Service&) = delete;

  void async_setup();

  boost::asio::io_service& get_io_service();

  std::string identity() const;

  ~Service();

 private:
  boost::asio::io_service& _ios;
    
};

}
}
