#include <stdlib.h>
#include <inttypes.h>
#include <string>
#include <set>
#include <tuple>
#include <memory>
#include <sstream>

#include "Identity.h"
#include "Destination.h"
#include "Datagram.h"
#include "Streaming.h"
#include "I2PService.h"

#include "i2pouichannel.h"
#include "service.h"

using namespace std;
using namespace ouichannel::i2p_ouichannel;

Channel::Channel(Service& service)
  : _ios(service.get_io_service()),
    resolver_(_ios),
    socket_(_ios)
{
}

boost::asio::io_service& Channel::get_io_service()
{
    return _ios;
}

void Channel::connect( std::string target_id
                          , const std::string& shared_secret
                          , OnConnect connect_handler)
{
    _tunnel_port = rand() % 32768 + 32768;

    i2p_oui_tunnel = std::make_unique<i2p::client::I2PClientTunnel>("i2p_oui_client", target_id, localhost, _tunnel_port, nullptr);
    _connect_handler = connect_handler;
    

    //Wait till we find a route to the service and tunnel is ready then try to acutally connect and then call the handl
    i2p_oui_tunnel->AddReadyCallback(boost::bind(&Channel::handle_tunnel_ready, this, boost::asio::placeholders::error));

}


void Channel::handle_tunnel_ready(const boost::system::error_code& err)
{
    if (!err)
    {
      // The tunnel is ready
      // Start an asynchronous resolve to translate the server and service names
      // into a list of endpoints.
      boost::asio::ip::tcp::resolver::query query(localhost, std::to_string(_tunnel_port));
      resolver_.async_resolve(query,
                              boost::bind(&Channel::handle_resolve, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::iterator));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }

}

void Channel::handle_resolve(const boost::system::error_code& err,
                             boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
  if (!err)
    {
      // Attempt a connection to the first endpoint in the list. Each endpoint
      // will be tried until we successfully establish a connection.
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
                            boost::bind(&Channel::handle_connect, this,
                                        boost::asio::placeholders::error, ++endpoint_iterator));
    }
  else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
}

void Channel::handle_connect(const boost::system::error_code& err,
                    boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
  if (!err)
    {
      // The connection was successful. call the handler
      (_connect_handler)(err);
     
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // The connection failed. Try the next endpoint in the list.
      socket_.close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
                            boost::bind(&Channel::handle_connect, this,
                                        boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
      //connection failed call the handler with error
      std::cout << "Error: " << err.message() << "\n";
      (_connect_handler)(err);
      
    }
}

Channel::~Channel()
{
  //TODO
}
