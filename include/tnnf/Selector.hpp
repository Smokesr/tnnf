/*! \file Selector.hpp
    \brief Storing sockets and following their state.*/

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

#ifndef TNNF_SELECTOR_HPP
#define TNNF_SELECTOR_HPP

#include <vector>

#include "Socket.hpp"

namespace tnnf {
    /*! \class Selector
        \brief A class that stores sockets and check their state.

        You have to add your sockets (which is inherited from class Socket)
        to the selector and frequently call the update() method to keep their
        status up to date. The Selector is watching which sockets has data, which
        can be written and which has an exception.*/
    class Selector {
        private:
            // Clear all user provided arrays.
            void clearTemp() noexcept {
                if(mWritable != nullptr) {
                    mWritable->clear();
                }
                if(mReadable != nullptr) {
                    mReadable->clear();
                }
                if(mFaulty != nullptr) {
                    mFaulty->clear();
                }
            }

            std::vector<Socket*> mSockets; //all sockets
            std::vector<Socket*>* mWritable, *mReadable, *mFaulty; //pointers to user provided arrays
            fd_set mFdSockets, mFdWritable, mFdReadable, mFdFaulty;
            fd_set* mFdWritablePointer, *mFdReadablePointer, *mFdFaultyPointer; //pointers to fd_set variables
            timeval* mTimeout; //this store the selector timeout
            int mSocketsMax; //store the largest socket number

        protected:

        public:
            /*! \fn Selector(std::vector<Socket*>* readable, std::vector<Socket*>* writable, std::vector<Socket*>* faulty)
                \brief Constructor.
                \param readable Array for the pointers of the readable sockets.
                \param writable Array for the pointers of the readable sockets.
                \param faulty Array for the pointers to the sockets which got exception.*/
            Selector(std::vector<Socket*>* readable, std::vector<Socket*>* writable, std::vector<Socket*>* faulty) noexcept :
                mWritable(nullptr),
                mReadable(nullptr),
                mFaulty(nullptr),
                mFdWritablePointer(nullptr),
                mFdReadablePointer(nullptr),
                mFdFaultyPointer(nullptr),
                mTimeout(nullptr),
                mSocketsMax(0)
            {
                FD_ZERO(&mFdSockets);
                FD_ZERO(&mFdWritable);
                FD_ZERO(&mFdReadable);
                FD_ZERO(&mFdFaulty);

                mTimeout = new timeval;
                mTimeout->tv_sec = 0;
                mTimeout->tv_usec = 0;

                if(writable == nullptr) {
                    mFdWritablePointer = nullptr;
                    mWritable = nullptr;
                }
                else {
                    setWritable(writable);
                }

                if(readable == nullptr) {
                    mFdReadablePointer = nullptr;
                    mReadable = nullptr;
                }
                else {
                    setReadable(readable);
                }

                if(faulty == nullptr) {
                    mFdFaultyPointer = nullptr;
                    mFaulty = nullptr;
                }
                else {
                    setFaulty(faulty);
                }
            }

            /*! \fn Copy and Move constructors and assignments
                \brief Copy methods deleted. Moving methods are available.*/
            Selector(const Selector& other) = delete;
            Selector& operator=(const Selector& other) = delete;
            Selector(Selector&& other) = default;
            Selector& operator=(Selector&& other) = default;

            /*! \fn void swap(Selector& other)
                \brief Swap the references between two Selector
                \param other Another Selector*/
            void swap(Selector& other) noexcept {
                Selector temp = std::move(*this);
                *this = std::move(other);
                other = std::move(temp);
            }

            /*! \fn ~Selector()
                \brief Destructor. All sockets will be unwatched and destroyed if it is necessary.*/
            ~Selector() {
                removeAll();
                delete mTimeout;
            }

            /*! \fn void update()
                \brief Get the state of the sockets and fill the user provided arrays.
                \return with 0 on timeout, -1 if error occurred, errno set to indicate the error.*/
            int update() {
                if(mFdReadablePointer != nullptr || mFdWritablePointer != nullptr || mFdFaultyPointer != nullptr) {
                    mFdWritable = mFdSockets;
                    mFdReadable = mFdSockets;
                    mFdFaulty = mFdSockets;

                    int selectError = 0;
                    if((selectError = select(mSocketsMax+1, mFdReadablePointer, mFdWritablePointer, mFdFaultyPointer, mTimeout)) <= 0) {
                        clearTemp();
                        return selectError;
                    }
                    else {
                        if(mFdWritablePointer != nullptr) {
                            mWritable->clear();

                            for(auto& i : mSockets) {
                                if(FD_ISSET(i->getSocket(), mFdWritablePointer)) {
                                    mWritable->push_back(i);
                                }
                            }
                        }

                        if(mFdReadablePointer != nullptr) {
                            mReadable->clear();

                            for(auto& i : mSockets) {
                                if(FD_ISSET(i->getSocket(), mFdReadablePointer)) {
                                    mReadable->push_back(i);
                                }
                            }
                        }

                        if(mFdFaultyPointer != nullptr) {
                            mFaulty->clear();

                            for(auto& i : mSockets) {
                                if(FD_ISSET(i->getSocket(), mFdFaultyPointer)) {
                                    mFaulty->push_back(i);
                                }
                            }
                        }
                        return selectError;
                    }
                }
                else {
                    return -2;
                }
            }

            /*! \fn void add(const socketType& sock)
                \brief Adds a socket to the Selector.

                You can use it without the template parameter:
                \code
                ClientSocket client(Address("127.0.0.1"));
                selector.add(client);
                \endcode
                \param sock The socket which will be stored.
                \tparam SocketType The type of the socket*/
            template<typename SocketType>
            void add(SocketType& sock) noexcept {
                FD_SET(sock.getSocket(), &mFdSockets);
                mSockets.push_back(new SocketType(sock));

                if(sock.getSocket() > mSocketsMax) {
                    mSocketsMax = sock.getSocket();
                }
            }

            /*! \fn void remove(Socket& sock)
                \brief Removes a socket. If the socket does not have more reference, it will be destroyed.
                \param sock The socket which will be removed.*/
            void remove(Socket& sock) noexcept {
                FD_CLR(sock.getSocket(), &mFdSockets);
                for(auto i = mSockets.begin(); i != mSockets.end(); i++) {
                    if(**i == sock) {
                        if((*i)->getSocket() == mSocketsMax) {
                            mSocketsMax = 0;
                        }
                        delete *i;
                        mSockets.erase(i);
                        break;
                    }
                }

                mSockets.shrink_to_fit();

                if(mSocketsMax == 0) {
                    for(auto& i : mSockets) {
                        if(i->getSocket() > mSocketsMax) {
                            mSocketsMax = i->getSocket();
                        }
                    }
                }
            }

            /*! \fn void removeAll()
                \brief Removes all socket from the selector.*/
            void removeAll() noexcept {
                FD_ZERO(&mFdSockets);
                FD_ZERO(&mFdWritable);
                FD_ZERO(&mFdReadable);
                FD_ZERO(&mFdFaulty);
                clearTemp();

                for(auto& i : mSockets) {
                    delete i;
                }
                mSockets.clear();
            }

            /*! \fn void setWritable(std::vector<Socket*>* array)
                \brief You can specify the array where the references to writable sockets will be stored.
                \param array*/
            void setWritable(std::vector<Socket*>* array) noexcept {
                if(array == nullptr) {
                    mWritable = nullptr;
                    mFdWritablePointer = nullptr;
                }
                else {
                    mWritable = array;
                    mFdWritablePointer = &mFdWritable;
                }
            }

            /*! \fn void setReadable(std::vector<Socket*>* array)
                \brief You can specify the array where the references to readable sockets will be stored.
                \param array*/
            void setReadable(std::vector<Socket*>* array) noexcept {
                if(array == nullptr) {
                    mReadable = nullptr;
                    mFdReadablePointer = nullptr;
                }
                else {
                    mReadable = array;
                    mFdReadablePointer = &mFdReadable;
                }
            }

            /*! \fn void setFaulty(std::vector<Socket*>* array)
                \brief You can specify the array where the references will be stored to
                that sockets which got exceptions.
                \param array*/
            void setFaulty(std::vector<Socket*>* array) noexcept {
                if(array == nullptr) {
                    mFaulty = nullptr;
                    mFdFaultyPointer = nullptr;
                }
                else {
                    mFaulty = array;
                    mFdFaultyPointer = &mFdFaulty;
                }
            }

            /*! \fn void setTimeout(const timeval& timeout)
                \brief Set timeout for update() method.
                \param timeout A reference to a timeval variable which will be copied.*/
            void setTimeout(const timeval& timeout) noexcept {
                if(mTimeout == nullptr) {
                    mTimeout = new timeval;
                }
                *mTimeout = timeout;
            }

            /*! \fn void setTimeout(timeval* timeout)
                \brief Set timeout for update() method.
                \param sec Seconds.
                \param sec Microseconds.*/
            void setTimeout(const int& sec, const int& usec) noexcept {
                if(mTimeout == nullptr) {
                    mTimeout = new timeval;
                }
                mTimeout->tv_sec = sec;
                mTimeout->tv_usec = usec;
            }

            /*! \fn std::vector<Socket*> getAll()
                \brief Get all stored sockets.
                \return The array which contains the references to the sockets.*/
            std::vector<Socket*> getAll() noexcept {
                std::vector<Socket*> temp;

                for(auto& i : mSockets) {
                    temp.push_back(i);
                }
                return temp;
            }
    };
}//tnnf

#endif
