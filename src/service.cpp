#include "service.h"
#include "i2pouichannel.h"

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
