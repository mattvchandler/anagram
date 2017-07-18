#!/usr/bin/env python3

# Copyright 2016 Matthew Chandler

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import argparse, sys

p = argparse.ArgumentParser(description = "Anagram generator")

p.add_argument("-p", "--show-partial", action="store_true",
        help = "Show partial anagrams. Full anagrams will be preceded by an '*'")

p.add_argument("-r", "--permutations", action="store_true",
        help = "Generate each permutation instead of each combination. Much slower, but uses much less memory")

p.add_argument("-n", "--no-apostrophe", action="store_true",
        help = "Don't generate words with apostrophes")

p.add_argument("-s", "--small-words", action="store_true",
        help = "Restrict small (<= 2 letters) words to a predefined set")

p.add_argument("-d", "--dictionary", default="/usr/share/dict/words",
        help = "Dictionary file (defaults to /usr/share/dict/words)")

p.add_argument("start", metavar = "TEXT", nargs = '+',
        help = "Text to generate anagrams for")

args = p.parse_args()

use_apostrophe = not args.no_apostrophe

seen_groups = set()

def find_words(ltrs, word_list, prefix = tuple()):
    if sum(ltrs) == 0:
        return

    new_word_list = []
    new_word_ltrs = []
    new_prefixes = []

    for word in word_list:
        word_ltrs = ltrs[:]
        for c in word:
            if c == "'":
                continue

            c = ord(c)

            if word_ltrs[c - ord('A')] == 0:
                break

            word_ltrs[c - ord('A')] -= 1
        else:
            new_prefix = prefix + (word,)

            if not args.permutations:
                new_prefix = tuple(sorted(new_prefix))

            anagram = " ".join(new_prefix)
            ltr_count = sum((len(i) for i in new_prefix))
            if use_apostrophe:
                ltr_count -= anagram.count("'")

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

start = "".join(args.start).upper().replace("'", "")

for i in start:
    if i not in [chr(ord('A') + i) for i in range(26)]:
        print("Illegal character in input:", i)
        sys.exit(1)

ltrs = [0] * 26
for l in start:
    ltrs[ord(l) - ord('A')] += 1

if args.small_words:
    legal_small_words = set(["A", "I", # , "C", "R", "U", "N")
        "AH", "AM", "AN", "AS", "AT", "BE", "BY", "DC", "DO", "DR", "EX", "GO",
        "HA", "HE", "HI", "HO", "IF", "II", "IN", "IS", "IT", "LA", "LO", "MA",
        "ME", "MR", "MS", "MY", "NO", "OF", "OH", "OK", "ON", "OR", "OW", "OX",
        "PA", "PI", "SO", "ST", "TO", "UP", "US", "WE"])

dictionary_set = set()
for word in open(args.dictionary, "r").readlines():
    word = word.strip().upper()
    skip_word = False
    for c in word:
        if (not use_apostrophe or c != "'") and (ord(c) < ord('A') or ord(c) > ord('Z')):
            skip_word = True
            break

    if not skip_word and (not args.small_words or len(word) > 2 or word in legal_small_words):
        dictionary_set.add(word)

find_words(ltrs, sorted(dictionary_set))
