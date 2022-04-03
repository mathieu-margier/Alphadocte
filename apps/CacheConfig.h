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
 * File: CacheConfig.h
 */

#ifndef APPS_CACHECONFIG_H_
#define APPS_CACHECONFIG_H_

#include "Config.h"

namespace Alphadocte {

namespace CLI {

/*
 * Stores information about a dictionary in a cache.
 */
class CacheConfig {
public:
    // Constructors
    /*
     * Create new cache for one dictionary from scratch.
     *
     * Args :
     * - dictionaryPath : path to the dictionary
     *
     * Throws :
     * - Exception if dictionaryPath does not refer to a file.
     */
    CacheConfig(std::filesystem::path dictionaryPath);

    /*
     * Load existing cache from a config object.
     *
     * Args :
     * - config : the data structure holding cache information (must have been loaded,
     *            otherwise it will not be valid).
     *
     * Throws :
     * - InvalidArgException if config is invalid (ie missing required entries, outdated cache)
     */
    CacheConfig(Config config);

    // Default constructors/destructor
    virtual ~CacheConfig() = default;
    CacheConfig(const CacheConfig &other) = default;
    CacheConfig(CacheConfig &&other) = default;
    CacheConfig& operator=(const CacheConfig &other) = default;
    CacheConfig& operator=(CacheConfig &&other) = default;

    /*
     * Return the underlying configuration, which can be freely browsed.
     * It is rather recommended to use this class getters to access
     * data though.
     */
    const Config& getConfig() const;
    /*
     * If valid, change the current configuration to the given one.
     * If not, throws an exception and restore the previous config.
     *
     * Args :
     * - config : the new config to use (if valid)
     *
     * Throws :
     * - InvalidArgException : if the config is invalid (as stated by isCacheValid)
     */
    void setConfig(Config config);

    /*
     * Return the path to the dictionary's file.
     *
     * Throws :
     * - Exception : if the config does not have the dictionary path, or is invalid
     */
    std::filesystem::path getDictionaryPath() const;

    /*
     * Return the timestamp of the last write access on the dictionary's file.
     *
     * Throws :
     * - Exception : if the config does not have the dictionary timestamp, or is invalid
     */
    std::filesystem::file_time_type getDictionaryTimestamp() const;

    /*
     * Return the top guesses cached for a template as well as their trust value,
     *  using a particular solver.
     *
     * Args :
     * - soverName : unique identifier of the solver
     * - solverVersion : version of the solver, bumped each time the solver algorithm's changes might modify results
     * - requestedNumberGuesses : the number of guesses wanted.
     *                            The number of returned guesses can be lower if there are
     *                            not enough guesses match the template.
     * - templateWord : the template word that guesses must match. A dot represent any letter,
     *                  while a letter impose this letter at this position.
     *
     * Throws :
     * - Exception : if the template for this solver is not found or invalid,
     *               or if the number of requested guesses have not been cached.
     *
     */
    std::vector<std::pair<std::string, double>> getTopGuesses(
            std::string_view solverName,
            unsigned int solverVersion,
            unsigned int requestedNumberGuesses,
            std::string_view templateWord) const;

    /*
     * Set the top guesses cached for a template as well as their trust value,
     *  using a particular solver.
     *
     * If an existing invalid/outdated entry is present, it is removed and replaced
     * with the new entry for the top guesses.
     *
     * Args :
     * - soverName : unique identifier of the solver
     * - solverVersion : version of the solver, bumped each time the solver algorithm's changes might modify results
     * - requestedNumberGuesses : the number of guesses wanted.
     *                            The number of returned guesses can be lower if there are
     *                            not enough guesses match the template.
     * - templateWord : the template word that guesses must match. A dot represent any letter,
     *                  while a letter impose this letter at this position.
     * - topGuesses : the top guesses to be cached, as well as their trust values.
     *                The number of guesses can be smaller than the number of requested guesses.
     *
     */
    void setTopGuesses(std::string solverName,
            unsigned int solverVersion,
            std::string templateWord,
            unsigned int requestedNumberGuesses,
            std::vector<std::pair<std::string, double>> topGuesses);

    /*
     * Check if the given cache is valid, that is :
     * - it has the required entries (FILE_PATH and FILE_TIMESTAMP)
     * - it is not outdated
     */
    bool isCacheValid() const;


private:
    // Private methods
    /*
     * Set the dictionary path that is used for the cache.
     *
     * Throws :
     * - InvalidArgException : if dictonary path is not absolute and/or does not refer to a file.
     */
    void setDictionaryPath(std::filesystem::path dictionaryPath);

    /*
     * Set/update the dictionary last write timestamp.
     */
    void setDictionaryTimestamp(std::filesystem::file_time_type dictionaryTimestamp);

    /*
     * Return a const reference to the section solver (identified by the solver name).
     *
     * Args :
     * - solverName : identifier of the solver
     *
     * Throws :
     * - Exception : if the solver section is not found
     */
    const Section& getSolverSection(std::string_view solverName) const;

    /*
     * Return a reference to the section solver (identified by the solver name).
     *
     * Args :
     * - solverName : identifier of the solver
     *
     * Throws :
     * - Exception : if the solver section is not found
     */
    Section& getSolverSection(std::string_view solverName);

    /*
     * Return a const reference to the section guess (which use the given template)
     * inside a solver section.
     *
     * Args :
     * - solverSection : the solver section which contains the guess section to find
     * - templateWord : the template word of the guess section to find
     *
     * Throws :
     * - InvalidArgException : if solverSection is not a solver section (invalid section name)
     * - Exception : if the solver section is not found
     */
    static const Section& getGuessSection(const Section& solverSection, std::string_view templateWord);

    /*
     * Return a reference to the section guess (which use the given template)
     * inside a solver section.
     *
     * Args :
     * - solverSection : the solver section which contains the guess section to find
     * - templateWord : the template word of the guess section to find
     *
     * Throws :
     * - InvalidArgException : if solverSection is not a solver section (invalid section name)
     * - Exception : if the solver section is not found
     */
    static Section& getGuessSection(Section& solverSection, std::string_view templateWord);

    // Fields
    Config m_config;

    // Static constants
public:
    // root entries
    inline static const std::string ENTRY_FILE_PATH = "file_path";
    inline static const std::string ENTRY_FILE_TIMESTAMP = "file_timestamp";

    // solver section
    inline static const std::string SECTION_SOLVER = "solver_entry";
    inline static const std::string ENTRY_SOLVER_NAME = "solver_name";
    inline static const std::string ENTRY_SOLVER_VERSION = "solver_version";

    // guess section
    inline static const std::string SECTION_GUESS = "guess_entry";
    inline static const std::string ENTRY_GUESS_TEMPLATE = "template";
    inline static const std::string ENTRY_GUESS_NUMBER = "requested_number";
    inline static const std::string ENTRY_GUESS_GUESS = "guess";
};

} /* namespace CLI */

} /* namespace Alphadocte */

#endif /* APPS_CACHECONFIG_H_ */
