#include <iostream>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>

#include "i2pouichannel.h"
#include "service.h"

using namespace std;
using namespace boost;

using namespace ouichannel;
using namespace i2p_ouichannel;

unique_ptr<Channel> channel;

static string remove_new_line(string s)
{
    while (!s.empty() && *(--s.end()) == '\n') {
        s.resize(s.size() - 1);
    }
    return s;
}

static string consume(asio::streambuf& buf, size_t n)
{
    string out(n, '\0');
    buf.sgetn((char*) out.c_str(), n);
    return out;
}


void handle_read_echo(const boost::system::error_code& ec, asio::streambuf& buffer)
{
                if (ec || !channel) return;

                cout << "Received: "
                     << remove_new_line(consume(buffer, buffer.size()))
                     << endl;
}

static void wait_for_the_echo(const boost::system::error_code& ec, asio::streambuf& buffer)
{
  if (ec || !channel)
    return;

  asio::async_read(*channel, buffer,
                   [&buffer](const system::error_code& ec, std::size_t size) 
                   {
                      handle_read_echo(ec, buffer);
                   }
               );
}

static void handle_user_input(const boost::system::error_code& ec, asio::streambuf& buffer)
{
  if (ec || !channel) return;
  asio::async_write(*channel, asio::buffer(consume(buffer, buffer.size())), [&buffer](const boost::system::error_code& ec, size_t size) {wait_for_the_echo(ec, buffer);});
}

static void run_chat(const boost::system::error_code& err) {
    auto& ios = channel->get_io_service();

    // Start printing received messages
    asio::spawn(ios, [] (asio::yield_context yield) {
            system::error_code ec;
            asio::streambuf buffer(512);

            while (true) {
                size_t n = asio::async_read_until(*channel, buffer, '\n', yield[ec]);

                if (ec || !channel) return;

                cout << "Received: "
                     << remove_new_line(consume(buffer, n))
                     << endl;
            }
        });

    // Read from input and send it to peer
    asio::posix::stream_descriptor input(ios, ::dup(STDIN_FILENO));

    asio::streambuf buffer(512);

    asio::spawn(ios, [&] (auto yield) {
            system::error_code ec;

            //service.async_setup(yield[ec]);
            if (ec) {
                cerr << "Failed to set up gnunet service: " << ec.message() << endl;
                return;
            }

            while (true) {
              system::error_code ec;
              asio::async_read_until(input, buffer, '\n', [&buffer] (const boost::system::error_code& ec, size_t size) mutable {
                  handle_user_input(ec, buffer);
                });

              if (ec || !channel)
                break;
            }
      }
      );
}


static void connect_and_run_chat( unique_ptr<Channel>& channel
                                , Service& service
                                , string target_id
                                , string port
                                , asio::yield_context yield)
{
    system::error_code ec;

    cout << "Connecting to " << target_id << endl;
    channel->connect(target_id, port, run_chat);
    
}

static void accept_and_run_chat( unique_ptr<Channel>& channel
                               , Service& service
                               , string port
                               , asio::yield_context yield)
{
    // system::error_code ec;

    // cout << "Accepting on port \"" << port << "\"" << endl;

    // {
    //     CadetPort p(service);
    //     p.open(*channel, port, yield[ec]);

    //     if (ec) {
    //         cerr << "Failed to accept: " << ec.message() << endl;
    //         return;
    //     }
    // }

    // cout << "Accepted" << endl;

    // run_chat(channel, yield);
}

static void print_usage(const char* app_name)
{
    cerr << "Usage:\n";
    cerr << "    " << app_name << " <config-file> <secret-phrase> [peer-id]\n";
    cerr << "If [peer-id] is used the app acts as a client, "
            "otherwise it acts as a server\n";
}


int main(int argc, char* const* argv)
{
    if (argc != 3 && argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    asio::io_service ios;

    Service service(argv[1], ios);

    string target_id;
    string port = argv[2];

    if (argc >= 4) {
        target_id = argv[3];
    }

    // Capture these signals so that we can disconnect gracefully.
    asio::signal_set signals(ios, SIGINT, SIGTERM);


    //signals.async_wait([&](system::error_code, int /* signal_number */) {
    //        channel.reset();
    //    });

    asio::spawn(ios, [&] (auto yield) {
            system::error_code ec;

            //service.async_setup(yield[ec]);

            if (ec) {
                cerr << "Failed to set up gnunet service: " << ec.message() << endl;
                return;
            }

            channel = make_unique<Channel>(service);

            if (!target_id.empty()) {
                connect_and_run_chat(channel, service, target_id, port, yield);
            }
            else {
                accept_and_run_chat(channel, service, port, yield);
            }
        });

    ios.run();
}
