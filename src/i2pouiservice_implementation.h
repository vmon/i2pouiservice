#ifndef I2P_OUISERVICE_IMPLEMENTATION_H
#define I2P_OUISERVICE_IMPLEMENTATION_H

#include "implementation.h"

namespace ouinet {

  namespace ouiservice {

    class I2POuiConnectionImplemetation : public ConnectionImplementation {
    public:
      virtual ~I2POuiConnectionImplementation();
      virtual asio::io_service& get_io_service();
      virtual size_t async_read_some(asio::mutable_buffers_1 buffer, asio::yield_context yield);
      virtual size_t async_write_some(asio::const_buffers_1 buffer, asio::yield_context yield);
      virtual void close(asio::yield_context yield) = 0;
    }
  }

}
#endif 
