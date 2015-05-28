/*! \file Socket.hpp
    \brief Holding the file descriptor and the ip address, provide interface to using sockets.*/

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

#ifndef TNNF_SOCKET_HPP
#define TNNF_SOCKET_HPP

#include <sys/socket.h>
#include <unistd.h>
#include <memory>

#include "Address.hpp"
#include "Packet.hpp"

namespace tnnf {

    class Socket;

    const uint32_t ERROR_SOCKET_CREATE = 100;        //! \var const uint32_t ERROR_SOCKET_CREATE
    const uint32_t ERROR_SOCKET_BIND = 110;          //! \var const uint32_t ERROR_SOCKET_BIND
    const uint32_t ERROR_SOCKET_CONNECT = 111;       //! \var const uint32_t ERROR_SOCKET_CONNECT
    const uint32_t ERROR_SOCKET_LISTEN = 112;        //! \var const uint32_t ERROR_SOCKET_LISTEN
    const uint32_t ERROR_SOCKET_ACCEPT = 113;        //! \var const uint32_t ERROR_SOCKET_ACCEPT
    const uint32_t ERROR_SOCKET_SETSOCKOPT = 114;    //! \var const uint32_t ERROR_SOCKET_SETSOCKOPT
    const uint32_t ERROR_SOCKET_GETSOCKOPT = 115;    //! \var const uint32_t ERROR_SOCKET_GETSOCKOPT
    const uint32_t ERROR_SOCKET_SEND = 116;          //! \var const uint32_t ERROR_SOCKET_SEND
    const uint32_t ERROR_SOCKET_RECEIVE = 117;       //! \var const uint32_t ERROR_SOCKET_RECEIVE
    const uint32_t ERROR_SOCKET_HANGUP = 118;        //! \var const uint32_t ERROR_SOCKET_HANGUP

    void DefaultSocketErrorCallback(Socket& faultySocket, const uint32_t& errorEvent, int& cErrno);

    typedef void (*SocketErrorFunction)(Socket&, const uint32_t&, int&); //function pointer typedef
    SocketErrorFunction gSocketErrorFunction = DefaultSocketErrorCallback; //function pointer

    /*! \fn SetSocketErrorCallback(ErrorFunction function)
        \brief You can define a function to handle socket errors. You have to use this function
            to set your error handling function active.
            Example:
            \code
                TnnfSocketErrorCallback(Socket& faultySocket, const uint32_t& errorEvent, int& cErrno) {
                    if(errorEvent = tnnf::ERROR_UNKNOW) {
                        std::cerr << "Unknown error." << std::endl;
                    }
                    else {
                        std::cerr << "Known error." << std::endl;
                    }
                }

                tnnf::SetSocketErrorCallback(TnnfSocketErrorCallback);
            \endcode
        \param function*/
    void SetSocketErrorCallback(SocketErrorFunction function) {
        gSocketErrorFunction = function;
    }

    /*! \class Socket
        \brief This is an abstract class. All other type of sockets derive from this.

        If you want to make a class derive from this one, you have to override this methods:
        \code
            void send(const Packet& packet, Address& address, const int& flags)
            void send(const Packet& packet, Address& address)
            void send(const Packet& packet, const int& flags)
            void send(const Packet& packet)
            void receive(PacketBuffer& buffer, Address& address, const int& flags)
            void receive(PacketBuffer& buffer, Address& address)
            void receive(PacketBuffer& buffer, const int& flags)
            void receive(PacketBuffer& buffer)
        \endcode */
    class Socket {
        private:

        protected:
            std::shared_ptr<int> mSocket;   //! \var std::shared_ptr<int> mSocket
            Address mAddress;               //! \var Address mAddress

            /*! \fn Socket(const std::shared_ptr<int>& sock, const Address& address)
                \brief Constructor. Used for Listener socket accept() method.
                    If you want to use this, you have to make the socket by POSIX calls
                    (like socket() or accept()). You have to call bind() too, if you want to
                    bind the socket to port. You have to be sure to provide an
                    std::shared_ptr exactly that variable which returned by socket() method,
                    because if the memory address change the other methods will not work.
                    Example:
                    \code
                        std::shared_ptr<int> sock = std::make_shared<int>(-1);
                        sockaddr_storage address;

                        sock = socket(PF_INET6, SOCK_STREAM, IPPROTOTCP)
                    \endcode
                    Automatically sets the socket option SO_REUSEADDR to 1
                \param sock An initialized socket carried by an std::shared_ptr.
                \param address Ip address, which will be stored with the socket.*/
            Socket(const std::shared_ptr<int>& sock, const Address& address) noexcept :
                mSocket(sock),
                mAddress(address),
                mSendFlags(0),
                mReceiveFlags(0)
            {
                setSocketOption(SO_REUSEADDR, 1);
            }

            /*! \fn Socket(const Address& address, const int& type, const int& protocol)
                \brief Constructor. Initializes a socket for you. Automatically sets the sockopt SO_REUSEADDR to 1.
                \param address Ip address, which will be stored with the socket.
                \param type The type of the socket. (like SOCK_STREAM or SOCK_DGRAM)
                \param protocol Has to fit to the type. (like IPPROTO_TCP or IPPROTO_UDP)*/
            Socket(const Address& address, const int& type, const int& protocol) noexcept :
                mSocket(nullptr),
                mAddress(address),
                mSendFlags(0),
                mReceiveFlags(0)
            {
                mSocket = std::make_shared<int>(-1);

                if(mAddress.isIPv6()) {
                    *mSocket = socket(PF_INET6, type, protocol);
                }
                else {
                    *mSocket = socket(PF_INET, type, protocol);
                }

                setSocketOption(SO_REUSEADDR, 1);
            }

            int mSendFlags, mReceiveFlags;

        public:
            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            Socket(const Socket& other) = default;
            Socket& operator=(const Socket& other) = default;
            Socket(Socket&& other) noexcept = default;
            Socket& operator=(Socket&& other) = default;

            /*! \fn ~Socket()
                \brief Close the socket if there is no more reference.*/
            virtual ~Socket() {
                if(mSocket.unique()) {
                    close(*mSocket);
                }
            }

            bool operator==(const Socket& other) const noexcept { return *this->mSocket == *other.mSocket; }
            bool operator!=(const Socket& other) const noexcept { return *this->mSocket != *other.mSocket; }

            /*! \fn void send()
                \brief On overridden send() methods you have to be sure that the specified packet is sent,
                with the specified flags. The address is optional, you can ignore it if you want.

                For example: UdpSocket always sends packets to the specified address or if it is
                unspecified then just to the default address. TcpSocket simply ignores the address
                variable, because it is not changeable.
                \param packet The packet which will be sent.
                \param address The address where the packet will arrive. (ignorable)
                \param flags The specified flags, which will be always used for once.*/
            virtual void send(const Packet& packet, Address& address, const int& flags) noexcept = 0;
            virtual void send(const Packet& packet, Address& address) noexcept = 0;
            virtual void send(const Packet& packet, const int& flags) noexcept = 0;
            virtual void send(const Packet& packet) noexcept = 0;

            /*! \fn void receive()
                \brief On overridden receive() methods you write the received data into the specified
                PacketBuffer, and let it build the packets.
                The address is optional, you can ignore it if you want.

                For example: See send()
                \param buffer The PacketBuffer which will be used.
                \param address The address where the packets came from. (ignorable)
                \param flags The specified flags, which will be always used for once.*/
            virtual void receive(PacketBuffer& buffer, Address& address, const int& flags) noexcept = 0;
            virtual void receive(PacketBuffer& buffer, Address& address) noexcept = 0;
            virtual void receive(PacketBuffer& buffer, const int& flags) noexcept = 0;
            virtual void receive(PacketBuffer& buffer) noexcept = 0;

            /*! \fn void setSendFlags(const int& flags)
                \brief Set the flags, which will be used every time at sending on this socket except,
                when the user specifies another one.
                \param flags*/
            void setSendFlags(const int& flags) noexcept {
                mSendFlags = flags;
            }

            /*! \fn void setReceiveFlags(const int& flags)
                \brief Sets the flags, which will be used every time at receiving on this socket except,
                when the user specifies another one.
                \param flags*/
            void setReceiveFlags(const int& flags) noexcept {
                mReceiveFlags = flags;
            }

            /*! \fn void setSocketOption(const int& optionName, const int& optionValue)
                \brief Sets socket options at socket level (SOL_SOCKET)
                \param optionName Specifies which socket option you want to modify. (like SO_REUSEADDR)
                \param optionValue 0 for disable, 1 for enable*/
            void setSocketOption(const int& optionName, const int& optionValue) noexcept {
                if(::setsockopt(*mSocket, SOL_SOCKET, optionName, &optionValue, sizeof(int)) == -1) {
                    gSocketErrorFunction(*this, ERROR_SOCKET_SETSOCKOPT, errno);
                }
            }

            /*! \fn int getSocketOption(const int& optionName)
                \brief Gets the value of a socket option at socket level (SOL_SOCKET)
                \param optionName Specifies which socket option you want to request. (like SO_REUSEADDR)
                \return 0 if disabled 1 if enabled, -1 on error, errno set to indicate the error*/
            int getSocketOption(const int& optionName) noexcept {
                int optionValue;
                socklen_t optionLength;

                if((optionValue = ::getsockopt(*mSocket, SOL_SOCKET, optionName, &optionValue, &optionLength)) == -1) {
                    gSocketErrorFunction(*this, ERROR_SOCKET_GETSOCKOPT, errno);
                }

                return optionValue;
            }

            /*! \fn const int& getSocket()
                \brief You can check this after creating a socket. (-1 on error, errno set to indicate the error)
                \return with a constant reference to the socket descriptor*/
            const int& getSocket() noexcept {
                return *mSocket;
            }

            /*! \fn Address& getAddress()
                \return with the stored address*/
            Address& getAddress() noexcept {
                return mAddress;
            }
    };

     /*! \fn void DefaultErrorFunction(Socket* faultySocket, const uint32_t& errorEvent, int& cErrno)
        \brief The default error callback. Print the errors to the std::cerr*/
    void DefaultSocketErrorCallback(Socket& faultySocket, const uint32_t& errorEvent, int& cErrno) {
        std::cerr << "TNNF_ERROR: On socket " << faultySocket.getSocket() << " " << strerror(cErrno) << std::endl;
    }
}//tnnf

#endif
