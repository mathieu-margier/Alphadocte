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
 * File: Exception.cpp
 */

#include <Alphadocte/Exceptions.h>


namespace Alphadocte {

Exception::Exception(const std::string& reason, const std::string& functionName)
        : std::runtime_error(reason), m_functionName{functionName}
{

}

const std::string& Exception::getFunctionName() const {
    return m_functionName;
}

std::string Exception::getFullMessage() const {
    return m_functionName + ": " + what();
}

InvalidArgException::InvalidArgException(const std::string& reason, const std::string& functionName)
        : Exception(reason, functionName)
{

}

} /* namespace Alphadocte */
