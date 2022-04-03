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
 * File: PlayerCLI.cpp
 */

#include <algorithm>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <boost/random/mersenne_twister.hpp>

#include <Alphadocte/Alphadocte.h>
#include <Alphadocte/Exceptions.h>
#include <Alphadocte/FixedSizeDictionary.h>
#include <Alphadocte/Game.h>
#include <Alphadocte/MotusGameRules.h>
#include <Alphadocte/TxtDictionary.h>
#include <Alphadocte/WordleGameRules.h>

#include "Common.h"

using namespace Alphadocte;
using namespace Alphadocte::CLI;

static const int LETTER_DELAY_MS = 200;

void printRules(RulesType rules);


int main() {
    std::cout << "Bienvenue sur le mode jeu de Alphadocte v" << ALPHADOCTE_VERSION_NAME;
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

    std::shared_ptr<Dictionary> dictionary = std::make_shared<TxtDictionary>(chooseDictionary());
    if (dictionary == nullptr) {
        std::cout << "Impossible de sélectionner un dictionnaire. Abandon du programme." << std::endl;
        return 1;
    }
    RulesType rulesType = chooseRules();
    std::shared_ptr<IGameRules> rules;

    if (rulesType == RulesType::MOTUS) {
        if (!dictionary->load()) {
            std::cout << "Impossible de charger le dictionnaire. Est-il au bon endroit ?" << std::endl;
            std::abort();
        }
        rules = std::make_shared<MotusGameRules>(dictionary);
    } else if (rulesType == RulesType::WORDLE) {
        std::shared_ptr<FixedSizeDictionary> wordleDict = std::make_shared<FixedSizeDictionary>(dictionary, ALPHADOCTE_WORDLE_DEFAULT_SIZE);
        if (!wordleDict->load()) {
            std::cout << "Impossible de charger le dictionnaire. Est-il au bon endroit ?" << std::endl;
            std::abort();
        }
        rules = std::make_shared<WordleGameRules>(wordleDict);
    } else {
        std::abort();
    }

    printRules(rulesType);
    std::cout << std::endl;

    Game game{rules};

    boost::mt19937 random(std::time(nullptr));
    bool play{true};

    while (play) {
        // cannot use dictionary directly since rules can create a new (fixed size) dictionary
        // which does not have the same word
        game.reset();
        game.setWord(std::string(rules->getDictionary()->getRandomWord(random)));
        std::string_view answer = game.getWord();
        game.start();

        std::string guess;
        while (!game.isOver()) {
            try {
                std::cout << "Essai n°" << game.getNbGuess() + 1 << '/' << rules->getMaxGuesses() << std::endl;
                std::cout << "Mot possible : " <<  rules->getTemplate(game) << std::endl;
                guess = askWord("Entrez un mot: ");
                std::cout << std::endl;
                std::transform(std::begin(guess), std::end(guess), std::begin(guess),tolower);

                // Check if valid before actually trying it, to print more helpful error messages
                if (std::size(guess) != std::size(answer))
                    throw Exception("pas le bon nombre de lettres !", "main()");
                else if (rulesType == RulesType::MOTUS && guess.at(0) != answer.at(0)) // first letter exist as |word| = |answer] and answer != ""
                    throw Exception(std::string("le mot doit commencer par la lettre ") + answer.at(0) + " !", "main()");
                else if (!rules->getDictionary()->contains(guess))
                    throw Exception("le mot n'est pas dans le dictionnaire.", "main()");

                auto hints = game.tryGuess(guess);
                // Print previous results instantaneously
                for (unsigned int i = 0; i < game.getNbGuess() - 1; i++) {
                    printHints(game.getTriedGuesses().at(i), game.getGuessesHints().at(i));
                }

                // Print last word's hints step by step
                printHints(guess, hints, LETTER_DELAY_MS);
                std::cout << std::endl;

            } catch (const Exception& e) {
                std::cout << "Erreur: " << e.what() << std::endl;
            }
        }

        if (game.isWon()) {
            std::cout << "Bravo, vous avez trouvé le mot " << game.getWord()
                      << " en " << game.getNbGuess() << " essais !" << std::endl;
        } else {
            std::cout << "Dommage, le mot à deviner était " << game.getWord() << '.' << std::endl;
        }

        play = askConfirmation("Refaire une partie ?", 'o', 'n', false);
        std::cout << std::endl;
    }
}

void printRules(RulesType rules) {
    std::string_view guess = "mange";
    auto exemple = Game::computeHints(guess, "lampe");

    switch (rules) {
    case RulesType::MOTUS:
        std::cout << "Règles de Motus :" << std::endl;
        std::cout << "Il faut deviner le mot secret (d'une longueur arbitraire) en 6 essais ou moins." << std::endl;
        std::cout << "Chaque essai doit commencer par la première lettre du mot secret et avoir son nombre de lettres." << std::endl;
        break;
    case RulesType::WORDLE:
        std::cout << "Règle du Wordle :" << std::endl;
        std::cout << "Il faut deviner le mot secret de 5 lettres en 5 essais ou moins." << std::endl;
        std::cout << "Chaque essai doit être un mot de 5 lettres." << std::endl;
        break;
    default:
        return;
    }

    std::cout << "Chaque lettre reçoit une couleur : " << std::endl;
    std::cout << colorCorrectLetter    << "rouge" << colorReset
              << " indique que la lettre est au bon endroit." << std::endl;
    std::cout << colorWrongLocation << "jaune" << colorReset
              << " indique que la lettre est dans le mot mais pas à cet endroit." << std::endl;
    std::cout << colorWrongLetter   << "bleu"  << colorReset
              << " indique que la lettre n'est pas dans le mot." << std::endl;
    std::cout << "Il faut entrer les mots sans les accents, cédilles etc." << std::endl;
    std::cout << std::endl;
    std::cout << "Exemple: ";
    printHints(guess, exemple);
    std::cout << "On sait que le 'e' et le 'a' sont au bon endroit, (au moins) un 'm' est présent à un endroit différent, et il n'y a ni de 'n' ni de 'g'." << std::endl;
}
