TNNF - Terribly Named NetFramework
===================================

TNNF is a lightweight header-only library for develop network applications.

Currently only operate with TCP.

### TNNF is able to...

* ...encapsulate your sendable data into a Packet, assign a type, and send it trough the network.
* ...build Packet from sequence of received bytes.
* ...watch connections with selector.

##FAQ

#### How to build a TCP connection?

Server side:
You will need a socket to look up new connection, and for this socket you will need an internet address, which trough you will listen. Recommended to specify a port too.

```cpp
#include "tnnf/Address.hpp"
#include "tnnf/ListenerSocket.hpp"

tnnf::Address listenerAddress("127.0.0.1", 25565); //We use the loopback address.
tnnf::ListenerSocket listener(listenerAddress, 10); //Arguments: An Address and the queueLength.

tnnf::TcpSocket client = listener.accept(); //Accept incoming connection.
```

Client side:
You just need the address of the server and connect to it.

```cpp
#include "tnnf/Address.hpp"
#include "tnnf/ClientSocket.hpp"

tnnf::ClientSocket server(tnnf::Address("127.0.0.1", 25565));

server.connect();
```

#### How to send data trough the TCP connection?

First of all, you need to serialize your data. I recommend [cereal](http://uscilab.github.io/cereal/).
This is not necessary, if you test only on your machine, but if you want to send your things to another computer, you have to be sure your data is serialized.

Send:

```cpp
#include "tnnf/Packet.hpp"

//I suppose you have a built connection.

tnnf::Packet msg1(0, "Hello!"); //Arguments: packet type, and the data.
tnnf::Packet msg2(1, std::to_string(21));

server.send(msg1);
server.send(msg2);
```

Receive:
```cpp
#include "tnnf/Packet.hpp"
#include "tnnf/PacketBuffer.hpp"

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
```

#### How to use the selector?

You can get lists of sockets from selector, which is readable, writable or got error. On The example we will only check the readable sockets. I recommend to override the default socket error callback, at least for handling the hang up.

```cpp
#include <vector>

#include "tnnf/Address.hpp"
#include "tnnf/Socket.hpp"
#include "tnnf/TcpSocket.hpp"
#include "tnnf/ListenerSocket.hpp"
#include "tnnf/Selector.hpp"

tnnf::PacketBuffer buffer;
tnnf::ListenerSocket listener(tnnf::Address("127.0.0.1", 25565), 10);
std::vector<tnnf::Socket*> readableSockets; //array, where the pointers to readable socket will be stored.
tnnf::Selector selector(&readableSockets, nullptr, nullptr); //Arguments: readable array, writable array, faulty array

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
```

#### How to handle errors?

You have to define two methods. One for socket errors, and one for others. After you just have to call SetCommonErrorCallback() and SetSocketErrorCallback() methods.
The socket error events are listed in Socket.hpp, and the common error codes are in tnnf.hpp.

```cpp
	void TnnfSocketErrorCallback(tnnf::Socket &faultySocket, const uint32_t &errorEvent, int &cErrno) {
		//handle errors here
	}
```
As you see, for socket errors you got three variables. The socket which failed, the event code and the POSIX errno. (you can examine it with strerror(cErrno) for error message)

```cpp
	void TnnfCommonErrorCallback(const uint32_t &errorEvent, const char* errorMessage) {
		//handle errors here
	}
```
For common errors you just got the error code and the error message.

