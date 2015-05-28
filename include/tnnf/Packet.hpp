/*! \file Packet.hpp
    \brief Encapsulate data*/

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

#ifndef TNNF_PACKET_HPP
#define TNNF_PACKET_HPP

#include <arpa/inet.h>

#include <limits>

#include "tnnf.hpp"

namespace tnnf {
    /*! \class Packet
        \brief This class for encapsulate your data
        to fit to the TCP sending methods, and identify it.*/
    class Packet {
        private:
            uint16_t mType, mSize;
            std::string mData;

        protected:

        public:
            /*! \fn Packet()
                \brief Default constructor. Construct an empty packet.*/
            Packet() noexcept :
                mType(EMPTY_PACKET_TYPE),
                mSize(2 * sizeof(uint16_t))
            {}

            /*! \fn Packet(const uint16_t& type, const std::string& data)
                \brief Constructor.
                    If you use EMPTY_PACKET_TYPE for type, your data will be ignored.
                    If your data size is bigger than Packet::maxSize an empty packet will be constructed.
                \param type A number between 0 and 65534. (65535 is reserved for empty packets)
                \param data Your serialized data.*/
            Packet(const uint16_t& type, const std::string& data) noexcept {
                if(type == EMPTY_PACKET_TYPE) {
                    mType = EMPTY_PACKET_TYPE;
                    mSize = 2 * sizeof(uint16_t);
                }
                else if(data.size() > maxSize - 2 * sizeof(uint16_t)) {
                    mType = EMPTY_PACKET_TYPE;
                    mSize = 2 * sizeof(uint16_t);

                    gCommonErrorFunction(ERROR_PACKET_TOO_BIG, "Packet size too big.");
                }
                else {
                    mType = type;
                    mData = data;
                    mSize = data.size() + 2 * sizeof(uint16_t);
                }
            }

            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            Packet(const Packet& other) = default;
            Packet& operator=(const Packet& other) = default;
            Packet(Packet&& other) noexcept  = default;
            Packet& operator=(Packet&& other) = default;

            /*! \fn void swap(Packet& other)
                \brief Swap the references between two Packets
                \param other Another Packet*/
            void swap(Packet& other) noexcept {
                Packet temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            //destructor
            ~Packet() {}

            /*! \fn const uint16_t& getType()
                \return constant reference to type*/
            const uint16_t& getType() const noexcept {
                return mType;
            }

            /*! \fn const uint16_t& getSize()
                \return constant reference to size*/
            const uint16_t& getSize() const noexcept {
                return mSize;
            }

            /*! \fn const uint16_t& getSize()
                \return reference to data*/
            std::string& getData() noexcept {
                return mData;
            }

            /*! \fn const uint16_t& getSize()
                \return constant reference to data*/
            const std::string& getData() const noexcept {
                return mData;
            }

            /*! \var maxSize
                \brief Maximum packet size.*/
            static uint16_t maxSize;

            /*! \var EMPTY_PACKET_TYPE
                \brief Indentify an empty packet. Default equal with 65535.*/
            static uint16_t EMPTY_PACKET_TYPE;
    };

    uint16_t Packet::maxSize = std::numeric_limits<uint16_t>::max(); //default 65535
    uint16_t Packet::EMPTY_PACKET_TYPE = std::numeric_limits<uint16_t>::max(); //default 65535
}//tnnf
#endif
