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
 * File: Config.h
 */

#ifndef APPS_CONFIG_H_
#define APPS_CONFIG_H_

#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

namespace Alphadocte {

namespace CLI {

// parsing constants
inline const char CHAR_COMMENT = '#';
inline const char CHAR_DEFAULT_WS = ' ';
inline const char CHAR_NEW_LINE = '\n';
inline const std::string KW_BEGIN_SECTION = "begin";
inline const std::string KW_END_SECTION = "end";

// sections names

// entry names
inline const std::string ENTRY_FILE_PATH = "file_path";
inline const std::string ENTRY_FILE_TIMESTAMP = "file_timestamp";


struct Entry {
    std::string name;
    std::string value;
};

struct Section {
    std::string name{};
    std::vector<Entry> entries{};
    std::vector<Section> sections{};
};

bool operator==(const Entry& lhs, const Entry& rhs);
bool operator==(const Section& lhs, const Section& rhs);

std::ostream& operator<<(std::ostream& os, const Entry& entry);
std::ostream& operator<<(std::ostream& os, const Section& section);

/*
 * Config file keeping values in a tree-like objects,
 * where nodes are called 'sections' and leaves 'entries'.
 * Sections are composed of a name, and other sections and entries.
 * Entries have a key in lowercase, with a (string) value.
 *
 * Read and write config from/to file in a custom format,
 * where key is the first word (put in lowercase),
 * and value is the remaining line, excluding the first whitespace
 * after key and a potential comment starting by '#'.
 * Thus, a value cannot contain a line break, as this is not needed
 * for this application.
 *
 */
class Config {
public:
    Config();

    // Default constructors/destructor
    virtual ~Config() = default;
    Config(const Config &other) = default;
    Config(Config &&other) = default;
    Config& operator=(const Config &other) = default;
    Config& operator=(Config &&other) = default;

    // Getters/setters
    /*
     * Return the root section of the config as a const reference.
     * Section name is "root" by default.
     */
    const Section& getRootSection() const;

    /*
     * Return the root section of the config as a reference.
     * Section name is "root" by default.
     */
    Section& getRootSection();

    /*
     * Set the config's root section.
     */
    void setRootSection(Section root);

    // Methods
    /*
     * Read the configuration from the given path.
     * Does not modify the config is the file cannot be parsed.
     *
     * Args :
     * - filePath : path to the file where the configuration is stored.
     *
     * Throws :
     * - Exception : if the file cannot be parsed (either IO error, or bad format).
     */
    void loadFromFile(const std::filesystem::path& filePath);

    /*
     * Write the configuration to the file at the location specified by the path.
     * Erase the content of the file if it already exists, and ignore comments.
     *
     * Args :
     * - filePath : path to the file where the configuration will be stored.
     *
     * Throws :
     * - Exception : if the configuration cannot be saved (IO error).
     */
    void writeToFile(const std::filesystem::path& filePath) const;

    /*
     * Clear the configuration, erasing all entries and sections.
     */
    void clear();

private:
    // Fields
    Section m_rootSection;
};

} /* namespace CLI */

} /* namespace Alphadocte */

#endif /* APPS_CONFIG_H_ */
