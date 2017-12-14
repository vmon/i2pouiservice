//
// async_client.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>

#include "Config.h"
#include "Log.h"
#include "FS.h"
#include "Tunnel.h"
#include "I2PTunnel.h"
#include "HTTPProxy.h"
#include "ClientContext.h"
#include "AddressBook.h"
#include "api.h"

using boost::asio::ip::tcp;

class client
{
public:
  client(boost::asio::io_service& io_service,
      const std::string& server, const std::string& path)
    : server_(server),
      io_service_(io_service),
      status_timer{io_service_, std::chrono::seconds{3}},
      resolver_(io_service),
      socket_(io_service),
      i2p_oui_tunnel("i2p_oui_client", "xGNxi33ttUqf2oM8KopC5i~jE4feDcWZJ~dG2yczuSq~ErH1VqGnL6T3W3zjwe5K59-fjeaVJ1oniccanNk3UqHDVut4sFKwpNISFO9RUnUbKGKbj0C1az7ra6Rf3v5vJb2ByvLS7IsPPMffI6CvxysXNvuYe7MqzK1QLaXQ9eDifL8~kHpbr-WT2ewV-F9h5dnzc0AGcgsiw~TCDZW14vLtU5ysQhaaxnfld6tsYW4u8ejSKwdGPnVkN-fcw08riisQ9Z2ETnmSvkCHu51lG8h1jahbkt4PjpZaANaHhdgJW3Gq770nsHTHi0X3huboNKDL8opegrZkFJxcxuSf3V1MyaH8~qr2xFJ7tJBEvwSwN10cIYN2gWx3GXfeqKfo6bRdNv8Pl7bvM8~knoZw6UA5wLe5tnbeObE11oCoczD1DHidRSc0O1T93-vGlXGKd7~ncy1I-7P7WJ8MIbkXaRoLWUmMw~VJntlyb1bRKrAocuTwm6PnlU8CmWLeLcSkAAAA", "127.0.0.1", 8123, nullptr),
      m_address_book(i2p::client::context.GetAddressBook ()),
      m_HttpProxy("myhttpproxy","127.0.0.1", 8124, nullptr)

  {
    // i2p::config::Init();
    // char* fake_argv[] = {"get_slash"};
    // i2p::config::ParseCmdline(0, fake_argv);

    // i2p::config::ParseConfig("");
    // i2p::config::Finalize();
    // i2p::config::IsDefault("addressbook.subscriptions");
    // //Preparing i2p stuff
    i2p::log::Logger().Start();

    // //m_address_book.Init();
    // i2p::fs::DetectDataDir("/home/klaymen/.i2pd", false);
    // i2p::fs::Init();

    LogPrint(eLogInfo, "Daemon: starting Tunnels");
    // i2p::tunnel::tunnels.Start();

    // LogPrint(eLogInfo, "Daemon: starting Client");
    // i2p::client::context.Start ();

    i2p::api::InitI2P(0, nullptr, "i2pouiservice");
    i2p::api::StartI2P();
    
    //m_address_book.Start();
    //m_HttpProxy.Start();
    i2p_oui_tunnel.Start();
    //    std::cout << i2p_oui_tunnel.is_dest_ready() << std::endl;

    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    std::ostream request_stream(&request_);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << "i2p-projekt.i2p" << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    status_timer.async_wait(boost::bind(&client::handle_check_tunnel_status, this, boost::asio::placeholders::error));

    tcp::resolver::query query(server_, "8123");
    /*resolver_.async_resolve(query,
                            boost::bind(&client::handle_resolve, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::iterator));*/


  }

private:
  void handle_resolve(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (!err)
    {
      // Attempt a connection to the first endpoint in the list. Each endpoint
      // will be tried until we successfully establish a connection.
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
          boost::bind(&client::handle_connect, this,
            boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_connect(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (!err)
    {
      // The connection was successful. 
      //Send the request.
      boost::asio::async_write(socket_, request_,
          boost::bind(&client::handle_write_request, this,
            boost::asio::placeholders::error));
    }
    else if (endpoint_iterator != tcp::resolver::iterator())
    {
      // The connection failed. Try the next endpoint in the list.
      socket_.close();
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
          boost::bind(&client::handle_connect, this,
            boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_check_tunnel_status(const boost::system::error_code& err)
  {
    if (!err)
    {
      std::cout << "Tunnel Ready?" << i2p_oui_tunnel.GetLocalDestination()->IsReady() << std::endl;
      std::cout.flush();

      if (i2p_oui_tunnel.GetLocalDestination()->IsReady()) {
      // The tunnel is ready
      // Start an asynchronous resolve to translate the server and service names
      // into a list of endpoints.
        tcp::resolver::query query(server_, "8123");
        resolver_.async_resolve(query,
                              boost::bind(&client::handle_resolve, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::iterator));
      } else {
        //wait again
        status_timer.expires_at(status_timer.expires_at() + std::chrono::seconds{3});
        status_timer.async_wait(boost::bind(&client::handle_check_tunnel_status, this, boost::asio::placeholders::error));

        }
     
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
          
  }
  void handle_tunnel_ready(const boost::system::error_code& err)
  {
    if (!err)
    {
      // The tunnel is ready
      // Start an asynchronous resolve to translate the server and service names
      // into a list of endpoints.
      tcp::resolver::query query(server_, "8123");
      resolver_.async_resolve(query,
                              boost::bind(&client::handle_resolve, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::iterator));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }

  }

  void handle_write_request(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Read the response status line.
      boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
                              boost::bind(&client::handle_read_status_line/*handle_read_content*/, this,
            boost::asio::placeholders::error));
      // boost::asio::async_read_until(socket_, response_, "\r\n",
      //     boost::bind(&client::handle_read_status_line, this,
      //       boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        std::cout << "Invalid response\n";
        return;
      }
      if (status_code != 200)
      {
        std::cout << "Response returned with status code ";
        std::cout << status_code << "\n";
        return;
      }

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
          boost::bind(&client::handle_read_headers, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << ": " << err.message() << ": " << err.message() << "\n";
    }
  }

  void handle_read_headers(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r")
        std::cout << header << "\n";
      std::cout << "\n";

      // Write whatever content we already have to output.
      if (response_.size() > 0)
        std::cout << &response_;

      // Start reading remaining data until EOF.
      boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
          boost::bind(&client::handle_read_content, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << ": " << err.message() << "\n";
    }
  }

  void handle_read_content(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Write all of the data that has been read so far.
      std::cout << &response_;

      // Continue reading remaining data until EOF.
      boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
          boost::bind(&client::handle_read_content, this,
            boost::asio::placeholders::error));
    }
    else if (err != boost::asio::error::eof)
    {
      std::cout << "Error: " << err << ": " << err.message() << "\n";
    }
  }

  std::string server_;
  boost::asio::io_service& io_service_;
  boost::asio::steady_timer status_timer;
  tcp::resolver resolver_;
  tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;

  i2p::client::I2PClientTunnel i2p_oui_tunnel;
  i2p::client::AddressBook& m_address_book;
  i2p::proxy::HTTPProxy m_HttpProxy;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cout << "Usage: async_client <server> <path>\n";
      std::cout << "Example:\n";
      std::cout << "  async_client www.boost.org /LICENSE_1_0.txt\n";
      return 1;
    }

    boost::asio::io_service io_service;
    //boost::asio::io_service::work work(io_service);
    client c(io_service, argv[1], argv[2]);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}

