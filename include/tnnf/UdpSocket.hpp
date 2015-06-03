/*! \file UdpSocket.hpp
    \brief UDP send and receive*/

/*
Copyright (c) 2015 Máté Vágó

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgement in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef TN3E_UDPSOCKET_HPP
#define TN3E_UDPSOCKET_HPP

#include "Socket.hpp"

namespace tnnf {
    class UdpSocket : public Socket {
        private:
            static socklen_t msAddressLength; //const address length
        protected:

        public:
            /*! \fn UdpSocket(const Address& address)
                \brief Constructor for initializing a new UDP socket.
                \param address Ip address, which will be stored with the socket.*/
            UdpSocket(const Address& address) : Socket(address, SOCK_DGRAM, IPPROTO_UDP) {}

            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            UdpSocket(const UdpSocket& other) = default;
            UdpSocket& operator=(const UdpSocket& other) = default;
            UdpSocket(UdpSocket&& other) noexcept = default;
            UdpSocket& operator=(UdpSocket&& other) = default;

            /*! \fn void swap(UdpSocket& other)
                \brief Swap the references between two UdpSockets
                \param other Another UdpSocket*/
            void swap(UdpSocket& other) noexcept {
                UdpSocket temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            /*! \fn void bind()
                \brief Bind the socket to the stored address.*/
            void bind() noexcept {
                if(getAddress().getPort() == 0 || ::bind(getSocket(), getAddress().toSockaddr(), sizeof(sockaddr)) == -1) {
                    gSocketErrorFunction(*this, ERROR_SOCKET_BIND, errno);
                }
            }

            /*! \fn void bind(const Address& address)
                \brief Bind the socket to the given address. The address overwrite
                    the currently stored address.*/
            void bind(const Address& address) {
                mAddress = address;
                bind();
            }

            /*! \fn ~UdpSocket()
                \brief Destructor.*/
            virtual ~UdpSocket() {}

            /*! \fn void send(const Packet& packet, Address& address, const int& flags)
                \brief Sends the specified packet to the given address with the specified flags.
                \param packet
                \param address
                \param flags*/
            void send(const Packet& packet, Address& address, const int& flags) noexcept final {
                int bytesSent = 0, bytesLeft = sizeof(uint16_t), currentlySent = 0;
                size_t sendableData = htons(packet.getSize());

                while(bytesSent < sizeof(uint16_t)) {
                    if((currentlySent = ::sendto(getSocket(), ((char*) &sendableData) + bytesSent, bytesLeft, flags, address.toSockaddr(), msAddressLength)) == -1) {
                        gSocketErrorFunction(*this, ERROR_SOCKET_SEND, errno);
                        return;
                    }

                    bytesSent += currentlySent;
                    bytesLeft -= currentlySent;
                }

                bytesSent = 0;
                bytesLeft = sizeof(uint16_t);

                sendableData = htons(packet.getType());

                while(bytesSent < sizeof(uint16_t)) {
                    if((currentlySent = ::sendto(getSocket(), ((char*) &sendableData) + bytesSent, bytesLeft, flags, address.toSockaddr(), msAddressLength)) == -1) {
                        gSocketErrorFunction(*this, ERROR_SOCKET_SEND, errno);
                        return;
                    }

                    bytesSent += currentlySent;
                    bytesLeft -= currentlySent;
                }

                bytesSent = 0;
                bytesLeft = packet.getData().size();

                while(bytesSent < packet.getData().size()) {
                    if((currentlySent = ::sendto(getSocket(), packet.getData().c_str() + bytesSent, bytesLeft, flags, address.toSockaddr(), msAddressLength)) == -1) {
                        gSocketErrorFunction(*this, ERROR_SOCKET_SEND, errno);
                        return;
                    }

                    bytesSent += currentlySent;
                    bytesLeft -= currentlySent;
                }
            }

            /*! \fn void send(const Packet& packet, Address& address)
                \brief Sends the specified packet to the given address with the stored flags.
                \param packet
                \param address*/
            void send(const Packet& packet, Address& address) noexcept final {
                send(packet, address, mSendFlags);
            }

            /*! \fn void send(const Packet& packet, const int& flags)
                \brief Sends the specified packet to the stored address with the specified flags.
                \param packet
                \param flags*/
            void send(const Packet& packet, const int& flags) noexcept final {
                send(packet, getAddress(), flags);
            }

            /*! \fn void send(const Packet& packet)
                \brief Sends the specified packet to the stored address with the stored flags.
                \param packet*/
            void send(const Packet& packet) noexcept final {
                send(packet, getAddress(), mSendFlags);
            }

            /*! \fn void receive(PacketBuffer& buffer, Address& address, const int& flags)
                \brief Blocking until receives packet on given address.
                \param buffer Where the packets will be stored.
                \param address Where you will get packets.
                \param flags Specifies receiving flags for this receive.*/
            void receive(PacketBuffer& buffer, Address& address, const int& flags) noexcept final {
                ssize_t currentlyReceived = 0;

                do {
                    if((currentlyReceived = ::recvfrom(getSocket(), buffer.getBuffer() + buffer.getCurrentSize(), buffer.getSize(), flags, address.toSockaddr(), &msAddressLength)) <= 0) {
                        if(currentlyReceived == 0) {
                            gSocketErrorFunction(*this, ERROR_SOCKET_HANGUP, errno);
                            return;
                        }
                        else {
                            gSocketErrorFunction(*this, ERROR_SOCKET_RECEIVE, errno);
                            return;
                        }
                    }

                    buffer.buildPackets(currentlyReceived);
                } while(!buffer.isPacketStored());
            }

            /*! \fn void receive(PacketBuffer& buffer, Address& address)
                \brief Blocking until receives packet on the given address, with the stored flags.
                \param buffer Where the packets will be stored.
                \param address Where you will get packets.*/
            void receive(PacketBuffer& buffer, Address& address) noexcept final {
                receive(buffer, address, mReceiveFlags);
            }

            /*! \fn void receive(PacketBuffer& buffer, const int& flags)
                \brief Blocking until receives packet on the stored address.
                \param buffer Where the packets will be stored.
                \param flags Specify receiving flags for this receive.*/
            void receive(PacketBuffer& buffer, const int& flags) noexcept final {
                ssize_t currentlyReceived = 0;

                do {
                    if((currentlyReceived = ::recvfrom(getSocket(), buffer.getBuffer() + buffer.getCurrentSize(), buffer.getSize(), flags, 0, 0)) <= 0) {
                        if(currentlyReceived == 0) {
                            gSocketErrorFunction(*this, ERROR_SOCKET_HANGUP, errno);
                        }
                        else {
                            gSocketErrorFunction(*this, ERROR_SOCKET_RECEIVE, errno);
                        }
                    }

                    buffer.buildPackets(currentlyReceived);
                } while(!buffer.isPacketStored());
            }

            /*! \fn void receive(PacketBuffer& buffer)
                \brief Blocking until receives packet on the stored address, with the stored flags.
                \param buffer Where the packets will be stored.*/
            void receive(PacketBuffer& buffer) noexcept final {
                receive(buffer, mReceiveFlags);
            }

    };

    socklen_t UdpSocket::msAddressLength = sizeof(sockaddr);
}//tn3e
#endif // TN3E_UDPSOCKET_HPP
