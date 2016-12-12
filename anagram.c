// Copyright 2016 Matthew Chandler

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define __STDC_WANT_LIB_EXT2__ 1
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <getopt.h>

#include <glib.h>

#if !(__STDC_ALLOC_LIB__ || _POSIX_C_SOURCE >= 200809L)
#warning getline not found in string.h: using internal implementation
ssize_t getline(char ** lineptr, size_t * n, FILE * stream)
{
    enum {BUFFER_STEP = 128};

    if(!*lineptr)
    {
        *n = BUFFER_STEP;
        *lineptr = realloc(*lineptr, *n);
    }

    size_t i = 0;
    while(true)
    {
        char c = fgetc(stream);
        if(ferror(stream) || feof(stream))
            return -1;

        if(i + 1 == *n)
        {
            *n += BUFFER_STEP;
            *lineptr = realloc(*lineptr, *n);
        }

        (*lineptr)[i++] = c;

        if(c == '\n')
        {
            (*lineptr)[i] = '\0';
            return i;
        }
    }
}
#endif

#if !(__STDC_ALLOC_LIB__ \
    || _XOPEN_SOURCE >= 500\
    ||  _POSIX_C_SOURCE >= 200809L\
    ||  _BSD_SOURCE\
    || _SVID_SOURCE)
#warning strdup not found in string.h: using internal implementation
char * strdup(const char * str1)
{
    char * ret = malloc(sizeof(char) * (strlen(str1) + 1));
    if(!ret)
        return ret;
    strcpy(ret, str1);
    return ret;
}
#endif

void g_ptr_array_free_wrapper(void * arr)
{
    g_ptr_array_free((GPtrArray *)arr, true);
}

int strcmp_wrapper(const void * a, const void * b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

enum {ALPHABET_LEN = 26};

void find_words(const size_t ltrs[ALPHABET_LEN],
                const GPtrArray * word_list,
                const GPtrArray * prefix,
                GHashTable * seen_groups,
                const bool show_partial,
                const bool permutations,
                const size_t start_ltr_count)
{
    size_t sum_ltrs = 0;
    for(size_t i = 0; i < ALPHABET_LEN; ++i)
        sum_ltrs += ltrs[i];

    if(sum_ltrs == 0)
        return;

    GArray * new_ltrs = g_array_new(false, false, sizeof(size_t) * ALPHABET_LEN);
    GPtrArray * new_word_list = g_ptr_array_new_with_free_func(&free);
    GPtrArray * new_prefixes = g_ptr_array_new_with_free_func(&g_ptr_array_free_wrapper);

    for(size_t word_i = 0; word_i < word_list->len; ++word_i)
    {
        char * word = g_ptr_array_index(word_list, word_i);

        size_t word_ltrs[ALPHABET_LEN];
        for(size_t i = 0; i < ALPHABET_LEN; ++i)
            word_ltrs[i] = ltrs[i];

        bool use_word = true;

        for(const char * c = g_ptr_array_index(word_list, word_i); *c; ++c)
        {
            if(*c == '\'')
                continue;

            if(*c < 'A' || *c > 'Z' || word_ltrs[*c - 'A'] == 0)
            {
                use_word = false;
                break;
            }

            --word_ltrs[*c - 'A'];
        }
        if(use_word)
        {
            GPtrArray * new_prefix = g_ptr_array_sized_new(prefix->len + 1);
            g_ptr_array_set_free_func(new_prefix, &free);
            for(size_t i = 0; i < prefix->len; ++i)
            {
                g_ptr_array_add(new_prefix, strdup(g_ptr_array_index(prefix, i)));
            }

            g_ptr_array_add(new_prefix, strdup(word));

            if(!permutations)
                g_ptr_array_sort(new_prefix, &strcmp_wrapper);

            size_t ltr_count = 0;
            GString * anagram = g_string_new("");
            for(size_t i = 0; i < new_prefix->len; ++i)
            {
                const char * prefix_word = g_ptr_array_index(new_prefix, i);
                for(const char * c = prefix_word; *c; ++c)
                {
                    if(*c != '\'')
                        ++ltr_count;
                }

                if(anagram->len != 0)
                    g_string_append(anagram, " ");
                g_string_append(anagram, prefix_word);
            }

            if(permutations || !g_hash_table_lookup_extended(seen_groups, anagram->str, NULL, NULL))
            {
                if(show_partial)
                {
                    if(ltr_count == start_ltr_count)
                        printf("* %s\n", anagram->str);
                    else
                        printf("  %s\n", anagram->str);
                }
                else
                {
                    if(ltr_count == start_ltr_count)
                        puts(anagram->str);
                }

                g_array_append_val(new_ltrs, word_ltrs);
                g_ptr_array_add(new_word_list, strdup(word));
                g_ptr_array_add(new_prefixes, new_prefix);

                if(!permutations)
                    g_hash_table_insert(seen_groups, strdup(anagram->str), NULL);
            }
            else
            {
                g_ptr_array_free(new_prefix, true);
            }
            g_string_free(anagram, true);
        }
    }

    for(size_t i = 0; i < new_ltrs->len; ++i)
        find_words(&g_array_index(new_ltrs, size_t, i * ALPHABET_LEN),
                   new_word_list,
                   g_ptr_array_index(new_prefixes, i),
                   seen_groups,
                   show_partial,
                   permutations,
                   start_ltr_count);

    g_array_free(new_ltrs, true);
    g_ptr_array_free(new_word_list, true);
    g_ptr_array_free(new_prefixes, true);
}

int main(int argc, char * argv[])
{
    bool show_partial = false;
    bool permutations = false;
    char * dictionary_filename = strdup("/usr/share/dict/words");

    int opt = 0;
    extern char * optarg;
    extern int optind, optopt;

    struct option longopts[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"dictionary", required_argument, NULL, 'd'},
        {"show-partial", no_argument, NULL, 'p'},
        {"permutations", no_argument, NULL, 'r'}
    };

    char * prog_name = argv[0] + strlen(argv[0]);
    while(prog_name != argv[0] && *(prog_name - 1) != '/' && *(prog_name - 1) != '\\')
        --prog_name;

    int ind = 0;
    while((opt = getopt_long(argc, argv, ":hd:pr", longopts, &ind)) != -1)
    {
        switch(opt)
        {
        case 'd':
            free(dictionary_filename);
            dictionary_filename = strdup(optarg);
            break;
        case 'p':
            show_partial = true;
            break;
        case 'r':
            permutations = true;
            break;
        case 'h':
            printf("usage: %s [-h] [-p] [-r] [-d DICTIONARY] TEXT [TEXT ...]\n\n", prog_name);
            puts(
                "Anagram generator\n\n"
                "positional arguments:\n"
                "  TEXT                  Text to generate anagrams for\n\n"
                "optional arguments:\n"
                "  -h, --help            Show this help message and exit\n"
                "  -p, --show-partial    Show partial anagrams. Full anagrams will be preceded\n"
                "                        by an '*'\n"
                "  -r, --permutations    Generate each permutation instead of each combination\n"
                "                        Much slower, but uses much less memory\n"
                "  -d DICTIONARY, --dictionary DICTIONARY\n"
                "                        Dictionary file (defaults to /usr/share/dict/words)");
            return EXIT_SUCCESS;
        case ':':
            fprintf(stderr, "Argument required for -%c\n", (char)optopt);
            fprintf(stderr, " usage: %s [-h] [-p] [-r] [-d DICTIONARY] TEXT [TEXT ...]\n", prog_name);
            free(dictionary_filename);
            return EXIT_FAILURE;
        case '?':
        default:
            fprintf(stderr, "Unknown option: -%c\n", (char)optopt);
            fprintf(stderr, " usage: %s [-h] [-p] [-r] [-d DICTIONARY] TEXT [TEXT ...]\n", prog_name);
            free(dictionary_filename);
            return EXIT_FAILURE;
        }
    }

    // get letter counts
    size_t start_ltr_count = 0;
    size_t ltrs[ALPHABET_LEN] = {0};
    for(int i = optind; i < argc; ++i)
    {
        for(char * c = argv[i]; *c; ++c)
        {
            if(*c == '\'')
                continue;

            *c = toupper(*c);
            if(*c < 'A' || *c > 'Z')
            {
                fprintf(stderr, "Illegal character in input: '%c'\n", *c);
                free(dictionary_filename);
                return EXIT_FAILURE;
            }
            ++ltrs[*c - 'A'];
            ++start_ltr_count;
        }
    }

    // open dictionary file
    FILE * dictionary_file = fopen(dictionary_filename, "r");
    if(!dictionary_file)
    {
        fprintf(stderr, "Error opening dictionary file %s: %s\n", dictionary_filename, strerror(errno));
        free(dictionary_filename);
        return EXIT_FAILURE;
    }
    free(dictionary_filename);

    GHashTable * dictionary_set = g_hash_table_new_full(&g_str_hash, &g_str_equal, &free, NULL);

    char * word = NULL;
    size_t line_len = 0;
    while(getline(&word, &line_len, dictionary_file) >= 0)
    {
        word[strlen(word) - 1] = '\0';

        bool skip_word = false;
        for(char * c = word; *c; ++c)
        {
            *c = toupper(*c);
            if(*c < 'A' || *c > 'Z')
            {
                skip_word = true;
                break;
            }
        }

        // keep sorted so we can bsearch
        static const char * const legal_small_words[] =
        {
            "A",
            "AH", "AM", "AN", "AS", "AT", "BE", "BY", "DC", "DO", "DR",
            "EX", "GO", "HA", "HE", "HI", "HO",
            "I",
            "IF", "II", "IN", "IS", "IT", "LA", "LO", "MA", "ME", "MR",
            "MS", "MY", "NO", "OF", "OH", "OK", "ON", "OR", "OW", "OX",
            "PA", "PI", "SO", "ST", "TO", "UP", "US", "WE"
        };

        if(!skip_word && (strlen(word) > 2 || bsearch(&word, legal_small_words, sizeof(legal_small_words) / sizeof(legal_small_words[0]), sizeof(legal_small_words[0]), &strcmp_wrapper)))
            g_hash_table_insert(dictionary_set, strdup(word), NULL);
    }
    if(ferror(dictionary_file))
    {
        fprintf(stderr, "Error reading dictionary file %s: %s\n", dictionary_filename, strerror(errno));
        free(dictionary_filename);
        free(word);
        fclose(dictionary_file);
        g_hash_table_destroy(dictionary_set);
        return EXIT_FAILURE;
    }

    free(word);
    fclose(dictionary_file);

    GPtrArray * dictionary = g_ptr_array_sized_new(g_hash_table_size(dictionary_set));
    g_ptr_array_set_free_func(dictionary, &free);
    {
        GHashTableIter i;
        gpointer word;
        g_hash_table_iter_init(&i, dictionary_set);
        while(g_hash_table_iter_next(&i, &word, NULL))
        {
            g_ptr_array_add(dictionary, word);
        }
    }

    g_ptr_array_sort(dictionary, &strcmp_wrapper);

    // steal, b/c all strings now owned by GPtrArray dictionary
    g_hash_table_steal_all(dictionary_set);
    g_hash_table_destroy(dictionary_set);

    GHashTable * seen_groups = g_hash_table_new_full(&g_str_hash, &g_str_equal, &free, NULL);
    GPtrArray * prefix = g_ptr_array_new_with_free_func(&free);
    find_words(ltrs, dictionary, prefix, seen_groups, show_partial, permutations, start_ltr_count);

    g_hash_table_destroy(seen_groups);
    g_ptr_array_free(prefix, true);
    g_ptr_array_free(dictionary, true);
    return EXIT_SUCCESS;
}
