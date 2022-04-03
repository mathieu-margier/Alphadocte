#!/bin/bash

USAGE="$0 <.dic> <.aff> <allowlist.txt> 
Generates the word list from the hunspell dictionnary files, using unmunch.

Only some categories of words are keeped (e.g. common names, adjectives, verbs). Notably, proper nouns, digit numbers, words with '-' or 'apostrophe' are discarded.

For the script to work, unmunch must be available from the PATH, whether it is intalled through the system's package manager (e.g. \"hunspell-tools\" on Debian/Ubuntu, \"hunspell\" on ArchLinux).
"

function log() {
    1>&2 echo "$*"
}

if [[ $# -ne 3 ]]; then
    log "Usage: $USAGE"
    exit 1
fi

DIC_FILE="$1"
AFF_FILE="$2"
ALLOW_LIST="$3"

TMP_SUB_DIC=`mktemp`
TMP_FILE_LIST=`mktemp`

log "1. Filtering words from dictionanry"
grep -f "$ALLOW_LIST" "$DIC_FILE" > "$TMP_SUB_DIC"

log "2. Generating word list"
unmunch "$TMP_SUB_DIC" "$AFF_FILE" > "$TMP_FILE_LIST" 2> /dev/null

if [[ $? -ne 0 ]]; then
    log "Error unmunch returned with non-zero exit code."
    rm "$TMP_FILE_LIST" "$TMP_SUB_DIC"
    exit 1
fi

TMP_ASCII_FILE_LIST=`mktemp`

log "3. Cleaning file list"

# Transliterate words to get rid of diacritics (e.g. é -> e), ligatures (ie œ -> oe)
iconv -t "ascii//TRANSLIT" "$TMP_FILE_LIST" -o "$TMP_ASCII_FILE_LIST"

# Remove '/' followed by flags that wasn't removed by unmunch for some flags
sed -i -e "s/\/[[:print:]]*$//g" "$TMP_ASCII_FILE_LIST"

# Remove "po:(...)" that also wasn't removed
sed -i -e "s/[[:space:]]\+po:[[:print:]]*$//g" "$TMP_ASCII_FILE_LIST"

# Keep words with only lower letters (e.g. remove words with '-', proper names with upper case, numbers)
cat "$TMP_ASCII_FILE_LIST" | grep -e "^[[:lower:]]\+$" | sort -u

log "All done !"

rm "$TMP_FILE_LIST" "$TMP_SUB_DIC" "$TMP_ASCII_FILE_LIST"


