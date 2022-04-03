all: fr_wordlist.txt en_wordlist.txt

fr_wordlist.txt: fr-classique.dic fr-classique.aff fr_po_allowlist.txt
	./generate_list.sh $^ > $@

en_wordlist.txt: en_raw.txt
	dos2unix -f -n "$<" tmp_unix.txt # a binary char is in the raw file
	grep -e "^[[:lower:]]\+$$" tmp_unix.txt > $@
	rm tmp_unix.txt

.PHONY: clean
clean:
	rm -f fr_wordlist.txt
