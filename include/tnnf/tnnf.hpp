/*! \file tnnf.hpp
    \brief common functions, error handling*/

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

#ifndef TNNF_HPP
#define TNNF_HPP

#include <cstring>

//! \namespace tnnf
namespace tnnf {

    const uint32_t ERROR_UNKNOWN = 0; //! \var const uint32_t ERROR_UNKNOW

    const uint32_t ERROR_SELECTOR_FAIL = 300;        //! \var const uint32_t ERROR_SELECTOR_FAIL
    const uint32_t ERROR_SELECTOR_TIMEOUT = 301;     //! \var const uint32_t ERROR_SELECTOR_TIMEOUT
    const uint32_t ERROR_SELECTOR_NO_TARGET = 302;   //! \var const uint32_t ERROR_SELECTOR_NO_TARGET

    const uint32_t ERROR_PACKET_TOO_BIG = 200;   //! \var const uint32_t ERROR_PACKET_TOO_BIG
    const uint32_t ERROR_PACKETBUFFER_TOO_SMALL = 250;   //! \var const uint32_t ERROR_PACKETBUFFER_TOO_SMALL



    /*! \fn void DefaultCommonErrorFunction(const uint32_t& errorCode, const char* errorMessage)
        \brief The default error callback. Print the errors to the std::cerr*/
    void DefaultCommonErrorCallback(const uint32_t& errorCode, const char* errorMessage) {
        std::cerr << "TNNF_ERROR: " << errorMessage << std::endl;
    }

    typedef void (*CommonErrorFunction)(const uint32_t&, const char*); //function pointer typedef
    CommonErrorFunction gCommonErrorFunction = DefaultCommonErrorCallback; //function pointer

    /*! \fn SetCommonErrorCallback(ErrorFunction function)
        \brief You can define a function to handle common errors. You have to use this function
            to set your error handling function active.
            Example:
            \code
                TnnfCommonErrorCallback(const uint32_t& errorCode, const char* errorMessage) {
                    if(errorNumber == tnnf::ERROR_UNKNOW) {
                        std::cerr << "Unknown error." << std::endl;
                    }
                    else {
                        std::cerr << "Known error." << std::endl;
                    }
                }

                tnnf::SetCommonErrorCallback(TnnfCommonErrorCallback);
            \endcode
        \param function*/
    void SetCommonErrorCallback(CommonErrorFunction function) {
        gCommonErrorFunction = function;
    }

    //! \fn std::string GetLastError()
    std::string GetLastError() {
        return strerror(errno);
    }

    //! \fn void PrintLastError()
    void PrintLastError() {
        std::cout << "TNNF_ERROR: " << strerror(errno) << std::endl;
    }
}

#endif // TNNF_HPP
