# Alphadocte

## Description

Logiciel en ligne de commande permettant de jouer à des jeux de type Wordle ou Motus, et de résoudre des parties de ces jeux.
Il est disponible sur GNU/Linux (testé pour Debian 11+, Ubuntu 20.04+) et Windows.
L'interface du logiciel est en français, et il peut gérer des parties avec des mots en anglais ou en français.

## Licence

Alphadocte est un logiciel libre, publié sous la licence GPLv3 ou plus récente.
Il s'agit d'une licence dite "copyleft", le code source peut être partagé et modifié tant que les conditions de la licence sont respectées.
Voir le fichier [LICENSE.txt](LICENSE.txt) pour plus d'informations.
Sauf mention contraire, la licence s'applique à tous les fichiers.

## Données

Les listes de mots sont dans le dossier data, et générées par un Makefile.
Pour ce faire, il faut être sous un environnement Bash, et avoir l'outil `unmunch` de hunspell installé (paquet "hunspell-tools" sur Ubuntu/Debian, "hunspell" sur ArchLinux).
Dans le dossier data, il suffit d'exécuter la commande `make`.

Les listes de mots proviennent de fichiers tiers :

+ français : [fr-classique.aff](data/fr-classique.aff) et [fr-classique.dic](data/fr-classique.dic) viennent de [Grammalecte](https://grammalecte.net/download.php?prj=fr), distribués sous licence MPLv2.
* anglais : [en_raw.txt](data/en_raw.txt) vient de [https://www-personal.umich.edu/~jlawler/wordlist.html](https://www-personal.umich.edu/~jlawler/wordlist.html), distribué sous aucune garantie.

## Compilation

Alphadocte utilise CMake comme build system, et requiert un compilateur supportant le C++20 (support partiel suffisant).
Il a été testé avec GCC 9.4+ (sans les tests, qui compilent uniquement pour GCC 11+) et Visual Studio 2022 (pour les tests, besoin d'enlever les ".[name] =" dans tests/Config.cpp de CONFIG1_SECTION si toujours pas supporté).
Mac n'est pas supporté comme plateforme.

### Dépendances

Voici les bibliothèques nécessaires pour la compilation :

+ [Boost](https://www.boost.org/) : sous licence BSL-1.0, doit être installé sur le système (version header-only suffit, paquet "libboost-dev" sur Ubuntu/Debian)
+ [Termcolor](https://termcolor.readthedocs.io/) : sous licence BSD-3-Clause, téléchargé automatiquement par CMake via git
+ [Catch2](https://github.com/catchorg/Catch2) : sous licence BSL-1.0, téléchargé automatiquement par CMake via git, uniquement pour les tests

### Procédure

Exécuter CMake sur le dossier source `<src>`, pour générer un dossier build `<build>` (de préférence en dehors du dossier source).
Le dossier build permet alors de compiler, selon le générateur choisi (projet pour IDE, Makefile, Ninja, etc.).

Sur GNU/Linux, cela peut ressembler à :

```bash
cmake -S <src> -B <build>
cmake --build <build>
```

Après installation sur GNU/Linux, ne pas oublier de lancer `ldconfig` pour indexer la bibliothèque ajoutée.

Voici une liste (non exhaustive) d'options pertinentes lors de la configuration de CMake :

Paramètre                               | Valeur par défaut | Description
---                                     | ---               | ---
ALPHADOCTE\_BOOST\_USE\_CONFIG\_PACKAGE | ON                | Chercher le fichier BoostConfig.cmake fournie dans les versions récentes de Boost
BUILD\_TESTING                          | ON                | Générer les tests unitaires
CMAKE\_BUILD\_TYPE                      | Release           | Le type de compilation (Debug, Release, etc.)
CMAKE\_INSTALL\_PREFIX                  | défini par CMake  | Le préfixe du chemin utilisé pour installer le logiciel
COMPILE\_WITH\_EXTRA\_WARNING           | OFF               | Lors de la compilation, afficher plus de messages d'avertissements, et les traiter comme des erreur. Uniquement testé avec GCC.

### Docker

Le dossier `docker` contient les `Dockerfile`s utilisés pour compiler et générer les paquets à destination de distributions GNU/Linux (Debian, Ubuntu).

## Liens

Inspiré par des vidéos sur le jeu Wordle et sa "résolution" grâce à la théorie de l'information :

+ [3blue1brown](https://www.youtube.com/channel/UCYO_jab_esuFRV4b17AJtAw): [partie 1](https://www.youtube.com/watch?v=v68zYyaEmEA) et [partie 2](https://www.youtube.com/watch?v=fRed0Xmc2Wg) (EN)
+ [Science Étonnante](https://www.youtube.com/channel/UCaNlbnghtwlsGF-KzAFThqA): [vidéo](https://www.youtube.com/watch?v=iw4_7ioHWF4) (FR)
