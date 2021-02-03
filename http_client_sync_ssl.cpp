//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP SSL client, synchronous
//
//------------------------------------------------------------------------------

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

static const std::string kVersion("1.0.0");
static const std::string kHost("tusd.tusdemo.net");
static const std::string kPort("443");
static const std::string kTarget("/files/");


template <class Stream>
void Post(Stream& stream, size_t length)
{
    // Set up an HTTP GET request message
    boost::beast::http::request<http::string_body> req(http::verb::post, kTarget, 11);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(boost::beast::http::field::host, kHost);
    req.set("Tus-Resumable", kVersion);
    req.set("Upload-Length", length);
    std::cout << "POST request:\n" <<  req << std::endl;

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);
    std::cout << "POST response:\n" <<  res << std::endl;
}

template <class Stream>
void Head(Stream& stream)
{
    std::string location;
    std::cout << "Copy and paste the location" << std::endl;
    std::cin >> location;
    auto target(&location[24]);

    // Set up an HTTP GET request message
    boost::beast::http::request<http::string_body> req(http::verb::head, target, 11);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(boost::beast::http::field::host, kHost);
    req.set("Tus-Resumable", kVersion);
    std::cout << "HEAD request:\n" <<  req << std::endl;

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);
    std::cout << "HEAD response:\n" <<  res << std::endl;
}

template <class Stream>
void Patch(Stream& stream, const std::string& content, bool includeContentLength)
{
    std::string location;
    std::cout << "Copy and paste the location" << std::endl;
    std::cin >> location;
    auto target(&location[24]);

    // Set up an HTTP GET request message
    boost::beast::http::request<http::string_body> req(http::verb::patch, target, 11);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(boost::beast::http::field::host, kHost);
    req.set("Tus-Resumable", kVersion);
    req.set("Content-Type", "application/offset+octet-stream");
    req.set("Upload-Offset", 0);
    std::cout << "PATCH request:\n" <<  req << std::endl;
    req.body() = content;
    if (includeContentLength)
      req.set("Content-Length", content.size());

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);
    std::cout << "PATCH response:\n" <<  res << std::endl;
}

// Performs an HTTP GET and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 2)
        {
            std::cerr << "Usage: http-client-sync-ssl <filename>"<< std::endl;
            return EXIT_FAILURE;
        }
        const boost::filesystem::path filepath(argv[1]);

        // The io_context is required for all I/O
        net::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx(ssl::context::tlsv12_client);

            // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream.native_handle(), kHost.c_str()))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        // Look up the domain name
        auto const results = resolver.resolve(kHost, kPort);

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream).connect(results);

        // Perform the SSL handshake
        stream.handshake(ssl::stream_base::client);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Load file
        std::ifstream ifs(filepath.string(), std::ifstream::binary);
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        bool run(true);
        while (run)
        {
            std::cout <<
                "Choose you action: \n"
                "1. POST\n"
                "2. HEAD\n"
                "3. PATCH with no Content-Length\n"
                "4. PATCH with Content-Length\n"
                "Q. Quit"
                << std::endl;

            char c;
            std::cin >> c;
            switch (c)
            {
            case '1':
                Post(stream, content.size());
                break;
            case '2':
                Head(stream);
                break;
            case '3':
                Patch(stream, content, false);
                break;
            case '4':
                Patch(stream, content, true);
                break;
            case 'q':
            case 'Q':
              run = false;
              break;
            }
        }

        // Gracefully close the stream
        beast::error_code ec;
        stream.shutdown(ec);
        if(ec == net::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if(ec)
            throw beast::system_error{ec};
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

  // If we get here then the connection is closed gracefully
  return EXIT_SUCCESS;
}
