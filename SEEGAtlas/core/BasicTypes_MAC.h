#pragma once


#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__

/**
 * @file BasicTypes.h
 *
 * Contains basic types definition
 *
 * @author Silvain Beriault
 */
#include <QtGlobal>

// Header files to include
// RIZ20151118 is not working on Yosemite
//#ifdef Q_OS_MAC //RIZ: different c++ compiler
    #include <memory>
//#else
//    #include <tr1/memory>
//#endif

#include <stdexcept>
#include <string>


// define NULL unless it has already been defined by a third party library
#ifndef NULL
#define NULL 0
#endif

// simpler macros to the std::tr1::shared_ptr and weak_ptr
// Also, if we want to use a different implementation of smart pointers, all we have to do is
// to redefine these two macros

// RIZ20151118 is not working on Yosemite
//#ifdef Q_OS_MAC
    #define mrilSmartPtr std::shared_ptr
    #define mrilWeakPtr std::weak_ptr
    #define static_pointer_cast std::static_pointer_cast
    #define dynamic_pointer_cast std::dynamic_pionter_cast
/*#else
    #define mrilSmartPtr std::tr1::shared_ptr
    #define mrilWeakPtr std::tr1::weak_ptr
    #define static_pointer_cast std::tr1::static_pointer_cast
    #define dynamic_pointer_cast std::tr1::dynamic_pionter_cast
#endif
*/

namespace seeg {
    /**
     * Simple exception class for all IGNSException
     */
    class IGNSException : public std::runtime_error {
    public:
        IGNSException() : runtime_error("IGNS Exception") {}
        IGNSException(const std::string& msg) : runtime_error(msg) {}
    };

#if ITK_VERSION_MAJOR > 3
#define ITK_THREAD_ID itk::ThreadIdType
#else
#define ITK_THREAD_ID int
#endif

}

#endif
