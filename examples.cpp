#include "tnnf/Address.hpp"
#include "tnnf/Socket.hpp"
#include "tnnf/ListenerSocket.hpp"
#include "tnnf/ClientSocket.hpp"
#include "tnnf/Selector.hpp"

void server() {
    tnnf::Address listenerAddress("127.0.0.1", 25565); //We use the loopback address.
    tnnf::ListenerSocket listener(listenerAddress, 10); //Arguments: An Address and the queueLength.

    tnnf::TcpSocket client = listener.accept(); //Accept incoming connection.

    tnnf::PacketBuffer buffer;
    tnnf::Packet receivedMsg;

    client.receive(buffer);

    while(buffer.isPacketStored()) {
        receivedMsg = buffer.getPacket();

        if(receivedMsg.getType() == 0) {
            std::cout << receivedMsg.getData() << std::endl;
        }
        else if(receivedMsg.getType() == 1) {
            int number = 5;
            number += atoi(receivedMsg.getData().c_str());
            std::cout << number << std::endl;
        }
    }
}

void client() {
    tnnf::ClientSocket server(tnnf::Address("127.0.0.1", 25565));
    server.connect();

    //I suppose you have a built connection.

    tnnf::Packet msg1(0, "Hello!"); //Arguments: packet type, and the data.
    tnnf::Packet msg2(1, std::to_string(21));

    server.send(msg1);
    server.send(msg2);
}

tnnf::Selector* gSelector = nullptr;

void TnnfSocketErrorCallback(tnnf::Socket& faultySocket, const uint32_t& errorEvent, int& cErrno) {
    std::cerr << "TNNF_ERROR: On socket " << faultySocket.getSocket() << " " << strerror(cErrno) << std::endl;

    if(errorEvent == tnnf::ERROR_SOCKET_HANGUP) {
        gSelector->remove(faultySocket);
    }
}

void selector() {
    tnnf::PacketBuffer buffer;
    tnnf::ListenerSocket listener(tnnf::Address("127.0.0.1", 25565), 10);
    std::vector<tnnf::Socket*> readableSockets; //array, where the pointers to readable socket will be stored.
    tnnf::Selector selector(&readableSockets, nullptr, nullptr); //Arguments: readable array, writable array, faulty array

    gSelector = &selector;
    tnnf::SetSocketErrorCallback(TnnfSocketErrorCallback);

    selector.setTimeout(600, 0); //set timeout to 10 minutes.
    selector.add(listener); //add listener socket to the selector.

    bool exit = false;
    while(!exit) {
        selector.update(); //update the selector.

        for(auto& sock : readableSockets) { //handle all readable sockets.
            if(*sock == listener) {	//we got new connection, if the listener socket is readable
                tnnf::TcpSocket newClient = listener.accept(); //accept the new client
                selector.add(newClient); //and add to the selector
            }
            else { //if not the listener
                sock->receive(buffer); //receive the packets

                while(buffer.isPacketStored()) { //if the buffer not empty
                    tnnf::Packet receivedPacket = buffer.getPacket();

                    std::cout << receivedPacket.getType() << " - " << receivedPacket.getData() << std::endl;
                }
            }
        }
    }
}

int main() {
	std::cout << "0: TcpServer, 1: TcpClient, 2: TcpSelectorServer" << std::endl;

    int question = 0;	
    std::cin >> question;

    if(question == 0) {
        server();
    }
    if(question == 1) {
        client();
    }
    if(question == 2) {
        selector();
    }

    return 0;
}

