#ifndef OUINET_OUISERVICE_IMPLEMENTATION_H
#define OUINET_OUISERVICE_IMPLEMENTATION_H

#include <boost/asio/buffer.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/system/error_code.hpp>

#include "../../namespaces.h"

#include <memory>
#include <string>
#include <vector>

/*
 * The abstract interface that OuiServices need to implement.
 */

namespace ouinet {



/*
 * HACK HACK HACK
 * This is clearly horribly wrong, but I see no better way in asio to do this. Any ideas?
 */
inline void set_error(asio::yield_context context, sys::error_code ec)
{
	*(context.ec_) = ec;
}



namespace ouiservice {

class ConnectionImplementation
{
	public:
	virtual ~ConnectionImplementation() {}
	virtual asio::io_service& get_io_service() = 0;
	virtual size_t async_read_some(asio::mutable_buffers_1 buffer, asio::yield_context yield) = 0;
	virtual size_t async_write_some(asio::const_buffers_1 buffer, asio::yield_context yield) = 0;
	virtual void close(asio::yield_context yield) = 0;
};

class ServiceImplementation
{
	public:
	virtual ~ServiceImplementation() {}
	virtual void start(asio::yield_context yield) = 0;
	virtual void stop(asio::yield_context yield) = 0;
	virtual void start_listen(asio::yield_context yield) = 0;
	virtual void stop_listen(asio::yield_context yield) = 0;
	virtual std::shared_ptr<ConnectionImplementation> connect(asio::yield_context yield) = 0;
	virtual std::shared_ptr<ConnectionImplementation> accept(asio::yield_context yield) = 0;
	virtual void cancel_accept() = 0;
};

class Implementation
{
	public:
	virtual ~Implementation() {}
	virtual std::shared_ptr<ServiceImplementation> join_service(asio::io_service& ios, std::string name) = 0;
};

} // namespace ouiservice
} // namespace ouinet

#endif
