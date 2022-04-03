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
 * File: Common.cpp
 */

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>

#include "Common.h"

#ifdef ALPHADOCTE_OS_WINDOWS
#include <windows.h>
#endif

namespace Alphadocte {

namespace CLI {

/*
 * Implement split in C++
 */
inline std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> array;
    std::istringstream iss{str};
    std::string element;

    while (iss.good()) {
        std::getline(iss, element, delimiter);
        array.emplace_back(element);
    }


    return array;
}

std::filesystem::path chooseDictionary() {
    std::string dictionary;

    auto dictionaries = getAvailableDictionaries();

    if (dictionaries.empty()) {
        std::cout << "Aucun dictionnaire trouvé, sont-ils correctement installés ?" << std::endl;
        return {};
    }

    bool frenchFound = dictionaries.find("FR") != std::cend(dictionaries);

    std::ostringstream prompt;
    prompt << "Veuillez choisir un dictionnaire parmi : ";
    bool first{true};
    for (const auto& pair : dictionaries) {
        if (!first)
            prompt << ", ";
        else
            first = false;

        prompt << pair.first;
    }
    prompt << "\nEntrer dictionnaire" << (frenchFound ? " (FR si vide)" : "") << ": ";

    bool accepted{false};
    while (!accepted) {
        dictionary = askWord(prompt.str(), frenchFound);

        // to upper case
        std::transform(std::begin(dictionary), std::end(dictionary), std::begin(dictionary),toupper);

        if (frenchFound && dictionary.empty())
            dictionary = "FR";

        if (dictionaries.find(dictionary) != std::cend(dictionaries)) {
            accepted = true;
        } else {
            std::cout << "Dictionnaire inconnu. Veuillez réessayer avec un autre dictionnaire." << std::endl;
        }
    }
    std::cout << std::endl;

    return dictionaries.at(dictionary);
}

RulesType chooseRules() {
    std::vector<std::string> choices{
        "Motus (taille du mot arbitraire, première lettre connue)",
        "Wordle (mot de taille 5)"
    };
    size_t choice = askChoice("Choisissez le mode de jeu :", choices);

    switch (choice) {
    case 0:
        // motus
        std::cout << std::endl;
        return RulesType::MOTUS;
    case 1:
        // wordle
        std::cout << std::endl;
        return RulesType::WORDLE;
    default:
        // should not happen
        std::cout << "Erreur, le choix renvoyé est en dehors des possibilités. Abandon du programme." << std::endl;
        std::abort();
    }
}

std::string askWord(std::string_view prompt, bool emptyAllowed) {
    bool accepted{false};
    std::string word;
    std::string line;
    std::istringstream iss;

    while (!accepted) {
        std::cout << prompt;

        if (!std::cin.good()) {
            std::cout << "Erreur, impossible de lire la réponse. Abandon du programme." << std::endl;
            std::abort();
        }
        std::getline(std::cin, line);
        if (!std::cin) {
            std::cout << "Erreur, lors de la lecture réponse. Abandon du programme." << std::endl;
            std::abort();
        }

        iss.clear();
        iss.str(line);
        iss >> word;

        if (iss.bad()) {
            // Don't check for fail, as we want to allow empty word in some situation,
            // which always set the fail bit.
            std::cout << "Erreur, impossible d'extraire le mot de la ligne." << std::endl;
            word.clear();
        } else if (!iss.eof()) {
            std::cout << "Pas plus d'un mot accepté!" << std::endl;
            word.clear();
        } else if (!emptyAllowed && word.empty()) {
            // Empty but not allowed
            std::cout << "Une réponse doit être donnée !" << std::endl;
            // no need to clear empty word
        } else {
            // Not empty or empty but allowed
            accepted = true;
        }
    }

    return word;
}

size_t askChoice(std::string_view prompt, const std::vector<std::string>& choicesDescription) {
    bool accepted{false};
    size_t choice;
    std::string line;
    std::istringstream iss;

    while (!accepted) {
        std::cout << prompt << std::endl;
        for (size_t i = 0; i < std::size(choicesDescription); i++) {
            std::cout << i + 1 << ". " << choicesDescription[i] << std::endl;
        }
        std::cout << "Votre choix : ";

        if (!std::cin.good()) {
            std::cout << "Erreur, impossible de lire la réponse. Abandon du programme." << std::endl;
            std::abort();
        }
        std::getline(std::cin, line);
        if (!std::cin) {
            std::cout << "Erreur, lors de la lecture réponse. Abandon du programme." << std::endl;
            std::abort();
        }

        iss.clear();
        iss.str(line);
        iss >> choice;

        if (iss.bad()) {
            std::cout << "Erreur d'entrée / sortie sur la ligne. Abandon" << std::endl;
            std::abort();
        } else if (iss.fail()) {
            std::cout << "Veuillez entrez un nombre entier !" << std::endl;
        } else if (!iss.eof()) {
            std::cout << "Une seule valeur est autorisée !" << std::endl;
        } else if (choice == 0 || choice > std::size(choicesDescription)) {
            std::cout << "Une réponse entre 1 et " << std::size(choicesDescription)
                      << " doit être donnée !" << std::endl;
        } else {
            accepted = true;
        }
    }

    return choice - 1;
}

void printHints(std::string_view guess, const std::vector<HintType> hints, int pauseTime) {
    for (word_size i = 0; i < std::size(hints); i++) {
        switch (hints.at(i)) {
        case HintType::CORRECT:
            std::cout << colorCorrectLetter << guess.at(i);
            break;
        case HintType::MISPLACED:
            std::cout << colorWrongLocation << guess.at(i);
            break;
        case HintType::WRONG:
            std::cout << colorWrongLetter << guess.at(i);
            break;
        }

        if (pauseTime > 0) {
            try {
                std::cout << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(pauseTime));
            } catch(const std::exception& e) {
                std::cerr << "Error while sleeping : " << e.what() << std::endl;
            } catch(...) {
                std::cerr << "Unknown error happened while sleeping." << std::endl;
            }
        }
    }

    std::cout << colorReset << std::endl;
}

word_size askPositiveInteger(std::string_view prompt, word_size min, word_size max) {
    bool accepted{false};
    unsigned int number;
    std::string line;
    std::istringstream iss;

    while (!accepted) {
        std::cout << prompt << std::flush;

        if (!std::cin.good()) {
            std::cout << "Erreur, impossible de lire la réponse. Abandon du programme." << std::endl;
            std::abort();
        }
        std::getline(std::cin, line);
        if (!std::cin) {
            std::cout << "Erreur, lors de la lecture réponse. Abandon du programme." << std::endl;
            std::abort();
        }

        iss.clear();
        iss.str(line);
        iss >> number;

        if (iss.bad()) {
            std::cout << "Erreur d'entrée / sortie sur la ligne. Abandon" << std::endl;
            std::abort();
        } else if (iss.fail()) {
            std::cout << "Veuillez entrez un nombre entier positif!" << std::endl;
        } else if (!iss.eof()) {
            std::cout << "Une seule valeur est autorisée !" << std::endl;
        } else if (number < min) {
            std::cout << "Le nombre doit être au moins " << min << std::endl;
        } else if (number > max) {
            std::cout << "Le nombre doit être au plus " << max << std::endl;
        } else {
            accepted = true;
        }
    }

    return number;
}

bool askConfirmation(std::string_view prompt, char yes, char no, bool defaultYes) {
    bool accepted{false};
    bool answer{defaultYes};

    std::string line;
    char choice;
    std::istringstream iss;

    while (!accepted) {
        std::cout << prompt << ' ' << '[' << static_cast<char>(defaultYes ? toupper(yes) : tolower(yes))
                  << '/' << static_cast<char>(!defaultYes ? toupper(no) : tolower(no)) << "] : " << std::flush;

        if (!std::cin.good()) {
            std::cout << "Erreur, impossible de lire la réponse. Abandon du programme." << std::endl;
            std::abort();
        }
        std::getline(std::cin, line);
        if (!std::cin) {
            std::cout << "Erreur, lors de la lecture réponse. Abandon du programme." << std::endl;
            std::abort();
        }

        iss.clear();
        iss.str(line);
        iss >> choice;
        choice = tolower(choice);

        char tmp;

        if (iss.bad()) {
            std::cout << "Erreur d'entrée / sortie sur la ligne. Abandon" << std::endl;
            std::abort();
        } else if (iss.fail()) {
            // empty choice
            // do not change answer, use default choice
            accepted = true;
        } else if (!(iss >> tmp).fail()) { // try to read another char and see if it fails
            std::cout << "Un seul caractère doit être rentré !" << std::endl;
        }
        else if (choice == yes){
            answer = true;
            accepted = true;
        } else if (choice == no) {
            answer = false;
            accepted = true;
        } else {
            std::cout << "Mauvais caractère." << std::endl;
        }
    }

    return answer;
}

std::filesystem::path getDataPath(bool forceRefreshCache) {
    static std::filesystem::path dataPath;

    if (forceRefreshCache) {
        dataPath.clear();
    }

    if (dataPath.empty()) {
        std::error_code error;

        // First check if local 'data' dir exists
        if (std::filesystem::is_directory(DATA_LOCAL_DIR, error) && !error) {
            dataPath = std::filesystem::absolute(DATA_LOCAL_DIR);
        }
        else {
            // Look for the application's data folder

#if defined ALPHADOCTE_OS_LINUX
            // Unix-like OS, use XDG specification
            std::string xdgEnvValue{ XDG_DATA_DEFAULT };
            char* envValue = std::getenv(XDG_DATA_ENV_VAR.c_str());

            if (envValue) {
                // env var is set
                xdgEnvValue = envValue;
            }

            std::vector<std::string> dirs = split(xdgEnvValue, ':');
            std::filesystem::path dirPath;
            for (const auto& dir : dirs) {
                dirPath = std::filesystem::path{ dir } / APP_NAME;

                // XDG only allows absolute paths
                if (dirPath.is_absolute() && std::filesystem::is_directory(dirPath, error) && !error) {
                    // found application  directory
                    dataPath = dirPath;
                    break;
                }
            }
#elif defined ALPHADOCTE_OS_WINDOWS
            // Windows: data folder next to the executable
            char pathBuffer[MAX_PATH];
            GetModuleFileNameA(nullptr, pathBuffer, MAX_PATH);
            dataPath = std::filesystem::absolute(std::filesystem::path(pathBuffer).parent_path() / "data");

            if (!std::filesystem::is_directory(dataPath)) {
                // missing directory
                dataPath.clear();
            }
#else
#error "Unsupported OS"
#endif
        }

    }


    return dataPath;
}

std::filesystem::path getCachePath(bool forceRefreshCache) {
    static std::filesystem::path cachePath;

    if (forceRefreshCache) {
        cachePath.clear();
    }

    if (cachePath.empty()) {

#if defined ALPHADOCTE_OS_LINUX
        // Unix-like OS, use XDG specification
        std::string xdgEnvValue;
        char *envValue = std::getenv(XDG_CACHE_ENV_VAR.c_str());
        if (envValue) {
            // env var is set
            xdgEnvValue = envValue;
            cachePath = std::filesystem::path{xdgEnvValue} / APP_NAME;
        } else {
            char *homeValue = std::getenv(HOME_ENV_VAR.c_str());

            if (homeValue) {
                cachePath = std::filesystem::path{homeValue} / XDG_CACHE_DEFAULT / APP_NAME;
            } else  {
                cachePath.clear();
                throw std::runtime_error{"Neither " + XDG_CACHE_ENV_VAR + " nor " + HOME_ENV_VAR + " are set, cannot find cache path."};
            }
        }
#elif defined ALPHADOCTE_OS_WINDOWS
        char* appdata = std::getenv(WIN_LOCAL_APP_ENV.c_str());
        if (!appdata) {
            cachePath.clear();
            throw std::runtime_error(WIN_LOCAL_APP_ENV + " environment variable must be set on Windows.");
        }

        cachePath = std::filesystem::absolute(std::filesystem::path(appdata) / APP_NAME / "cache");
#else
#error "Unsupported OS"
#endif

        if (!cachePath.is_absolute()) {
            std::string errorMsg = "Cache directory path " + cachePath.string() + " is not absolute.";
            cachePath.clear();
            throw std::runtime_error{errorMsg};
        }

        // Check if directory exists and/or can be created
        std::error_code error;
        if  (!std::filesystem::is_directory(cachePath, error) || error) {
            if (!std::filesystem::create_directories(cachePath, error) || error) {
                std::string errorMsg = "Could not create cache directory at " + cachePath.string() + ".";
                cachePath.clear();
                throw std::runtime_error{errorMsg};
            }
        }

        // Cache path is valid.
    }

    // Cache path is correct, return it

    return cachePath;
}

/*
 * Return the dictionaries found in the data directory.
 *
 * Dictionaries must be stored directly inside the data directory,
 * with '_wordlist' as filename suffix.
 * Map the prefix of the filename to the path.
 */
std::map<std::string, std::filesystem::path> getAvailableDictionaries() {
    std::map<std::string, std::filesystem::path> result;
    std::filesystem::path dataDir = getDataPath();

    if (dataDir.empty())
        return result;

    std::error_code error;
    std::string filename, dictName;
    for (const auto& entry : std::filesystem::directory_iterator{dataDir}) {
        if (entry.is_regular_file(error) && !error) {
            filename = entry.path().filename().string();

            if (filename.ends_with(DICTIONARY_SUFIX)) {
                // found dictionary,
                dictName = filename.substr(0, std::size(filename) - std::size(DICTIONARY_SUFIX));
                // to upper case
                std::transform(std::begin(dictName), std::end(dictName), std::begin(dictName),toupper);
                result.emplace(dictName, entry.path());
            }

        }
    }

    return result;
}

#ifdef ALPHADOCTE_OS_WINDOWS
WinUtf8Terminal::WinUtf8Terminal() : m_originalCp{ GetConsoleOutputCP() } {
    SetConsoleOutputCP(CP_UTF8);
}

WinUtf8Terminal::~WinUtf8Terminal() {
    SetConsoleOutputCP(m_originalCp);
}
#endif


} /* namespace CLI */

} /* namespace Alphadocte */
