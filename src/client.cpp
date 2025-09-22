#include "taproot/client.h"
#include "taproot/config.h"
#include <asio.hpp>
#include <iostream>
#include <cctype>
#include <unordered_map>

Client::Client(asio::io_context& ctx, const Config& cfg) : host(cfg.host), port(cfg.port), ctx(ctx), socket(ctx), config(cfg) {
    // Set up interface
    asio::error_code err;
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(this->host, err), this->port); // use resolver here instead?
    if (err) {
        throw std::runtime_error(std::format("Invalid address '{}:{}'", this->host, this->port));
    }

    // Client connection retry loop
    const int maxRetries = 5;
    for (int i = 0; i < maxRetries; i++) {
        this->socket.connect(endpoint, err);
        if (!err) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cerr << "Retrying..." << std::endl;
    }

    if (err) {
       throw std::runtime_error(std::format("Failed to connect to address '{}:{}'. {}", this->host, this->port, err.message()));
    }
}

Client::~Client() {
    this->socket.close();
}

std::string Client::send(const std::string& data) {
    /* Send input data to server for dispatching. Input should be a command. Return a map response. */
    try {
        asio::write(this->socket, asio::buffer(data + "\n"));
        asio::streambuf buffer;
        std::ostringstream responseStream;
        while (true) {
            asio::read_until(this->socket, buffer, "END\n"); // For now, read until reach "END\n" delimiter
            std::istream is(&buffer);
            std::string chunk((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            responseStream << chunk;
            if (chunk.find("END\n") != std::string::npos) break;
        }
        std::string response = responseStream.str();
        response.erase(response.find("END\n"));
        return response;
    } catch (const std::system_error& e) {
        std::cerr << std::format("Server disconnected: {}\n", e.what());
        return "";
    } catch (const std::exception& e) {
        std::cerr << std::format("Client error: {}\n", e.what());
        return "";
    }
}