#include <iostream>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

#include <i2p_ouiservice_implemetation.h>
#include <i2p_ouichannel.h>

using namespace std;
using namespace gnunet_channels;

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

static void run_chat(unique_ptr<Channel>& c, asio::yield_context yield) {
    auto& ios = c->get_io_service();

    // Start printing received messages
    asio::spawn(ios, [&c] (asio::yield_context yield) {
            sys::error_code ec;
            asio::streambuf buffer(512);

            while (true) {
                size_t n = asio::async_read_until(*c, buffer, '\n', yield[ec]);

                if (ec || !c) return;

                cout << "Received: "
                     << remove_new_line(consume(buffer, n))
                     << endl;
            }
        });

    // Read from input and send it to peer
    asio::posix::stream_descriptor input(ios, ::dup(STDIN_FILENO));

    asio::streambuf buffer(512);

    while (true) {
        sys::error_code ec;
        size_t n = asio::async_read_until(input, buffer, '\n', yield[ec]);
        if (ec || !c) break;
        asio::async_write(*c, asio::buffer(consume(buffer, n)), yield[ec]);
        if (ec || !c) break;
    }
}

static void connect_and_run_chat( unique_ptr<Channel>& channel
                                , Service& service
                                , string target_id
                                , string port
                                , asio::yield_context yield)
{
    sys::error_code ec;

    cout << "Connecting to " << target_id << endl;
    channel->connect(target_id, port, yield[ec]);

    if (ec) {
        cerr << "Failed to connect: " << ec.message() << endl;
        return;
    }

    cout << "Connected" << endl;

    run_chat(channel, yield);
}

static void accept_and_run_chat( unique_ptr<Channel>& channel
                               , Service& service
                               , string port
                               , asio::yield_context yield)
{
    sys::error_code ec;

    cout << "Accepting on port \"" << port << "\"" << endl;

    {
        CadetPort p(service);
        p.open(*channel, port, yield[ec]);

        if (ec) {
            cerr << "Failed to accept: " << ec.message() << endl;
            return;
        }
    }

    cout << "Accepted" << endl;

    run_chat(channel, yield);
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

    I2POuiService service(argv[1], ios);

    string target_id;
    string port = argv[2];

    if (argc >= 4) {
        target_id = argv[3];
    }

    // Capture these signals so that we can disconnect gracefully.
    asio::signal_set signals(ios, SIGINT, SIGTERM);

    unique_ptr<Channel> channel;

    signals.async_wait([&](sys::error_code, int /* signal_number */) {
            channel.reset();
        });

    asio::spawn(ios, [&] (auto yield) {
            sys::error_code ec;

            service.async_setup(yield[ec]);

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
