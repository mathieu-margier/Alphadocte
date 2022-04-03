/*
 * Copyright (C) 2022  Mathieu Margier
 *
 *  This file is part of Alphadocte.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 * 
 * File: Exception.h
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <stdexcept>
#include <string>

#include <Alphadocte/Alphadocte.h>

namespace Alphadocte {

/*
 * Base class for exception thrown by Alphadocte
 */
class Exception : public std::runtime_error {
public:
    /*
     * Base exception constructor, called by alphadocte's functions.
     *
     * Args:
     * - reason: message (unlocalized) explaining this exception, and return by member what()
     * - functionName: the function that will throw this exception, eg "Alphadocte:some_function(int, std::string)"
     */
    Exception(const std::string& reason, const std::string& functionName);

    // Default constructor
    virtual ~Exception() = default;
    Exception(const Exception &other) = default;
    Exception(Exception &&other) = default;
    Exception& operator=(const Exception &other) = default;
    Exception& operator=(Exception &&other) = default;

    // Getters
    /*
     * Return the function that thrown this exception
     */
    const std::string& getFunctionName() const;

    /*
     * Return the full message of the exception, concatenating the function name and the reason returned by what()
     */
    std::string getFullMessage() const;

private:
    std::string m_functionName;
};

/*
 * Exception thrown when the arguments given to a function do not match the requirements.
 */
class InvalidArgException : public Exception {
public:
    InvalidArgException(const std::string& reason, const std::string& functionName);

    // Default constructor
    virtual ~InvalidArgException() = default;
    InvalidArgException(const InvalidArgException &other) = default;
    InvalidArgException(InvalidArgException &&other) = default;
    InvalidArgException& operator=(const InvalidArgException &other) = default;
    InvalidArgException& operator=(InvalidArgException &&other) = default;

};



} /* namespace Alphadocte */

#endif /* EXCEPTIONS_H_ */
