#include "service.h"

//i2p stuff
#include "Log.h"
#include "api.h"

using namespace std;
using namespace i2p_ouiservice;

Service::Service(const string& datadir, boost::asio::io_service& ios)
  : _ios(ios)
{
  //here we are going to read the config file and
  //set options based on those values for now we just
  //set it up by some default values;

  //start i2p logger
  i2p::log::Logger().Start();

  LogPrint(eLogInfo, "Starting i2p tunnels");

  string datadir_arg = "--datadir=" + datadir;

  std::vector<const char*> argv({"i2pouiservice", datadir_arg.data()});

  i2p::api::InitI2P(argv.size(), (char**) argv.data(), argv[0]);
  i2p::api::StartI2P();
}

boost::asio::io_service& Service::get_io_service()
{
  return _ios;
}

/**
   chooses a port and accept on it
*/
void Service::accept(std::string private_key_str, Channel& channel, OnConnect connect_handler) {
  using tcp = boost::asio::ip::tcp;

  //we have to accept to this port so the i2pservertunnel can forward us the connection
  acceptor_ = std::make_unique<tcp::acceptor>(_ios, tcp::endpoint(tcp::v4(), 0));

  uint16_t port = acceptor_->local_endpoint().port();

  acceptor_->async_accept(channel.socket_,
                          [this, ch = &channel](boost::system::error_code ec) {
                              if (ec) {
                                  std::cout << "Error: " << ec.message() << "\n";
                              }
                              ch->handle_connect(ec);
                          });

  channel.accept(port, connect_handler, get_i2p_tunnel_ready_timeout(),  private_key_str);
}
