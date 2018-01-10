#include <iostream>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <unistd.h>

#include "i2pouichannel.h"
#include "service.h"

using namespace std;
using namespace boost;

using namespace ouichannel;
using namespace i2p_ouichannel;

unique_ptr<Channel> channel;

static const char* PRIVATE_KEY = "vGSGr4PutpxqnP4QMYpfu1LrD9R0l5GBQS0Y2dErGKoIEtVx25O0gvTlnKfr0DA~Y75rCQe0A2zKELhghsdA6~5aBAiQRJqdOEm-e5tmGIAicjDCzgNRJ-k1aA0d6m7p9zEk7mI5xWZkM1Ipkj-WiPc-QYh1sX6JS6e3~8BtZyMH1GAJMEe3-ZNY2-nE-H62rXlp8gm70fJkXEcfk3l12rLQsJyiTg2chCohuO9pJx8BIzWkFHy2j8icsmAMXDIMXuj6~U28AAWFuXFQs9Go~EVFOtKa7n~WboCc4wmxm7gT-GjTJxTQdY-sDkWJMyrxUMSPhsXwa3gNc0oD-envaUSSHPF4Fl3FO~TPuNbTiqurWk14dZ3-JGTNTHlzBdqfZGjY8VpbsDraaCKSGwVrHUdhtHolMJFdlWeuZyqYH6BSgbQPrI7tMkvr5oG~J6WMk39KvM-Xq6aJiq6fFqr7ls8-B2rf2xFIv8WnF2iBVi~8GNLAlj7t9pkqU1MkwP8EBQAEAAEAALqMSlCLnLUy7q8~e~iCc8fU7XVVIXAh8iD2XEWeqKsLbuchjRb44Kim0ociu6kEixRTgpDtfHvsUzS-nSWhTG00hT1xA8WbMKW-fhks-8X7vOxJO7xKY89KVrJmk6xMjX9cesLkhiJkP4m8KV4LYz036VISmwoivfNzPEB-ObDKMqd01BljEqyS4f82jFf2MqQLtno9JXLD7uKb17Pvd4ys24WmI8nvhtck5JeCx4ew-unvLSUNfgP5UaVB4Jk1NQDc4af9UVLuXfyLaYYgHTamQHn-Ap-VwfwkGr3Vn2YofI6TOEui0hyv0AR1JolMQ5CfklKl5cMDJhorBGizdfS-RjYAWRV9b4bWvSHZWpBxJj70a1En1Wm2zzezBMUbCQ==";

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

static void run_chat(const boost::system::error_code& ec, Channel* channel) {
    auto& ios = channel->get_io_service();

    if (ec) {
      cerr << "Failed connect/accept: " << ec.message() << endl;
      return;
    }

    // This co-routine reads always from the socket and write it to std out.
    asio::spawn(ios, [channel] (asio::yield_context yield) {
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

    // This co-routine reads from std input and send it to peer
    asio::spawn(ios, [channel, &ios] (auto yield) {
            system::error_code ec;
            asio::posix::stream_descriptor input(ios, ::dup(STDIN_FILENO)); 

            asio::streambuf buffer(512);

            cout << "Enter a message to the peer" << endl;
            while (true) {
              system::error_code ec;
              size_t size = asio::async_read_until(input, buffer, '\n',
                                                   yield[ec]);
              if (ec || !channel)
                break;

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
    cout << "Accepting" << endl;
    service.listen(PRIVATE_KEY, run_chat);
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
