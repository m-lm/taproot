#include "taproot/server.h"
#include "taproot/db.h"
#include "taproot/config.h"
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>
#include <iostream>

Server::Server(asio::io_context& ctx, const Config& cfg) : host(cfg.host), port(cfg.port), ctx(ctx), socket(ctx), acceptor(ctx), config(cfg), db(std::make_unique<DB>("MyDB")) {
    // Networking interface (with IO context)
    asio::error_code err;

    // Address to be able to connect (IPv4)
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(this->host, err), this->port);

    // Set up server listener for client (with acceptor)
    this->acceptor.open(endpoint.protocol());
    this->acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    this->acceptor.bind(endpoint);
    this->acceptor.listen();
    std::cout << "Listening..." << std::endl;
}

Server::~Server() {
    if (this->acceptor.is_open()) {
        this->acceptor.close();
    }
    if (this->db) {
        this->db->shutdown();
    }
}

void Server::run() {
    /* Set up communication line with acceptor and socket. */
    this->running.store(true);
    while (this->running.load() && this->acceptor.is_open()) {
        asio::ip::tcp::socket socket(this->ctx);
        asio::error_code err;
        this->acceptor.accept(socket, err); // Block until client connects
        if (err) {
            if (err == asio::error::operation_aborted) break;
            if (err == asio::error::bad_descriptor) break;
            std::cerr << "Accept error: " << err.message() << "\n";
            continue;
        }
        // Spawn thread
        std::thread([this, s = std::move(socket)]() mutable {
            try {
                this->handleClient(std::move(s));
            } catch (const std::exception& e) {
                std::cerr << "Client thread exception: " << e.what() << "\n";
            }
        }).detach();
    }
    if (this->acceptor.is_open()) {
        this->acceptor.close();
    }
}

void Server::stop() {
    /* Stop listening of acceptor. */
    this->running.store(false);
    asio::error_code err;
    if (this->acceptor.is_open()) {
        this->acceptor.close(err);
    }
}

std::string Server::dispatcher(const std::string& input) {
    /* Dispatch incoming commands from client to DB, and then return a response. */
    /* Deals with APIs */
    std::vector<std::string> tokens = tokenize(input);
    size_t tokenSize = tokens.size();
    if (input == "stats") {
        return db->displayStats();
    }
    else if (input == "clear") {
        db->clearData();
        return "Data successfully deleted";
    }
    else if (tokens[0] == "use" && tokenSize == 2) {
        try {
            db->shutdown();
            db = std::make_unique<DB>(tokens[1]);
            return "Switched keyspace to " + tokens[1];
        } catch (const std::exception& e) {
            return std::format("Failed to switch keyspace to {}: {}", tokens[1], e.what());
        }
    }
    else if (input == "show") {
        return db->display();
    }
    else if (input == "keys") {
        return db->displayKeys();
    }
    else if (input == "values") {
        return db->displayValues();
    }
    else if (input == "quit" || input == "exit") {
        this->running.store(false);
        this->db->shutdown();
        return "-1";
    }
    else {
        std::vector<std::string> res = db->parseCommand(input); // Returns a vector
        if (res.empty()) {
            return "";
        }
        return printVector(res);
    }
}

void Server::handleClient(asio::ip::tcp::socket clientSocket) {
    /* Deserialize and parse incoming client data via TCP socket before passing into dispatcher. */
    auto remoteEndpoint = clientSocket.remote_endpoint();
    try {
        while (true) {
            asio::streambuf buffer;
            asio::read_until(clientSocket, buffer, "\n");

            std::istream is(&buffer);
            std::string incoming;
            std::getline(is, incoming);

            std::string response = this->dispatcher(incoming);
            asio::write(clientSocket, asio::buffer(response + "END\n"));
        }
    } catch (const std::exception& e) {
        std::cerr << std::format("Client {}:{} disconnected: {}\n", remoteEndpoint.address().to_string(), remoteEndpoint.port(), e.what());
    }
}