/*! \file ClientSocket.hpp
    \brief TCP client, can connect to a ListenerSocket*/

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

#ifndef TNNF_CLIENTSOCKET_HPP
#define TNNF_CLIENTSOCKET_HPP

#include "TcpSocket.hpp"

namespace tnnf {
    /*! \class ClientSocket
        \brief This class can connect to a ListenerSocket, and communicate with another TCP socket.*/
    class ClientSocket : public TcpSocket {
        private:

        protected:

        public:
            /*! \fn ClientSocket(const Address& serverAddress)
                \brief Constructor.
                \see Socket::getSocket() for error handling.
                \param serverAddress The Ip address of the server where you want to connect.*/
            ClientSocket(const Address& serverAddress) : TcpSocket(serverAddress) {}

            /*! \fn ClientSocket(const Address& serverAddress)
                \brief Constructor.
                \param serverAddress The Ip address of the server where you want to connect.
                \param address Your ip address, through which you want to communicate.
                    The socket will be bound to this address.*/
            ClientSocket(const Address& serverAddress, Address address) : TcpSocket(serverAddress) {
                if(::bind(getSocket(), address.toSockaddr(), sizeof(sockaddr)) == -1) {
                    gSocketErrorFunction(*this, ERROR_SOCKET_BIND, errno);
                }
            }

            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            ClientSocket(const ClientSocket& other) = default;
            ClientSocket& operator=(const ClientSocket& other) = default;
            ClientSocket(ClientSocket&& other) noexcept = default;
            ClientSocket& operator=(ClientSocket&& other) = default;

            /*! \fn void swap(ClientSocket& other)
                \brief Swaps the references between two ClientSockets
                \param other Another ClientSocket*/
            void swap(ClientSocket& other) noexcept {
                ClientSocket temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            //destructor
            ~ClientSocket() {}

            /*! \fn void connect()
                \brief Connect to the server.
                \return with -1 if error occurred, errno set to indicate the error.*/
            int connect() {
                return ::connect(getSocket(), getAddress().toSockaddr(), sizeof(sockaddr));
            }
    };
}//tnnf

#endif
