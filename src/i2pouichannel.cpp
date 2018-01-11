#include <boost/bind.hpp>

#include "I2PService.h"
#include "api.h"

#include "i2pouichannel.h"
#include "service.h"

using namespace std;
using namespace i2p_ouiservice;

Channel::Channel(Service& service)
  : _ios(service.get_io_service()),
    socket_(_ios)
{
}

boost::asio::io_service& Channel::get_io_service()
{
    return _ios;
}

void Channel::connect( std::string target_id
                     , uint32_t connect_timeout
                     , OnConnect connect_handler)
{
    _tunnel_port = rand() % 32768 + 32768;

    cout << "tunnel port: " << _tunnel_port << endl;

    i2p_oui_tunnel = std::make_unique<i2p::client::I2PClientTunnel>("i2p_oui_client", target_id, localhost, _tunnel_port, nullptr);
    _connect_handler = connect_handler;

    i2p_oui_tunnel->Start();

    //Wait till we find a route to the service and tunnel is ready then try to acutally connect and then call the handle
    i2p_oui_tunnel->AddReadyCallback(boost::bind(&Channel::handle_tunnel_ready, this, boost::asio::placeholders::error));
    //we need to set a timeout in order to trigger the timer for checking the tunnel readyness
    i2p_oui_tunnel->SetConnectTimeout(connect_timeout);
}

void Channel::accept(int listen_port, uint32_t connect_timeout, i2p::data::PrivateKeys private_keys)
{
  //we need to make a local destination first.
  std::shared_ptr<i2p::client::ClientDestination> local_destination;

  local_destination = i2p::api::CreateLocalDestination(private_keys, true);
    
  i2p_oui_tunnel = std::make_unique<i2p::client::I2PServerTunnel>("i2p_oui_server", localhost, listen_port, local_destination);

  i2p_oui_tunnel->Start();

  //Wait till we find a route to the service and tunnel is ready then try to acutally connect and then call the handl
  i2p_oui_tunnel->AddReadyCallback([](const boost::system::error_code& ec) {
          if (ec) {
            std::cout << "Error: " << ec.message() << "\n";
          }
      });

  //we need to set a timeout in order to trigger the timer for checking the tunnel readyness
  i2p_oui_tunnel->SetConnectTimeout(connect_timeout);
}


void Channel::handle_tunnel_ready(const boost::system::error_code& ec)
{
    if (ec) {
        std::cout << "Error: " << ec.message() << "\n";
        return;
    }

    bool is_ready = i2p_oui_tunnel->GetLocalDestination()->IsReady();
    assert(is_ready && "TODO: Can it not be ready given (!ec)?");

    namespace ip = boost::asio::ip;
    socket_.async_connect(ip::tcp::endpoint(ip::address_v4::loopback(), _tunnel_port),
                          [this] (const boost::system::error_code& ec) {
                              if (ec) {
                                  std::cout << "Error: " << ec.message() << "\n";
                              }

                              _connect_handler(ec);
                          });
}
