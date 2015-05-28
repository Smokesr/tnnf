/*! \file PacketBuffer.hpp
    \brief build received packets from bytes and store them.*/

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

#ifndef TNNF_PACKETBUFFER_HPP
#define TNNF_PACKETBUFFER_HPP

#include <sstream>
#include <cstring>
#include <queue>

#include "Packet.hpp"

namespace tnnf {
    /*! \class PacketBuffer
        \brief This class builds packets from bytes and */
    class PacketBuffer {
        private:
            char* mBuffer; //recieved bytes
            size_t mSize, mCurrentlyStoredBytes; //size of the buffer, and how much is filled
            std::queue<Packet> mStoredPackets; //completed packages

        protected:

        public:
            /*! \fn PacketBuffer(const size_t& size = Packet::maxSize)
                \brief Constructor. It is recommended to make buffer twice as bigger,
                    than maximum size of one packet.
                    If you construct smaller buffer than Packet::maxSize, the
                    buffer set to nullptr
                \see getBuffer() for handling this error.
                \param size is size of the buffer. It is 65535 by default. */
            explicit PacketBuffer(const size_t& size = Packet::maxSize) :
                mBuffer(nullptr),
                mSize(size),
                mCurrentlyStoredBytes(0)
            {
                if(size >= Packet::maxSize) {
                    mBuffer = new char[mSize];
                }
                else {
                    mSize = 0;
                }
            }

            /*! \fn Copy and Move constructors and assignments
                \brief Copy and move are available.*/
            PacketBuffer(const PacketBuffer& other) :
                mBuffer(nullptr),
                mSize(other.mSize),
                mCurrentlyStoredBytes(other.mCurrentlyStoredBytes),
                mStoredPackets(other.mStoredPackets)
            {
                mBuffer = new char[mSize];

                for(size_t i = 0; i < mCurrentlyStoredBytes; i++) {
                    mBuffer[i] = other.mBuffer[i];
                }
            }

            PacketBuffer& operator=(const PacketBuffer& other) {
                mBuffer = new char[other.mSize];
                mSize = other.mSize;
                mCurrentlyStoredBytes = other.mCurrentlyStoredBytes;
                mStoredPackets = other.mStoredPackets;

                for(size_t i = 0; i < mCurrentlyStoredBytes; i++) {
                    mBuffer[i] = other.mBuffer[i];
                }
            }

            PacketBuffer(PacketBuffer&& other) noexcept = default;
            PacketBuffer& operator=(PacketBuffer&& other) = default;

            /*! \fn void swap(PacketBuffer& other)
                \brief Swap the references between two PacketBuffers
                \param other Another PacketBuffer*/
            void swap(PacketBuffer& other) noexcept {
                PacketBuffer temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            //destructor
            ~PacketBuffer() {
                if(mBuffer != nullptr) {
                    delete[] mBuffer;
                }
            }

            /*! \fn void buildPackets(const int& receivedBytes)
                \brief Build Packets from received bytes.
                \param receivedBytes*/
            void buildPackets(const int& receivedBytes) noexcept {
                mCurrentlyStoredBytes += receivedBytes;
                char* cursor = &mBuffer[0];
                uint16_t packetSize = 0;

                while(mCurrentlyStoredBytes >= sizeof(uint16_t)) { //if the buffer store enough bytes to get the size of the next packet
                    packetSize = ntohs( *((uint16_t*) cursor));

                    if(mCurrentlyStoredBytes >= packetSize) { //if there is the full packet
                        std::stringstream ss;
                        for(uint16_t i = 2 * sizeof(uint16_t); i < packetSize; i++) { //get the data
                            ss << mBuffer[i];
                        }

                        cursor = &mBuffer[sizeof(uint16_t)];
                        mStoredPackets.emplace(ntohs( *((uint16_t*) cursor)), ss.str()); //construct a new packet

                        for(uint16_t i = packetSize; i < mCurrentlyStoredBytes; i++) { //shrink the buffer
                            mBuffer[i - packetSize] = mBuffer[i];
                        }

                        mCurrentlyStoredBytes -= packetSize;
                        cursor = &mBuffer[mCurrentlyStoredBytes];
                        memset(cursor, 0, packetSize);
                        cursor = &mBuffer[0];
                    }
                    else {
                        break;
                    }
                }
            }

            /*! \fn bool isPacketsStored()
                \return true if there are completed packets, false when not*/
            bool isPacketsStored() noexcept {
                return !mStoredPackets.empty();
            }

            /*! \fn const size_t& getNumOfStoredPackets()
                \return a constant reference to number of completed packets*/
            const size_t& getNumOfStoredPackets() noexcept {
                return mStoredPackets.size();
            }

            /*! \fn char* getBuffer()
                \return a pointer to the buffer. (nullptr if constructor failed.)*/
            char* getBuffer() noexcept {
                return mBuffer;
            }

            /*! \fn const size_t& getSize()
                \return the maximum size of the buffer*/
            const size_t& getSize() const noexcept {
                return mSize;
            }


            /*! \fn const size_t& getCurrentSize()
                \return how many bytes the buffer contain yet.*/
            const size_t& getCurrentSize() const noexcept {
                return mCurrentlyStoredBytes;
            }

            /*! \fn Packet getPacket()
                \return the first completed packet*/
            Packet getPacket() noexcept {
                Packet packet = mStoredPackets.front();
                mStoredPackets.pop();
                return packet;
            }
    };
}//tnnf
#endif
