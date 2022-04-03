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
 * File: Config.cpp
 */

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Config.h"
#include <Alphadocte/Exceptions.h>

namespace {
// parsing functions declarations
Alphadocte::CLI::Section parse(std::istream& input);
Alphadocte::CLI::Section parse_section(std::istream& input, const std::string& lookahead, std::istream& lineStream, unsigned int& lineNo);
Alphadocte::CLI::Entry parse_entry(const std::string& lookahead, std::string lineRemaining, unsigned int lineNo);
// write functions declarations
void write_root(std::ostream& output, const Alphadocte::CLI::Section& rootSection);
void write_section(std::ostream& output, const Alphadocte::CLI::Section& section);
void write_entry(std::ostream& output, const Alphadocte::CLI::Entry& entry);
}

namespace Alphadocte {

namespace CLI {

Config::Config() : m_rootSection{"root", {}, {}} {}

const Section& Config::getRootSection() const {
    return m_rootSection;
}

Section& Config::getRootSection() {
    return m_rootSection;
}

void Config::setRootSection(Section root) {
    m_rootSection = std::move(root);
}

// Methods
void Config::loadFromFile(const std::filesystem::path& filePath) {
    std::error_code error;

    if (!std::filesystem::is_regular_file(filePath, error) || error) {
        throw Alphadocte::Exception("File " + filePath.string() + " either does not exist, is not a file, or is not accessible.",
                "Alphadocte::CLI::Config::loadFromFile(const std::filesystem::path&)");
    }

    std::ifstream fileStream{filePath};
    setRootSection(parse(fileStream));
}

void Config::writeToFile(const std::filesystem::path& filePath) const {
    std::ofstream stream{filePath.c_str(), std::ios::binary}; // open in binary mode so that '\n' is not converted to '\r\n' on windows

    if (!stream.is_open()) {
        throw Alphadocte::Exception("Unable to open file " + filePath.string() + " in write mode.",
                "Alphadocte::CLI::Config::writeToFile(const std::filesystem::path&) const");
    }

    write_root(stream, m_rootSection);
    stream.close();

    if (stream.fail()) {
        throw Alphadocte::Exception("Unable to close correctly file " + filePath.string() + ", its content should be deemed as invalid.",
                "Alphadocte::CLI::Config::writeToFile(const std::filesystem::path&) const");
    }
}

void Config::clear() {
    m_rootSection.entries.clear();
    m_rootSection.sections.clear();
}

bool operator==(const Entry& lhs, const Entry& rhs) {
    return lhs.name == rhs.name && lhs.value == rhs.value;
}

bool operator==(const Section& lhs, const Section& rhs) {
    return lhs.name == rhs.name && lhs.entries == rhs.entries && rhs.sections == lhs.sections;
}

std::ostream& operator<<(std::ostream& os, const Entry& entry) {
    return os << entry.name << '=' << '"' << entry.value << '"';
}

std::ostream& operator<<(std::ostream& os, const Section& section) {
    os << section.name << '{';
    std::string sep{};
    for (const Entry& e : section.entries) {
        os << sep << e;
        sep = "; ";
    }

    // separate entries and sections if there is at least one entry
    os << sep;
    sep.clear();

    for (const Section& s : section.sections) {
        os << sep << s;
        sep = "; ";
    }
    return os << '}';
}

} /* namespace CLI */

} /* namespace Alphadocte */

namespace {
// parsing functions definitions

using namespace Alphadocte::CLI;
using namespace std::string_literals;

Section parse(std::istream& input) {
    std::string lookahead, line;
    std::istringstream iss;
    Section root;
    unsigned int lineNo{1};

    root.name = "root";
    while (input.good()) {
        iss.clear();
        lookahead.clear();

        std::getline(input, line);
        iss.str(line);
        iss >> lookahead;

        // ignore case for first word of line
        std::transform(std::begin(lookahead), std::end(lookahead), std::begin(lookahead),tolower);

        if (lookahead.empty() || lookahead.front() == CHAR_COMMENT) {
            // Ignore empty or comment lines
            lineNo++;
        } else if (lookahead == KW_BEGIN_SECTION) {
            // try to parse section
            root.sections.emplace_back(parse_section(input, lookahead, iss, lineNo));
            // lineNo is incremented by section parsing
        } else if (lookahead == KW_END_SECTION) {
            throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": got an end of section outside of a section.",
                    "<anonymous>::parse(std::istream&)");
        }
        else {
            // Default : entry identifier
            // -> try to parse entry
            auto streamPos = iss.tellg();

            if (streamPos != decltype(iss)::pos_type(-1)) {
                root.entries.emplace_back(parse_entry(lookahead, line.substr(streamPos), lineNo));
            }
            else {

                throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": key "s + lookahead + " must have a value associated."s,
                        "<anonymous>::parse(std::istream&)");
            }

            lineNo++;
        }
    }

    if (input.bad()) {
        throw Alphadocte::Exception("IO error occurred before reaching end of file.",
                "<anonymous>::parse(std::istream&)");
    }

    // empty file
    return root;
}

Section parse_section(std::istream& input, const std::string& lookahead, std::istream& lineStream, unsigned int& lineNo) {
    Section section;
    unsigned int beginLine{lineNo};

    // assume lookahead is KW_BEGIN_SECTION

    if (lineStream.bad()) {
        throw Alphadocte::Exception("IO error occurred while reading line "s + std::to_string(lineNo) + "."s,
                "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
    }

    // Retrieve section name
    lineStream >> section.name;

    if (lineStream.fail()) {
        throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": expected section name after "s + lookahead + ", got nothing."s,
                "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
    }

    // Make sure nothing comes after section name
    std::string discarded;
    lineStream >> discarded;

    if (!discarded.empty() && discarded.front() != CHAR_COMMENT) {
        throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": got "s + discarded + " after section name, expected nothing."s,
                "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
    }
    lineNo++;

    // Parse section content
    std::string line, lineLookahead;
    std::istringstream iss;
    bool endReached{false};
    while (input.good()) {
        iss.clear();
        lineLookahead.clear();

        std::getline(input, line);
        iss.str(line);
        iss >> lineLookahead;

        // ignore case for first word of line
        std::transform(std::begin(lineLookahead), std::end(lineLookahead), std::begin(lineLookahead), tolower);

        if (lineLookahead.empty() || lineLookahead.front() == CHAR_COMMENT) {
            // Ignore empty or comment lines
            lineNo++;
        } else if (lineLookahead == KW_END_SECTION) {
            // end current section
            std::string endSectionName;

            iss >> endSectionName;

            if (endSectionName.empty() || endSectionName.front() == CHAR_COMMENT) {
                throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": missing section name after end."s,
                        "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
            } else if (endSectionName != section.name) {
                throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": ending section with a different name (got "s
                        + endSectionName + ", expected "s + section.name + ")."s,
                        "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
            }

            discarded.clear();
            iss >> discarded;

            if (!discarded.empty() && discarded.front() != CHAR_COMMENT) {
                throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": got "s + discarded + " after section name, expected nothing.",
                        "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
            }

            endReached = true;
            lineNo++;
            break;
        } else if (lineLookahead == KW_BEGIN_SECTION) {
            // try to parse section
            section.sections.emplace_back(parse_section(input, lineLookahead, iss, lineNo));
            // lineNo is incremented by section parsing
        } else {
            // Default : entry identifier
            // -> try to parse entry
            auto streamPos = iss.tellg();

            if (streamPos != decltype(iss)::pos_type(-1)) {
                section.entries.emplace_back(parse_entry(lineLookahead, line.substr(streamPos), lineNo));
            }
            else {
                throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": key "s + lineLookahead + " must have a value associated."s,
                        "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
            }

            lineNo++;
        }
    }

    if (input.bad()) {
        throw Alphadocte::Exception("IO error occurred before reaching end of file."s,
                "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
    } else if (!endReached) {
        throw Alphadocte::Exception("Reached end of file without closing section "s + section.name + " begun at line "s + std::to_string(beginLine) + '.',
                "<anonymous>::parse_section(std::istream&, const std::string&, std::istream&, unsigned int&)");
    }

    return section;
}

Entry parse_entry(const std::string& lookahead, std::string lineRemaining, unsigned int lineNo) {
    Entry entry;

    // assume lookahead is not a reserved keyword
    entry.name = lookahead;

    auto commentIt = std::find(std::begin(lineRemaining), std::end(lineRemaining), CHAR_COMMENT);

    if (commentIt != std::end(lineRemaining)) {
        // ignore comment
        lineRemaining.erase(commentIt, std::end(lineRemaining));
    }

    if (std::size(lineRemaining) <= 1) {
        throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": key "s + entry.name + " must have a value associated."s,
                "<anonymous>::parse_entry(const std::string&, std::string, unsigned int)");
    }

    if (!std::isblank(static_cast<unsigned char>(lineRemaining.front()))) {
        throw Alphadocte::Exception("Line "s + std::to_string(lineNo) + ": key "s + entry.name + " must have a space after."s,
                "<anonymous>::parse_entry(const std::string&, std::string, unsigned int)");
    }

    lineRemaining.erase(std::begin(lineRemaining));
    entry.value = std::move(lineRemaining);

    return entry;
}

// write functions definitions
void write_root(std::ostream& output, const Alphadocte::CLI::Section& rootSection) {
    if (output.fail()) {
        throw Alphadocte::Exception("Unable to write config due to IO error.",
                "<anonymous>::write_root(std::ostream&, const Alphadocte::CLI::Section&)");
    }

    for (const auto& entry : rootSection.entries) {
        write_entry(output, entry);
    }

    for (const auto& section : rootSection.sections) {
        write_section(output, section);
    }
}

void write_section(std::ostream& output, const Alphadocte::CLI::Section& section) {
    if (output.fail()) {
        throw Alphadocte::Exception("Unable to write section named " + section.name + " due to IO error.",
                "<anonymous>::write_section(std::ostream&, const Alphadocte::CLI::Section&)");
    }

    output << KW_BEGIN_SECTION << CHAR_DEFAULT_WS << section.name << CHAR_NEW_LINE;

    for (const auto& entry : section.entries) {
        write_entry(output, entry);
    }

    for (const auto& s : section.sections) {
        write_section(output, s);
    }

    output << KW_END_SECTION << CHAR_DEFAULT_WS << section.name << CHAR_NEW_LINE;
}

void write_entry(std::ostream& output, const Alphadocte::CLI::Entry& entry) {
    if (output.fail()) {
        throw Alphadocte::Exception("Unable to write entry named " + entry.name + " due to IO error.",
                "<anonymous>::write_entry(std::ostream&, const Alphadocte::CLI::Entry&)");
    }

    output << entry.name << CHAR_DEFAULT_WS << entry.value << CHAR_NEW_LINE;
}

}
