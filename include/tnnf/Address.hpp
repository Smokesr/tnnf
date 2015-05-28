/*! \file Address.hpp
    \brief store IPv4 or IPv6 address*/

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

#ifndef TNNF_ADDRESS_HPP
#define TNNF_ADDRESS_HPP

#include <arpa/inet.h>
#include <netdb.h>

#include <memory>
#include <cstring>
#include <algorithm>

namespace tnnf {
    /*! \class Address
        \brief This class store an IPv4 or IPv6 address*/
    class Address {
        private:
            sockaddr_storage mAddress; //enough space for ipv6 too

        protected:

        public:

            /*! \fn Address(const std::string& address)
                \brief Constructor for initializing an address from an Inet address.
                    You have to specify the version of the Internet Protocol.
                \param address*/
            Address(const std::string& address) noexcept {
                if(std::find(address.begin(), address.end(), ':') == address.end()) {
                    memset(&mAddress, 0, sizeof(mAddress));
                    sockaddr_in* pAddress = ((sockaddr_in*) &mAddress);
                    pAddress->sin_family = PF_INET; //family
                    pAddress->sin_port = htons(0); //port

                    inet_pton(PF_INET, address.c_str(), &(pAddress->sin_addr));
                }
                else {
                    memset(&mAddress, 0, sizeof(mAddress));
                    sockaddr_in6* pAddress = ((sockaddr_in6*) &mAddress);
                    pAddress->sin6_family = PF_INET6; //family
                    pAddress->sin6_port = htons(0); //port

                    inet_pton(PF_INET6, address.c_str(), &(pAddress->sin6_addr));
                }
            }

            /*! \fn Address(const std::string& address, const uint16_t& port)
                \brief Constructor for initializing an address from an Inet address, and a port.
                    You have to specify the version of the Internet Protocol.
                \param address
                \param port*/
            Address(const std::string& address, const uint16_t& port) {
                if(std::find(address.begin(), address.end(), ':') == address.end()) {
                    memset(&mAddress, 0, sizeof(mAddress));
                    sockaddr_in* pAddress = ((sockaddr_in*) &mAddress);
                    pAddress->sin_family = PF_INET; //family
                    pAddress->sin_port = htons(port); //port

                    inet_pton(PF_INET, address.c_str(), &(pAddress->sin_addr));
                }
                else {
                    memset(&mAddress, 0, sizeof(mAddress));
                    sockaddr_in6* pAddress = ((sockaddr_in6*) &mAddress);
                    pAddress->sin6_family = PF_INET6; //family
                    pAddress->sin6_port = htons(port); //port

                    inet_pton(PF_INET6, address.c_str(), &(pAddress->sin6_addr));
                }
            }

            /*! \fn Address(sockaddr_in& address)
                \brief Construct from struct sockaddr_in*/
            Address(sockaddr_in& address) noexcept {
                mAddress = *((sockaddr_storage*)&address);
            }

            /*! \fn Address(sockaddr_in6& address)
                \brief Construct from struct sockaddr_in6*/
            Address(sockaddr_in6& address) noexcept {
                mAddress = *((sockaddr_storage*)&address);
            }

            /*! \fn Address(sockaddr& address)
                \brief Construct from struct sockaddr*/
            Address(sockaddr& address) noexcept {
                mAddress = *((sockaddr_storage*)&address);
            }

            /*! \fn Address(sockaddr_storage& address)
                \brief Construct from struct sockaddr_storage*/
            Address(sockaddr_storage& address) noexcept {
                mAddress = address;
            }

            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            Address(const Address& other) = default;
            Address& operator=(const Address& other) = default;
            Address(Address&& other) noexcept = default;
            Address& operator=(Address&& other) = default;

            /*! \fn void swap(Address& other)
                \brief Swap the references between two Addresses
                \param other Another Address*/
            void swap(Address& other) noexcept {
                Address temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            //destructor
            ~Address() {

            }

/* *for later purposes*
            void dnsLookUp() {
             int error;
                addrinfo hints, *serviceInfo;

                memset(&hints, 0, sizeof(hints));
                hints.ai_family = AF_UNSPEC;

                if ((error = getaddrinfo(address.c_str(), 0, &hints, &serviceInfo)) != 0) {
                    gai_strerror(error)
                }

                mAddress = *((sockaddr_storage*)serviceInfo->ai_addr);

                freeaddrinfo(serviceInfo);

                if(isIpV6()) {
                    ((sockaddr_in6*)&mAddress)->sin6_port = 0;
                }
                else {
                    ((sockaddr_in*)&mAddress)->sin_port = 0;
                }
            }

*/
            /*! \fn bool isIPv6()
                \brief Gets the protocol version of the address.
                \return true if the address is ipv6, false if ipv4*/
            bool isIPv6() noexcept {
                if(mAddress.ss_family == AF_INET) {
                    return false;
                }
                else {
                    return true;
                }
            }

            /*! \fn sockaddr* toSockaddr()
                \return A sockaddr pointer to the address.*/
            sockaddr* toSockaddr() noexcept {
                return (sockaddr*)&mAddress;
            }

            /*! \fn uint16_t getPort()
                \return the port in the machine's endianness.*/
            uint16_t getPort() noexcept {
                if(isIPv6()) {
                    sockaddr_in6* inet6Address = (sockaddr_in6*)&mAddress;
                    return ntohs(inet6Address->sin6_port);
                }
                else {
                    sockaddr_in* inetAddress = (sockaddr_in*)&mAddress;
                    return ntohs(inetAddress->sin_port);
                }
            }

            /*! \fn std::string getIp()
                \return the ip as a string*/
            std::string getIp() noexcept {
                if(isIPv6()) {
                    sockaddr_in6* inet6Address = (sockaddr_in6*)&mAddress;
                    char strAddress[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &(inet6Address->sin6_addr), strAddress, INET6_ADDRSTRLEN);
                    return strAddress;
                }
                else {
                    sockaddr_in* inetAddress = (sockaddr_in*)&mAddress;
                    char strAddress[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(inetAddress->sin_addr), strAddress, INET_ADDRSTRLEN);
                    return strAddress;
                }
            }
    };
}//tnnf

#endif
