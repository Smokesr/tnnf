/*! \file TcpSocket.hpp
    \brief TCP send and receive*/

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

#ifndef TNNF_TCPSOCKET_HPP
#define TNNF_TCPSOCKET_HPP

#include "Socket.hpp"
#include "Packet.hpp"
#include "PacketBuffer.hpp"

namespace tnnf {
    /*! \class TcpSocket
        \brief This class makes possible to build TCP connections, sending and receiving packets.*/
    class TcpSocket : public Socket {
        private:
            TcpSocket(const std::shared_ptr<int>& sock, const Address& address) noexcept : Socket(sock, address) {} //constructor for GetInstace method.

        protected:
            /*! \fn TcpSocket(const Address& address)
                \brief Constructor for initializing a new TCP socket.
                \param address Ip address, which will be stored with the socket.*/
            TcpSocket(const Address& address) : Socket(address, SOCK_STREAM, IPPROTO_TCP) {}

            /*! \fn static TcpSocket GetInstance(const std::shared_ptr<int>& sock, const Address& address)
                \brief Method to set a new socket with already initialized data.
                \param sock An initialized socket carried by std::shared_ptr.
                \param address Ip address, where the packets will be sent.
                \return with an instance of a new TcpSocket*/
            static TcpSocket GetInstance(const std::shared_ptr<int>& sock, const Address& address) {
                return TcpSocket(sock, address);
            }

        public:
            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            TcpSocket(const TcpSocket& other) = default;
            TcpSocket& operator=(const TcpSocket& other) = default;
            TcpSocket(TcpSocket&& other) noexcept = default;
            TcpSocket& operator=(TcpSocket&& other) = default;

            /*! \fn void swap(TcpSocket& other)
                \brief Swap the references between two TcpSockets
                \param other Another TcpSocket*/
            void swap(TcpSocket& other) noexcept {
                TcpSocket temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            /*! \fn ~TcpSocket()
                \brief Destructor.*/
            virtual ~TcpSocket() {}

            /*! \fn void send(const Packet& packet, Address& address, const int& flags)
                \brief Sends the specified packet to the stored address with the specified flags.
                \param packet
                \param address It will be ignored.
                \param flags*/
            void send(const Packet& packet, Address& address, const int& flags) noexcept final {
                send(packet, flags);
            }

            /*! \fn void send(const Packet& packet, Address& address)
                \brief Sends the specified packet to the stored address with the stored flags.
                \param packet
                \param address It will be ignored.*/
            void send(const Packet& packet, Address& address) noexcept final {
                send(packet, mSendFlags);
            }

            /*! \fn void send(const Packet& packet, const int& flags)
                \brief Sends the specified packet to the stored address with the specified flags.
                \param packet
                \param flags*/
            void send(const Packet& packet, const int& flags) noexcept final {
                int bytesSent = 0, bytesLeft = sizeof(uint16_t), currentlySent = 0;
                size_t sendableData = htons(packet.getSize());

                while(bytesSent < sizeof(uint16_t)) {
                    if((currentlySent = ::send(getSocket(), ((char*) &sendableData) + bytesSent, bytesLeft, flags)) == -1) {
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
                    if((currentlySent = ::send(getSocket(), ((char*) &sendableData) + bytesSent, bytesLeft, flags)) == -1) {
                        gSocketErrorFunction(*this, ERROR_SOCKET_SEND, errno);
                        return;
                    }

                    bytesSent += currentlySent;
                    bytesLeft -= currentlySent;
                }

                bytesSent = 0;
                bytesLeft = packet.getData().size();

                while(bytesSent < packet.getData().size()) {
                    if((currentlySent = ::send(getSocket(), packet.getData().c_str() + bytesSent, bytesLeft, flags)) == -1) {
                        gSocketErrorFunction(*this, ERROR_SOCKET_SEND, errno);
                        return;
                    }

                    bytesSent += currentlySent;
                    bytesLeft -= currentlySent;
                }
            }

            /*! \fn void send(const Packet& packet)
                \brief Sends the specified packet to the stored address with the stored flags.
                \param packet*/
            void send(const Packet& packet) noexcept final {
                return send(packet, mSendFlags);
            }

            /*! \fn void receive(PacketBuffer& buffer, Address& address, const int& flags)
                \brief Blocking until receives packet on the stored address.
                \param buffer Where the packets will be stored.
                \param address will be ignored.
                \param flags Specifies receiving flags for this receive.*/
            void receive(PacketBuffer& buffer, Address& address, const int& flags) noexcept final {
                receive(buffer, flags);
            }

            /*! \fn void receive(PacketBuffer& buffer, Address& address)
                \brief Blocking until receives packet on the stored address, with the stored flags.
                \param buffer Where the packets will be stored.
                \param address will be ignored.*/
            void receive(PacketBuffer& buffer, Address& address) noexcept final {
                receive(buffer, mReceiveFlags);
            }

            /*! \fn void receive(PacketBuffer& buffer, const int& flags)
                \brief Blocking until receives packet on the stored address.
                \param buffer Where the packets will be stored.
                \param flags Specify receiving flags for this receive.*/
            void receive(PacketBuffer& buffer, const int& flags) noexcept final {
                ssize_t currentlyReceived = 0;

                do {
                    if((currentlyReceived = ::recv(getSocket(), buffer.getBuffer() + buffer.getCurrentSize(), buffer.getSize()/2, flags)) <= 0) {
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

            /*! \fn void receive(PacketBuffer& buffer)
                \brief Blocking until receives packet on the stored address, with the stored flags.
                \param buffer Where the packets will be stored.*/
            void receive(PacketBuffer& buffer) noexcept final {
                receive(buffer, mReceiveFlags);
            }
    };
}//tnnf
#endif
