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
 * File: SolverCLI.cpp
 */

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

#include <Alphadocte/EntropyMaximizer.h>
#include <Alphadocte/Exceptions.h>
#include <Alphadocte/FixedSizeDictionary.h>
#include <Alphadocte/MotusGameRules.h>
#include <Alphadocte/TxtDictionary.h>
#include <Alphadocte/WordleGameRules.h>

#include "Common.h"
#include "CacheConfig.h"

using namespace Alphadocte;
using namespace Alphadocte::CLI;

static const int DECIMAL_PRECISION      = 5;
static const int NUMBER_OF_GUESS        = 10;
static const int NUMBER_OF_GUESS_DIGITS = 1+floor(log10(NUMBER_OF_GUESS));

std::string askGuess(std::string_view templateWord, std::string_view defaultGuess);
std::vector<HintType> askHints(word_size n);

int main() {
    std::cout << "Bienvenue sur le mode solver de Alphadocte v" << ALPHADOCTE_VERSION_NAME;
    std::cout << " (logiciel libre sous licence GPLv3+)." << std::endl;

    if (getDataPath().empty()) {
        std::cout << "Erreur, impossible de localiser le dossier données de l'application." << std::endl;
        std::cout << "A-t-elle été installée correctement ?" << std::endl;
#ifdef ALPHADOCTE_OS_LINUX
        std::cout << "Sur Linux, ce dossier (" << APP_NAME
                  << ") doit être présent dans un des dossiers de " << XDG_DATA_ENV_VAR << std::endl;
#elif defined ALPHADOCTE_OS_WINDOWS
        std::cout << "Le dossier data doit être présent à côté de l'exécutable." << std::endl;
#endif
        return 1;
    }

    auto dictionaryPath = chooseDictionary();
    std::shared_ptr<Dictionary> dictionary = std::make_shared<TxtDictionary>(dictionaryPath);
    if (dictionary == nullptr) {
        std::cout << "Impossible de sélectionner un dictionnaire. Abandon du programme." << std::endl;
        return 1;
    }

    CacheConfig cache{dictionaryPath};
    std::string stem = dictionaryPath.stem().string();
    std::string dict_name = stem.substr(0, std::size(stem) - std::size(std::string("_wordlist")));
    std::filesystem::path configPath = getCachePath() / dict_name;
    try {
        Config config;
        config.loadFromFile(configPath);
        cache.setConfig(std::move(config));
    } catch (const Alphadocte::Exception& e) {
        std::cout << "Avertissement : impossible de charger le cache du dictionnaire. Obligation de faire les calculs de zéro." << std::endl;
        std::cout << "Raison: " << e.what() << std::endl;
        std::cout << std::endl;
    }

    RulesType rulesType = chooseRules();
    std::shared_ptr<IGameRules> rules;

    if (rulesType == RulesType::MOTUS) {
        if (!dictionary->load()) {
            std::cout << "Impossible de charger le dictionnaire. Est-il au bon endroit ?" << std::endl;
            return 1;
        }
        rules = std::make_shared<MotusGameRules>(dictionary);
    } else if (rulesType == RulesType::WORDLE) {
        std::shared_ptr<FixedSizeDictionary> wordleDict = std::make_shared<FixedSizeDictionary>(dictionary, ALPHADOCTE_WORDLE_DEFAULT_SIZE);
        if (!wordleDict->load()) {
            std::cout << "Impossible de charger le dictionnaire. Est-il au bon endroit ?" << std::endl;
            return 1;
        }
        rules = std::make_shared<WordleGameRules>(wordleDict);
    } else {
        std::cout << "Règle inconnue." << std::endl;
        return 1;
    }

    EntropyMaximizer solver{rules};
    std::string templateWord;

    if (rulesType == RulesType::MOTUS) {
        word_size wordSize = askPositiveInteger("Entrez le nombre de lettres : ");
        std::string firstLetterWord;

        while (firstLetterWord.empty()) {
            firstLetterWord = askWord("Entrez le première lettre du mot : ");

            if (std::size(firstLetterWord) != 1) {
                std::cout << "Erreur : vous devez entrer une seule lettre !" << std::endl;
                std::cout << std::endl;
                firstLetterWord.clear();
            }

            std::fill_n(std::back_inserter(templateWord), wordSize, '.');
            templateWord.at(0) = firstLetterWord.at(0);
        }
    } else if (rulesType == RulesType::WORDLE) {
        templateWord = ".....";
    } else {
        std::cout << "Règle inconnue." << std::endl;
        return 1;
    }

    solver.setTemplate(templateWord);
    bool playing{true};
    bool first{true};

    std::cout << std::setprecision(DECIMAL_PRECISION) << std::fixed;
    std::cout << std::endl;

    while (playing) {
        std::vector<std::pair<std::string,double>> guesses;

        if (first) {
            try {
                guesses = cache.getTopGuesses(
                        solver.getSolverName(),
                        solver.getSolverVersion(),
                        NUMBER_OF_GUESS,
                        solver.getTemplate()
                );
            } catch (const Exception& e) {
                std::cout << "Premier mot pas dans le cache." << std::endl;
                std::cout << "Calcul du premier mot, cela va prendre du temps..." << std::endl;
                std::cout << std::endl;
                guesses = solver.computeNextGuesses(NUMBER_OF_GUESS);

                // Save guesses in cache for next games
                cache.setTopGuesses("entropy_maximizer", 1, templateWord, NUMBER_OF_GUESS, guesses);
                cache.getConfig().writeToFile(configPath);
            }
            first = false;
        }
        else {
            std::cout << "Calcul du prochain mot à tenter." << std::endl;
            std::cout << "Encore " << std::size(solver.getPotentialSolutions()) << " solutions potentielles, soit " << solver.computeCurrentEntropy() << " bits." << std::endl;
            std::cout << "Veuillez patienter..." << std::endl;
            guesses = solver.computeNextGuesses(NUMBER_OF_GUESS);
        }
        std::cout << std::endl;

        if (guesses.empty()) {
            std::cout << "Erreur, impossible de trouver une solution potentielle." << std::endl;
            std::cout << "Le mot à trouver n'est probablement pas dans le dictionnaire du solver, "
                      << "ou les indices ont mal été rentrés." << std::endl;
            break;
        }

        std::cout << "Propositions :" << std::endl;
        for (size_t i = 0; i < std::size(guesses); i++) {
            std::cout << std::right << std::setw(NUMBER_OF_GUESS_DIGITS) << i + 1;
            std::cout << std::left << ". " << guesses[i].first;
            std::cout << " avec " << guesses[i].second << " bits" << std::endl;
        }
        std::cout << std::endl;

        std::vector<HintType> hints = askHints(std::size(templateWord));

        if (hints.empty())
            break;

        std::string guess = askGuess(templateWord, guesses[0].first);
        std::cout << "Information réellement obtenue : " << solver.computeActualEntropy(guess, hints) << std::endl;
        solver.addHint(guess, hints);

        std::cout << std::endl;
    }
}

std::string askGuess(std::string_view templateWord, std::string_view defaultGuess) {
    std::string guess;
    bool accepted{false};

    std::ostringstream prompt;
    prompt << "Entrez le mot essayé (vide pour " << defaultGuess << "): ";

    while (!accepted) {
        guess = askWord(prompt.str(), true);
        std::transform(std::begin(guess), std::end(guess), std::begin(guess), tolower);

        if (guess.empty()) {
            guess = defaultGuess;
            break;
        } else if (std::size(templateWord) != std::size(guess)) {
            std::cout << "Erreur: le mot ne fait pas le bon nombre de lettres." << std::endl;
        } else {
            // check if guess matches the template word, which have the same length
            bool matchesTemplate{true};
            for (size_t i = 0; i < std::size(guess); i++) {
                if (templateWord[i] != '.' && templateWord[i] != guess[i]) {
                    matchesTemplate = false;
                    break;
                }
            }

            if (!matchesTemplate) {
               std::cout << "Erreur: le mot ne respecte pas les lettres imposées par le jeu." << std::endl;
            } else {
                accepted = true;
            }
        }
    }

    return guess;
}

std::vector<HintType> askHints(word_size n) {
    std::vector<HintType> hintVector;

    std::cout << "Une lettre par indice, dans l'ordre donné, avec :" << std::endl;
    std::cout << "v pour lettre bien placée (vert sur Wordle, rouge sur Motus)" << std::endl;
    std::cout << "o pour lettre mal placée  (jaune sur Wordle, rond jaune sur Motus)" << std::endl;
    std::cout << "x pour lettre incorrecte  (gris sur Wordle, bleu sur Motus) " << std::endl;

    std::string hints;
    bool accepted{false};
    while (!accepted) {
        hints = askWord("Entrez les indices obtenus (vide pour quitter): ", true);
        std::transform(std::begin(hints), std::end(hints), std::begin(hints), tolower);

        if (hints.empty()) {
            return hintVector;
        } else if (std::size(hints) != n) {
            std::cout << "Erreur: pas le bon nombre d'indices (doit être " << n << ")." << std::endl;
        } else if (std::any_of(std::cbegin(hints), std::cend(hints), [](char c){ return c != 'v' && c != 'o' && c != 'x'; })) {
            std::cout << "Erreur: les indices doivent être écrits avec v, o ou x" << std::endl;
        } else {
            accepted = true;
        }
    }

    for (char c : hints) {
        hintVector.push_back(c == 'v' ? HintType::CORRECT : (c == 'o' ? HintType::MISPLACED : HintType::WRONG));
    }

    return hintVector;
}
