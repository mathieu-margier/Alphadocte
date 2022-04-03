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
 * File: Common.h
 */

#ifndef APPS_COMMON_H_
#define APPS_COMMON_H_

/*
 * Private header use to provide various useful functions to the CLI executables.
 *
 * This is NOT a part of the library.
 */

#include <Alphadocte/Hint.h>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <Alphadocte/Alphadocte.h>
#ifdef ALPHADOCTE_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX // avoid namespace pollution (e.g. min, max) on Windows)
#endif
#endif
#include <termcolor/termcolor.hpp>

namespace Alphadocte {

// forward declarations
class Dictionary;

namespace CLI {

inline const std::string           APP_NAME{"alphadocte"};
inline const std::string           DICTIONARY_SUFIX{"_wordlist.txt"};
inline const std::filesystem::path DATA_LOCAL_DIR{"data"};
inline const word_size             ALPHADOCTE_WORDLE_DEFAULT_SIZE{5};

#if defined ALPHADOCTE_OS_LINUX
inline const std::string XDG_DATA_ENV_VAR{"XDG_DATA_DIRS"};
inline const std::string XDG_DATA_DEFAULT{ "/usr/local/share/:/usr/share/"};
inline const std::string XDG_CACHE_ENV_VAR{"XDG_CACHE_HOME"};
inline const std::string XDG_CACHE_DEFAULT{".cache"};
inline const std::string HOME_ENV_VAR{"HOME"};
#elif defined ALPHADOCTE_OS_WINDOWS
inline const std::string WIN_LOCAL_APP_ENV{"LOCALAPPDATA"};
#else
#error "Unsupported OS"
#endif

// Declarations
enum class RulesType {
    MOTUS,
    WORDLE
};

/*
 * Ask the user for a word, which can be empty if emptyAllowed is true.
 * If several words are given, ask them again to enter a word.
 *
 * Args:
 * - prompt : the string displayed to prompt for the user
 * - emptyAllowed : whether an empty word is allowed as answer
 */
std::string askWord(std::string_view ask, bool emptyAllowed = false);

/*
 * Ask for a choice between a finite number of options (from 1 to n both inclusive).
 * Do not allow for multiple answers in one line.
 *
 * Args :
 * - prompt : the string displayed to prompt for the user
 * - choicesDescription : the description of choices, which will be printed
 *
 * Return the choice, ie the index of the element chosen in the vector
 */
size_t askChoice(std::string_view prompt, const std::vector<std::string>& choicesDescription);

/*
 * Ask for a positive integer.
 * Do not allow for multiple answers in one line.
 *
 * Args :
 * - prompt : the string displayed to prompt for the user
 *
 * Return the number entered by the user.
 */
word_size askPositiveInteger(std::string_view prompt, word_size min = 0, word_size max = std::numeric_limits<word_size>::max());

/*
 * Ask the user for a confirmation (ie user must enter a character to confirm.)
 * If the answer starts with (case insensitive) the char of yes or no, the appropriate answer is chosen.
 * If no answer are given, default option is chosen.
 * Else, the confirmation is asked again.
 *
 * Args :
 * - prompt : the string displayed to prompt for the user (option yes/no will be appended to it)
 * - yes : the character meaning yes, must be lower case ('y' by default)
 * - no : the character meaning no, must be upper case ('n' by default)
 * - defaultYes : if true, yes is the default answer (when nothing or an incorrect answer
 *                is provided), else no
 */
bool askConfirmation(std::string_view prompt, char yes = 'y', char no = 'n', bool defaultYes = true);

/*
 * Tells the player to choose a dictionary.
 * French dictionary by default.
 *
 * Return a path to the chosen dictionary.
 */
std::filesystem::path chooseDictionary();

/*
 * Tells the player to choose the rules (between Motus and Wordle).
 *
 * Return the type of the rules chosen.
 */
RulesType chooseRules();

/*
 * Print the hint in a pretty way to the stdout stream (supposed to be viewed in a terminal)
 * Can make a pause between each hint if asked.
 *
 * Args :
 * - hints : the list of hint
 * - pauseTime : if > 0, the delay in ms between the print of each hint (default: no delay)
 */
void printHints(std::string_view guess, const std::vector<HintType> hints, int pauseTime = 0.);

/*
 * Return a copy of the path to the application's (read-only) data folder.
 * Use the folder 'data' if present in the working directory,
 * else check the system data folders (platform dependent).
 *
 * The path is always absolute.
 *
 * If found, keep the result in cache.
 * If application folder cannot be found, returns an empty path.
 *
 * Args :
 * - forceRefreshCache : if true, ignore the value cache and
 *                       search the data path anyway.
 */
std::filesystem::path getDataPath(bool forceRefreshCache = false);

/*
 * Return a copy of the path to the application's (read-write) cache folder.
 * Use the user's cache folder (platform dependent).
 *
 * If found, keep the result in cache. Creates the folder if it does not exist.
 * If application cache folder cannot be found, returns an empty path.
 *
 * Args :
 * - forceRefreshCache : if true, ignore the value cache and
 *                       search the data path anyway.
 *
 * Throws :
 * - std::runtime_error : either path could not be found due to missing/invalid environment variables
 *                        or the cache folder could not have been created.
 */
std::filesystem::path getCachePath(bool forceRefreshCache = false);

/*
 * Return the dictionaries found in the data directory.
 *
 * Dictionaries must be stored directly inside the data directory,
 * with '_wordlist' as filename suffix.
 * Dictionary name is the prefix of its filename, in upper case.
 * When several names conflicts each other (ie same name with different cases),
 * only one of those paths is (arbitrarily) kept.
 *
 * Map the dictionary name to the path.
 */
std::map<std::string, std::filesystem::path> getAvailableDictionaries();

// Shortcut for hint coloring in the terminal
template <typename CharT>
inline std::basic_ostream<CharT>& colorCorrectLetter(std::basic_ostream<CharT>& stream) {
    return stream << termcolor::on_red << termcolor::bright_white;
}

template <typename CharT>
inline std::basic_ostream<CharT>& colorWrongLocation(std::basic_ostream<CharT>& stream) {
    return stream << termcolor::on_bright_yellow << termcolor::grey;
}

template <typename CharT>
inline std::basic_ostream<CharT>& colorWrongLetter(std::basic_ostream<CharT>& stream) {
    return stream << termcolor::on_bright_blue << termcolor::bright_white;
}

template <typename CharT>
inline std::basic_ostream<CharT>& colorReset(std::basic_ostream<CharT>& stream) {
    return stream << termcolor::reset;
}

#ifdef ALPHADOCTE_OS_WINDOWS
// Make terminal use UTF8
class WinUtf8Terminal {
    // automatically set windows terminal to code page UTF-8 at creation
    // and restore to previous code page at destruction
public:
    WinUtf8Terminal();
    virtual ~WinUtf8Terminal();
private:
    UINT m_originalCp;
};
inline const WinUtf8Terminal _win_utf8;
#endif

} /* namespace CLI */

} /* namespace Alphadocte */

#endif /* APPS_COMMON_H_ */
