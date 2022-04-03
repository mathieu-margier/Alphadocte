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
 * File: stubs/DictionaryStub.h
 */

#ifndef DICTIONARYSTUB_H_
#define DICTIONARYSTUB_H_

#include <Alphadocte/Dictionary.h>

namespace Alphadocte {

class DictionaryStub : public Dictionary {
public:
    DictionaryStub(std::vector<std::string> words);

    virtual ~DictionaryStub() = default;
    DictionaryStub(const DictionaryStub &other) = default;
    DictionaryStub(DictionaryStub &&other) = default;
    DictionaryStub& operator=(const DictionaryStub &other) = default;
    DictionaryStub& operator=(DictionaryStub &&other) = default;

    bool load() override;
    bool isLoaded() const override;
};

} /* namespace Alphadocte */

#endif /* DICTIONARYSTUB_H_ */
