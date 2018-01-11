#include <iostream>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

#include "i2poui.h"

using namespace std;
using namespace boost;

using namespace i2p_ouiservice;

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

static void run_chat(const boost::system::error_code& ec) {
    auto& ios = channel->get_io_service();

    if (ec) {
      cerr << "Failed connect/accept: " << ec.message() << endl;
      return;
    }

    // This co-routine reads always from the socket and write it to std out.
    asio::spawn(ios, [] (asio::yield_context yield) {
            system::error_code ec;
            asio::streambuf buffer(512);

            while (true) {
                size_t n = asio::async_read_until(*channel, buffer, '\n', yield[ec]);

                if (ec) return;

                cout << "Received: "
                     << remove_new_line(consume(buffer, n))
                     << endl;
            }
        });

    // This co-routine reads from std input and send it to peer
    asio::spawn(ios, [&ios] (auto yield) {
            system::error_code ec;
            asio::posix::stream_descriptor input(ios, ::dup(STDIN_FILENO)); 

            asio::streambuf buffer(512);

            cout << "Enter a message to the peer" << endl;
            while (true) {
              system::error_code ec;
              size_t size = asio::async_read_until(input, buffer, '\n',
                                                   yield[ec]);
              if (ec) break;

              cout << "sending your message..." << endl;
              if (size > 0) 
                asio::async_write(*channel, asio::buffer(consume(buffer, buffer.size())), yield[ec]);
            }
      });
}

static void connect_and_run_chat( Service& service
                                , string target_id
                                , asio::yield_context yield)
{
    cout << "Connecting to " << target_id << endl;
    channel->connect(target_id, service.get_i2p_tunnel_ready_timeout(), run_chat);
}

static void accept_and_run_chat( Service& service
                               , asio::yield_context yield)
{
    cout << "Accepting on" << endl;
    cout << service.public_identity() << endl;
    service.accept(*channel, run_chat);
}

static void print_usage(const char* app_name)
{
    cerr << "Usage:\n";
    cerr << "    " << app_name << " [peer-id]\n";
    cerr << "If [peer-id] is used the app acts as a client, "
            "otherwise it acts as a server\n";
}

int main(int argc, char* const* argv)
{
    if (argc != 1 && argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    asio::io_service ios;

    bool is_client = argc >= 2;

    Service service(is_client ? "chat_client" : "chat_server", ios);

    asio::spawn(ios, [&] (auto yield) {
            channel = make_unique<Channel>(service);

            if (is_client) {
              connect_and_run_chat(service, argv[1], yield);
            }
            else {
              accept_and_run_chat(service, yield);
            }
        });

    boost::asio::io_service::work work(ios);
    ios.run();
}
