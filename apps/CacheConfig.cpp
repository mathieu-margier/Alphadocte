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
 * File: CacheConfig.cpp
 */

#include <algorithm>

#include "CacheConfig.h"
#include <Alphadocte/Exceptions.h>

namespace Alphadocte {

namespace CLI {

CacheConfig::CacheConfig(std::filesystem::path dictionaryPath)
         : m_config{} {
    std::error_code error;
    if (!std::filesystem::is_regular_file(dictionaryPath, error) || error) {
        throw Alphadocte::Exception("Dictionary at " + dictionaryPath.string() + " is not a file.",
                "Alphadocte::CLI::CacheConfig::CacheConfig(std::filesystem::path)");
    }

    auto& root = m_config.getRootSection();
    root.entries.emplace_back(Entry{ENTRY_FILE_PATH, std::filesystem::absolute(dictionaryPath).string()});
    auto timestamp = std::filesystem::last_write_time(dictionaryPath, error);

    if (error) {
        throw Alphadocte::Exception("Could not read last write time of dictionary at " + dictionaryPath.string(),
                "Alphadocte::CLI::CacheConfig::CacheConfig(std::filesystem::path)");
    }
    root.entries.emplace_back(Entry{ENTRY_FILE_TIMESTAMP, std::to_string(timestamp.time_since_epoch().count())});
}

CacheConfig::CacheConfig(Config config)
        : m_config{std::move(config)} {

    if (!isCacheValid()) {
        throw Alphadocte::InvalidArgException("Invalid configuration supplied as cache.",
                "Alphadocte::CLI::CacheConfig::CacheConfig(Alphadocte::CLI:Config)");
    }
}

const Config& CacheConfig::getConfig() const {
    return m_config;
}

void CacheConfig::setConfig(Config config) {
    std::swap(m_config, config);

    if (!isCacheValid()) {
        std::swap(m_config, config);
        throw Alphadocte::InvalidArgException("Invalid configuration supplied as cache.",
                "Alphadocte::CLI::CacheConfig::setConfig(Alphadocte::CLI::Config config)");
    }
}

std::filesystem::path CacheConfig::getDictionaryPath() const {
    auto it = std::find_if(
            std::cbegin(m_config.getRootSection().entries),
            std::cend(m_config.getRootSection().entries),
            [](const auto& entry){ return entry.name == ENTRY_FILE_PATH; });

    if (it == std::cend(m_config.getRootSection().entries)) {
        throw Alphadocte::Exception("Missing entry " + ENTRY_FILE_PATH,
                "Alphadocte::CLI::CacheConfig::getDictionaryPath()");
    }

    std::filesystem::path dictPath{it->value};

    if (!dictPath.is_absolute()) {
        throw Alphadocte::Exception("Value of " + ENTRY_FILE_PATH + " is not an absolute path.",
                "Alphadocte::CLI::CacheConfig::getDictionaryPath()");
    }

    return dictPath;
}

std::filesystem::file_time_type CacheConfig::getDictionaryTimestamp() const {
    auto it = std::find_if(
            std::cbegin(m_config.getRootSection().entries),
            std::cend(m_config.getRootSection().entries),
            [](const auto& entry){ return entry.name == ENTRY_FILE_TIMESTAMP; });

    if (it == std::cend(m_config.getRootSection().entries)) {
        throw Alphadocte::Exception("Missing entry " + ENTRY_FILE_TIMESTAMP,
                "Alphadocte::CLI::CacheConfig::getDictionaryTimestamp()");
    }

    std::istringstream timeIss{it->value};
    std::filesystem::file_time_type::rep timeClock;
    timeIss >> timeClock;

    if (timeIss.fail()) {
        throw Exception("Value of " + ENTRY_FILE_TIMESTAMP + " is not a valid timestamp.",
                "Alphadocte::CLI::CacheConfig::getDictionaryTimestamp()");
    }

    return std::filesystem::file_time_type{std::filesystem::file_time_type::duration{timeClock}};
}

namespace {

// keep helper functions local to this translation unit

/*
 * Check if the given solver section is correct.
 *
 * Throws:
 * - Alphadocte::Exception if the section is incorrect
 */
void check_solver_section(const Section& solverSection, std::string_view solverName, unsigned int solverVersion, const std::string& functionName) {
    auto solverVersionIt = std::find_if(
            std::cbegin(solverSection.entries),
            std::cend(solverSection.entries),
            [](const auto& e) { return e.name == CacheConfig::ENTRY_SOLVER_VERSION; }
    );

    if (solverVersionIt == std::cend(solverSection.entries)) {
        throw Alphadocte::Exception("Missing entry " + CacheConfig::ENTRY_SOLVER_VERSION + " in section solver " + std::string(solverName) + '.', functionName);
    }

    unsigned long solverVersionParsed{};
    try {
        solverVersionParsed = std::stoul(solverVersionIt->value);
    } catch (const std::exception& e) {
        throw Alphadocte::Exception("Invalid value for entry " + CacheConfig::ENTRY_SOLVER_VERSION + " : " + solverVersionIt->value + " is not a positive integer.", functionName);
    }

    if (solverVersionParsed != solverVersion) {
        throw Alphadocte::Exception("Actual solver version is different from entry's " + CacheConfig::ENTRY_SOLVER_VERSION + " : got "
                + std::to_string(solverVersionParsed) + ", expected " + std::to_string(solverVersion) + '.', functionName);
    }
}

}

std::vector<std::pair<std::string, double>> CacheConfig::getTopGuesses(
        std::string_view solverName,
        unsigned int solverVersion,
        unsigned int requestedNumberGuesses,
        std::string_view templateWord) const {

    std::vector<std::pair<std::string, double>> guesses;
    const auto& solverSection = getSolverSection(solverName);
    check_solver_section(solverSection, solverName, solverVersion,
            "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");

    const auto& guessSection = getGuessSection(solverSection, templateWord);
    auto guessNumberIt = std::find_if(
            std::cbegin(guessSection.entries),
            std::cend(guessSection.entries),
            [](const auto& e) { return e.name == ENTRY_GUESS_NUMBER; }
    );

    if (guessNumberIt == std::cend(guessSection.entries)) {
        throw Alphadocte::Exception("Missing entry " + ENTRY_GUESS_NUMBER + " in section guess with template " + std::string(templateWord),
                "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
    }
    unsigned long guessNumberParsed{};
    try {
        guessNumberParsed = std::stoul(guessNumberIt->value);
    } catch (const std::exception& e) {
        throw Alphadocte::Exception("Invalid value for entry " + ENTRY_GUESS_NUMBER + " : " + guessNumberIt->value + " is not a positive integer.",
                "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
    }

    if (guessNumberParsed < requestedNumberGuesses) {
        throw Alphadocte::Exception(std::string("Not enough guesses in cache."),
                "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
    }

    size_t i = 0;
    for (const auto& entry : guessSection.entries) {
        if (entry.name != ENTRY_GUESS_GUESS)
            continue;

        size_t spacePos = entry.value.find(' ');

        if (spacePos == entry.value.npos) {
            throw Alphadocte::Exception("Values of entry " + ENTRY_GUESS_GUESS + " must be separated by a space.",
                    "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
        }

        std::string guessName{entry.value.substr(0, spacePos)};
        std::string guessTrust{entry.value.substr(spacePos)};

        if (std::size(guessName) != std::size(templateWord)) {
            throw Alphadocte::Exception("Guess " + guessName + " does not have the same number of letters as template \"" + std::string(templateWord) + '"' + '.',
                    "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
        }

        std::transform(std::begin(guessName), std::end(guessName), std::begin(guessName), tolower);
        if (!std::all_of(std::cbegin(guessName),std::cend(guessName), islower)) {
            throw Alphadocte::Exception("Guess " + guessName + " contains invalid characters.",
                    "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
        }

        bool matchesTemplate{true};
        for (size_t i = 0; i < std::size(guessName); i++) {
            if (templateWord.at(i) != '.' && templateWord.at(i) != guessName.at(i)) {
                matchesTemplate = false;
                break;
            }
        }

        if (!matchesTemplate) {
            throw Alphadocte::Exception("Guess " + guessName + " does not match the template \"" + std::string(templateWord) + '"' + '.',
                    "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
        }

        double trustValue{};
        try {
            trustValue = std::stod(guessTrust);
        } catch (const std::exception& e) {
            throw Alphadocte::Exception("Guess trust value (" + guessTrust.substr(1) + ") cannot be parsed as a number.",
                    "Alphadocte::CLI::CacheConfig::getTopGuesses(std::string_view, unsigned int, unsigned int, std::string_view) const");
        }

        guesses.emplace_back(std::make_pair(guessName, trustValue));

        if (++i >= requestedNumberGuesses)
            break;
    }

    return guesses;
}

void CacheConfig::setTopGuesses(std::string solverName,
        unsigned int solverVersion,
        std::string templateWord,
        unsigned int requestedNumberGuesses,
        std::vector<std::pair<std::string, double>> topGuesses) {

    Section& root = m_config.getRootSection();

    Section* solverSection{nullptr};
    try {
        solverSection = &getSolverSection(solverName);
        check_solver_section(*solverSection, solverName, solverVersion,
                "Alphadocte::CLI::CacheConfig::setTopGuesses(std::string, unsigned int, std::string, unsigned int, std::vector<std::pair<std::string, double>>)");
    } catch (const Alphadocte::Exception& e) {
        // invalid solver section, recreating it

        if (solverSection) {
            // remove invalid section
            root.sections.erase(std::remove_if(
                    std::begin(root.sections),
                    std::end(root.sections),
                    [solverSection](const auto& section) { return solverSection == &section; }
            ));
        }

        root.sections.emplace_back(Section{});
        solverSection = &root.sections.back();
        solverSection->name = SECTION_SOLVER;
        solverSection->entries.emplace_back(Entry{ENTRY_SOLVER_NAME, solverName});
        solverSection->entries.emplace_back(Entry{ENTRY_SOLVER_VERSION, std::to_string(solverVersion)});
    }

    Section* guessSection{nullptr};
    try {
        guessSection = &getGuessSection(*solverSection, templateWord);

        // remove matching section if it exists
        solverSection->sections.erase(std::remove_if(
                std::begin(solverSection->sections),
                std::end(solverSection->sections),
                [guessSection](const auto& section) { return guessSection == &section; }
        ));
    } catch (const Alphadocte::Exception& e) {
        // guess section does not exist, create it anyway
    }
    solverSection->sections.emplace_back(Section{});
    guessSection = &solverSection->sections.back();

    // Fill section
    guessSection->name = SECTION_GUESS;
    guessSection->entries.emplace_back(Entry{ENTRY_GUESS_TEMPLATE, templateWord});
    guessSection->entries.emplace_back(Entry{ENTRY_GUESS_NUMBER, std::to_string(requestedNumberGuesses)});

    for (const auto& guessPair : topGuesses) {
        guessSection->entries.emplace_back(Entry{ENTRY_GUESS_GUESS, guessPair.first + ' ' + std::to_string(guessPair.second)});
    }
}

bool CacheConfig::isCacheValid() const {
    try {
        auto filepath = getDictionaryPath();
        auto timestamp = getDictionaryTimestamp();

        return timestamp == std::filesystem::last_write_time(filepath);
    } catch (...) {
        return false;
    }
}

void CacheConfig::setDictionaryPath(std::filesystem::path dictionaryPath) {
    std::error_code error;
    if (!dictionaryPath.is_absolute()) {
        throw Alphadocte::InvalidArgException("Value of " + ENTRY_FILE_PATH + " is not an absolute path.",
                "Alphadocte::CLI::CacheConfig::setDictionaryPath(std::filesystem::path)");
    }

    if (!std::filesystem::is_regular_file(dictionaryPath, error) || error) {
        throw Alphadocte::InvalidArgException("Value of " + ENTRY_FILE_PATH + " does not refer to a file.",
                "Alphadocte::CLI::CacheConfig::setDictionaryPath(std::filesystem::path)");
    }

    auto it = std::find_if(
            std::begin(m_config.getRootSection().entries),
            std::end(m_config.getRootSection().entries),
            [](const auto& entry){ return entry.name == ENTRY_FILE_PATH; });

    if (it == std::end(m_config.getRootSection().entries)) {
        // no existing entry, adding it
        m_config.getRootSection().entries.emplace_back(Entry{ENTRY_FILE_PATH, dictionaryPath.string()});
    } else {
        // replace existing entry
        it->value = dictionaryPath.string();
    }
}

void CacheConfig::setDictionaryTimestamp(std::filesystem::file_time_type dictionaryTimestamp) {
    auto it = std::find_if(
            std::begin(m_config.getRootSection().entries),
            std::end(m_config.getRootSection().entries),
            [](const auto& entry){ return entry.name == ENTRY_FILE_TIMESTAMP; });

    auto timestamp = dictionaryTimestamp.time_since_epoch().count();

    if (it == std::end(m_config.getRootSection().entries)) {
        // no existing entry, adding it
        m_config.getRootSection().entries.emplace_back(Entry{ENTRY_FILE_TIMESTAMP, std::to_string(timestamp)});
    } else {
        // replace existing entry
        it->value = std::to_string(timestamp);
    }

}

const Section& CacheConfig::getSolverSection(std::string_view solverName) const {
    auto it = std::find_if(
            std::cbegin(m_config.getRootSection().sections),
            std::cend(m_config.getRootSection().sections),
            [solverName](const auto& section){ return section.name == SECTION_SOLVER
                    && std::find_if(
                            std::cbegin(section.entries),
                            std::cend(section.entries),
                            [solverName](const auto& entry) {
                                return entry.name == ENTRY_SOLVER_NAME && entry.value == solverName;
                       }) != std::cend(section.entries);
    });


    if (it == std::cend(m_config.getRootSection().sections)) {
        throw Alphadocte::Exception("Solver section with name \"" + std::string(solverName) + "\" not found.",
                "Alphadocte::CLI::CacheConfig::getSolverSection(std::string_view) const");
    }

    return *it;
}

Section& CacheConfig::getSolverSection(std::string_view solverName) {
    // re-use code of const getter
    return const_cast<Section&>(std::as_const(*this).getSolverSection(solverName));
}

const Section& CacheConfig::getGuessSection(const Section& solverSection, std::string_view templateWord) {
    if (solverSection.name != SECTION_SOLVER) {
        throw Alphadocte::InvalidArgException("Guess section must be searched inside solver section.",
                "Alphadocte::CLI::CacheConfig::getGuessSection(const Alphadocte::CLI::Section&, std::string_view)");
    }

    auto it = std::find_if(
            std::cbegin(solverSection.sections),
            std::cend(solverSection.sections),
            [templateWord](const auto& section){ return section.name == SECTION_GUESS
                && std::find_if(
                        std::cbegin(section.entries),
                        std::cend(section.entries),
                        [templateWord](const auto& entry) {
                            return entry.name == ENTRY_GUESS_TEMPLATE && entry.value == templateWord;
                    }) != std::cend(section.entries);
    });

    if (it == std::cend(solverSection.sections)) {
        throw Alphadocte::Exception("Guess section with template \"" + std::string(templateWord) + "\" not found.",
                "Alphadocte::CLI::CacheConfig::getGuessSection(const Alphadocte::CLI::Section&, std::string_view)");
    }

    return *it;
}

Section& CacheConfig::getGuessSection(Section& solverSection, std::string_view templateWord) {
    if (solverSection.name != SECTION_SOLVER) {
        throw Alphadocte::InvalidArgException("Guess section must be searched inside solver section.",
                "Alphadocte::CLI::CacheConfig::getGuessSection(const Alphadocte::CLI::Section&, std::string_view)");
    }

    auto it = std::find_if(
            std::begin(solverSection.sections),
            std::end(solverSection.sections),
            [templateWord](const auto& section){ return section.name == SECTION_GUESS
                && std::find_if(
                        std::cbegin(section.entries),
                        std::cend(section.entries),
                        [templateWord](const auto& entry) {
                            return entry.name == ENTRY_GUESS_TEMPLATE && entry.value == templateWord;
                    }) != std::cend(section.entries);
    });

    if (it == std::end(solverSection.sections)) {
        throw Alphadocte::Exception("Guess section with template \"" + std::string(templateWord) + "\" not found.",
                "Alphadocte::CLI::CacheConfig::getGuessSection(const Alphadocte::CLI::Section&, std::string_view)");
    }

    return *it;
}

} /* namespace CLI */

} /* namespace Alphadocte */

