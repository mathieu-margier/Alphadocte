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
 * File: TxtDictionary.h
 */

#ifndef TXTDICTIONARY_H_
#define TXTDICTIONARY_H_

#include <Alphadocte/Dictionary.h>
#include <filesystem>
#include <string_view>


namespace Alphadocte {

/*
 * Implementation of the Dictionary interface, reading the word list from a .txt file.
 * All of the loaded words are kept in memory.
 */
class TxtDictionary : public Dictionary {
public:
    // Constructors
    /*
     * Creates a dictionary loading its words from a .txt file.
     * File loading is performed by #load(), not by the constructor.
     *
     * Arguments :
     * + filepath : path to the file either absolute or relative to the program's working directory.
     * + rng : random number generator already initialized, such as std::default_random_engine
     */
    TxtDictionary(std::filesystem::path filepath);

    // Default constructors / destructor
    virtual ~TxtDictionary() = default;
    TxtDictionary(const TxtDictionary &other) = default;
    TxtDictionary(TxtDictionary &&other) = default;
    TxtDictionary& operator=(const TxtDictionary &other) = default;
    TxtDictionary& operator=(TxtDictionary &&other) = default;

    // Getters
    /*
     * Return the txt file path from which the dictionary is loaded.
     */
    const std::filesystem::path& getFilepath() const;

    // Inherited methods
    /*
     * Load the dictionary words. If already loaded, this function does nothing and returns false.
     *
     * Return true if all the words have been loaded successfully, false otherwise.
     */
    bool load() override;

    /*
     * Return if the dictionary is loaded, ie if load has been called while returning true.
     */
    bool isLoaded() const override;

    // Fields
private:
    std::filesystem::path m_filepath;
};

} /* namespace Alphadocte */

#endif /* TXTDICTIONARY_H_ */
