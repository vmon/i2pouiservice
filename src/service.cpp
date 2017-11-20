#include <gnunet_channels/service.h>
#include "scheduler.h"
#include "cadet_connect.h"
#include "hello_get.h"

#include "api.h"

using namespace std;
using namespace gnunet_channels;


Service::Service(string config_path, asio::io_service& ios)
  : _ios(ios)
{
  //start i2p logger
  i2p::log::Logger().Start();

  LogPrint(eLogInfo, "Starting i2p tunnels");
    
  i2p::api::InitI2P(0, nullptr, "i2pouiservice");
  i2p::api::StartI2P();
    
}

asio::io_service& Service::get_io_service()
{
  return _ios;
}

Service::~Service()
{
    _impl->was_destroyed = true;
}
