#include "service.h"

//i2p stuff
#include "Log.h"
#include "api.h"

using namespace std;
using namespace ouichannel;
using namespace i2p_ouichannel;

Service::Service(string config_path, boost::asio::io_service& ios)
  : _ios(ios)
{
  //start i2p logger
  i2p::log::Logger().Start();

  LogPrint(eLogInfo, "Starting i2p tunnels");
    
  i2p::api::InitI2P(0, nullptr, "i2pouiservice");
  i2p::api::StartI2P();
    
}

boost::asio::io_service& Service::get_io_service()
{
  return _ios;
}

Service::~Service()
{
  //TODO?
}

void Service::async_setup()
{
  return;
}

/**
   chooses a port and listen on it 
*/
void Service::listen(OnConnect connect_handler) {
  _listen2i2p_port = rand() % 32768 + 32768;
  _connect_handler = connect_handler;

  //we have to listen to this prot so the i2pservertunnel can forward us the connection
  acceptor_ = std::make_unique<boost::asio::ip::tcp::acceptor>(_ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _listen2i2p_port));
  Channel* new_connection = new Channel(*this);
  acceptor_->async_accept(new_connection->socket_,
                          boost::bind(&Service::handle_accept, this, new_connection,
                                      boost::asio::placeholders::error));
  new_connection->listen("", _listen2i2p_port, connect_handler);
  
}

void Service::handle_accept(Channel* new_connection,
         const boost::system::error_code& error)
{
    if (!error)
    {
      new_connection->handle_connect(error);
    }
    else
    {
      delete new_connection;
    }

    Channel* re_connect = new Channel(*this);
    acceptor_->async_accept(re_connect->socket_,
          boost::bind(&Service::handle_accept, this, re_connect,
          boost::asio::placeholders::error));

}


