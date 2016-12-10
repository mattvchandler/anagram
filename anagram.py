#!/usr/bin/env python3

import argparse, sys

p = argparse.ArgumentParser(description = "Anagram generator")

p.add_argument("-p", "--show-partial", action="store_true",
        help = "Show partial anagrams. Full anagrams will be preceeded by an '*'")

p.add_argument("-r", "--permutations", action="store_true",
        help = "Generate each permutaton instead of each combination. Much slower, but uses much less memory")

p.add_argument("-d", "--dictionary", default="/usr/share/dict/words",
        help = "Dictionary file (defaults to /usr/share/dict/words)")

p.add_argument("start", metavar = "TEXT", nargs = '+',
        help = "Text to generate anagrams for")

args = p.parse_args()

start = "".join(args.start).upper()

for i in start:
    if i not in [chr(ord('A') + i) for i in range(26)]:
        print("Illegal character in input:", i)
        sys.exit(1)

from copy import deepcopy

seen_groups = set()

def find_words(ltrs, word_list, prefix = tuple()):
    if sum(ltrs.values()) == 0:
        return

    new_word_list = []
    new_word_ltrs = []
    new_prefixes = []

    for word in word_list:
        word_ltrs = deepcopy(ltrs)
        for l in word:
            if l not in word_ltrs:
                break

            if word_ltrs[l] == 0:
                break

            word_ltrs[l] -= 1
        else:
            new_prefix = prefix + (word,)

            if not args.permutations:
                new_prefix = tuple(sorted(new_prefix))

            anagram = " ".join(new_prefix)
            ltr_count = sum((len(i) for i in new_prefix))

            if args.permutations or anagram not in seen_groups:
                if args.show_partial:
                    if ltr_count == len(start):
                        print("*", anagram)
                    else:
                        print(" ", anagram)
                else:
                    if ltr_count == len(start):
                        print(anagram)

                new_word_ltrs.append(word_ltrs)
                new_word_list.append(word)
                new_prefixes.append(new_prefix)

                if not args.permutations:
                    seen_groups.add(anagram)

    for new_word, new_ltrs, new_prefix in zip(new_word_list, new_word_ltrs, new_prefixes):
        find_words(new_ltrs, new_word_list, new_prefix)

ltrs = {}
for l in start:
    if l not in ltrs:
        ltrs[l] = 1
    else:
        ltrs[l] += 1

def legal_word(word):
    word = word.strip().upper()
    if len(word) == 1:
        return word in ("A", "I") # , "C", "R", "U", "N")
    elif len(word) == 2:
        return word in ("AH", "AM", "AN", "AS", "AT", "BE", "BY", "DC", "DO",
                "DR", "EX", "GO", "HA", "HE", "HI", "HO", "IF", "II", "IN",
                "IS", "IT", "LA", "LO", "MA", "ME", "MR", "MS", "MY", "NO",
                "OF", "OH", "OK", "ON", "OR", "OW", "OX", "PA", "PI", "SO",
                "ST", "TO", "UP", "US", "WE")
    else:
        return True

find_words(ltrs, list(set((i.strip().upper() for i in open(args.dictionary, "r").readlines() if legal_word(i)))))
