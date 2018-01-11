#include <fstream>
#include <streambuf>
#include "service.h"

//i2p stuff
#include "Log.h"
#include "api.h"

using namespace std;
using namespace i2p_ouiservice;

static string load_private_key()
{
    string datadir = i2p::fs::GetDataDir();
    string key_file_name = datadir + "/private_key";

    ifstream in_file(key_file_name);

    if (in_file.is_open()) {
        return string( istreambuf_iterator<char>(in_file)
                     , istreambuf_iterator<char>());
    }

    // File doesn't exist
    std::shared_ptr<i2p::client::ClientDestination> local_dst;
    local_dst = i2p::api::CreateLocalDestination(true);
    string priv_key = local_dst->GetPrivateKeys().ToBase64();

    ofstream out_file(key_file_name);
    out_file << priv_key;

    return priv_key;
}

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

  _private_keys.FromBase64(load_private_key());
}

boost::asio::io_service& Service::get_io_service()
{
  return _ios;
}

std::string Service::public_identity() const
{
  return _private_keys.GetPublic()->ToBase64();
}

/**
   chooses a port and accept on it
*/
void Service::accept(Channel& channel, OnConnect connect_handler) {
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

  channel.accept(port, connect_handler, get_i2p_tunnel_ready_timeout(), _private_keys);
}
