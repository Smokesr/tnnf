/*! \file ListenerSocket.hpp
    \brief TCPSocket, accept new connections*/

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

#ifndef TNNF_LISTENERSOCKET_HPP
#define TNNF_LISTENERSOCKET_HPP

#include "TcpSocket.hpp"

namespace tnnf {
    /*! \class ListenerSocket
        \brief This class wait for tcp connections, and make a socket from them.*/
    class ListenerSocket : public TcpSocket {
        private:
            unsigned int mQueueLength; //max queue
            static socklen_t TNNF_SOCKADDR_LENGTH; //size of address.

        protected:

        public:
            /*! \fn ListenerSocket(const Address& address, unsigned int queueLength)
                \brief Constructor. You have to specify an address, where the socket will
                    listen to connections, and the number, which describes
                    how many connections can wait for the acception at the same time.
                \param address You have to specify the port too.
                \param queueLength the length of the queue
                \throw tnnf_error with error code TNNF_ERROR_SOCKET_CREATE
                \throw tnnf_error with error code TNNF_ERROR_SOCKET_BIND, if the port is null,
                    or something failed at a binding.
                \throw tnnf_error with error code TNNF_ERROR_SOCKET_LISTEN*/
            ListenerSocket(const Address& address, unsigned int queueLength) : TcpSocket(address)  {
                if(getAddress().getPort() == 0 || ::bind(getSocket(), getAddress().toSockaddr(), sizeof(sockaddr)) == -1) {
                    gSocketErrorFunction(*this, ERROR_SOCKET_BIND, errno);
                }

                if(::listen(getSocket(), mQueueLength) == -1) {
                    gSocketErrorFunction(*this, ERROR_SOCKET_LISTEN, errno);
                }
            }

            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            ListenerSocket(const ListenerSocket& other) = default;
            ListenerSocket& operator=(const ListenerSocket& other) = default;
            ListenerSocket(ListenerSocket&& other) = default;
            ListenerSocket& operator=(ListenerSocket&& other) = default;

            /*! \fn void swap(Address& other)
                \brief Swap the references between two Addresses
                \param other Another Address*/
            void swap(ListenerSocket& other) noexcept {
                ListenerSocket temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            //destructor
            ~ListenerSocket() {

            }

            /*! \fn TcpSocket accept()
                \brief accept a waiting socket.
                \return with a TcpSocket, which is connected to the same address as listener,
                    but different port. The instance will be equal with -1 if error occurred,
                    errno set to indicate the error.*/
            TcpSocket accept() {
                std::shared_ptr<int> sock = std::make_shared<int>(-1);
                sockaddr_storage address;

                if((*sock = ::accept(getSocket(), (sockaddr*)&address, &TNNF_SOCKADDR_LENGTH)) == -1) {
                    gSocketErrorFunction(*this, ERROR_SOCKET_ACCEPT, errno);
                }

                return GetInstance(sock, Address(address));
            }
    };

    socklen_t ListenerSocket::TNNF_SOCKADDR_LENGTH = sizeof(sockaddr_storage); //set to big enough
}//tnnf

#endif
