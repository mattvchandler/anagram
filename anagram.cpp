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

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;
const size_t ALPHABET_LEN = 26;

void find_words(const std::array<std::size_t, ALPHABET_LEN> & ltrs,
                const std::vector<std::string> & word_list,
                const std::vector<std::string> & prefix,
                std::unordered_set<std::string> & seen_groups,
                const bool show_partial,
                const bool permutations,
                const std::size_t start_ltr_count)
{
    if(std::accumulate(ltrs.begin(), ltrs.end(), 0) == 0)
        return;

    std::vector<std::array<std::size_t, ALPHABET_LEN>> new_ltrs;
    std::vector<std::string> new_word_list;
    std::vector<std::vector<std::string>> new_prefixes;

    for(auto & word: word_list)
    {
        std::array<std::size_t, ALPHABET_LEN> word_ltrs(ltrs);
        bool use_word = true;

        for(auto & c: word)
        {
            if(c < 'A' || c > 'Z' || word_ltrs[c - 'A'] == 0)
            {
                use_word = false;
                break;
            }

            --word_ltrs[c - 'A'];
        }
        if(use_word)
        {
            auto new_prefix = prefix;
            new_prefix.emplace_back(word);

            if(!permutations)
                std::sort(new_prefix.begin(), new_prefix.end());

            std::size_t ltr_count = 0;
            std::string anagram;
            for(const auto & word: new_prefix)
            {
                ltr_count += word.size();

                if(!anagram.empty())
                    anagram += " ";
                anagram += word;
            }

            if(permutations || !seen_groups.count(anagram))
            {
                if(show_partial)
                {
                    if(ltr_count == start_ltr_count)
                        std::cout<<"* "<<anagram<<std::endl;
                    else
                        std::cout<<"  "<<anagram<<std::endl;
                }
                else
                {
                    if(ltr_count == start_ltr_count)
                        std::cout<<anagram<<std::endl;
                }

                new_ltrs.emplace_back(word_ltrs);
                new_word_list.emplace_back(word);
                new_prefixes.emplace_back(new_prefix);

                if(!permutations)
                    seen_groups.emplace(anagram);
            }
        }
    }

    for(std::size_t i = 0; i < new_ltrs.size(); ++i)
        find_words(new_ltrs[i],
                   new_word_list,
                   new_prefixes[i],
                   seen_groups,
                   show_partial,
                   permutations,
                   start_ltr_count);
}

std::string generate_usage(char * argv[],
        const po::options_description & optional_desc,
        const po::options_description & positional_desc,
        const po::positional_options_description & pd)
{
    std::string usage = "usage:";

    std::string prog_name(argv[0]);
    auto sep_pos = prog_name.find_last_of("/");
    if(sep_pos != std::string::npos)
        prog_name = prog_name.substr(sep_pos + 1);
    usage += " " + prog_name;

    for(auto & opt: optional_desc.options())
    {
        // strip long option if both forms allowed
        std::string opt_txt = opt->format_name().substr(0, opt->format_name().find_first_of(" ["));

        if(opt->semantic()->max_tokens() > 0)
        {
            std::string format_param;
            if(!opt->format_parameter().empty())
            {
                // strip default value
                format_param = opt->format_parameter().substr(0, opt->format_parameter().find_first_of(" ("));
            }
            else
            {
                format_param = "VALUE";
            }

            // multitoken allowed
            if(opt->semantic()->max_tokens() > 1)
            {
                format_param += " [" + format_param + "...]";
            }
            // zero tokens allowd
            if(opt->semantic()->min_tokens() == 0)
            {
                format_param = "[" + format_param + "]";
            }

            opt_txt += " " + format_param;
        }

        // bracket optional params
        if(!opt->semantic()->is_required())
        {
            opt_txt = "[" + opt_txt + "]";
        }

        usage += " " + opt_txt;
    }

    // add positional args
    std::size_t num_positional = positional_desc.options().size();
    std::string cur_pos = "";
    std::size_t pos_no = 0;
    for(std::size_t i = 0; i < std::min<std::size_t>(10, pd.max_total_count()); ++i)
    {
        auto pos_name = pd.name_for_position(i);
        usage += " " + positional_desc.find(pos_name, false, false, false).format_parameter();

        if(pos_name != cur_pos)
        {
            cur_pos = pos_name;
            if(++pos_no == num_positional && pd.max_total_count() == std::numeric_limits<decltype(pd.max_total_count())>::max())
            {
                usage += "...";
                break;
            }
        }
    }

    return usage;
}

int main(int argc, char * argv[])
{
    const std::string prog_desc = "Anagram generator";
    po::options_description optional_desc("optional arguments");
    po::options_description positional_desc("positional arguments");
    po::options_description all_desc;
    po::positional_options_description pd;

    optional_desc.add_options()
        ("help,h", "Show this help message and exit")
        ("show-partial,p", "Show partial anagrams. Full anagrams will be preceeded by an '*'")
        ("permutations,r", "Generate each permutaton instead of each combination. Much slower, but uses much less memory")
        ("dictionary,d", po::value<std::string>()->default_value("/usr/share/dict/words")->value_name("DICTIONARY"),
            "Dictionary file");

    positional_desc.add_options()
        ("text", po::value<std::vector<std::string>>()->value_name("TEXT")->required(),
            "Text to generate anagrams for");

    pd.add("text", -1);

    all_desc.add(optional_desc);
    all_desc.add(positional_desc);

    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(all_desc).positional(pd).run(), vm);

        if(vm.count("help") || vm.count("h"))
        {
            std::cout<<generate_usage(argv, optional_desc, positional_desc, pd)<<"\n"<<prog_desc<<"\n\n";
            std::cout<<positional_desc<<"\n";
            std::cout<<optional_desc<<std::endl;
            return EXIT_SUCCESS;
        }

        po::notify(vm);
    }
    catch(po::error & e)
    {
        std::cerr<<e.what()<<std::endl;
        std::cerr<<generate_usage(argv, optional_desc, positional_desc, pd)<<std::endl;
        return EXIT_FAILURE;
    }

    std::string start;
    for(auto & i: vm["text"].as<std::vector<std::string>>())
        start += i;

    std::array<std::size_t, ALPHABET_LEN> ltrs;
    std::fill(ltrs.begin(), ltrs.end(), 0);
    for(auto & c: start)
    {
        c = std::toupper(c);
        if(c < 'A' || c > 'Z')
        {
            std::cerr<<"Illegal character in input: '"<<c<<"'"<<std::endl;
            return EXIT_FAILURE;
        }
        ++ltrs[c - 'A'];
    }

    std::ifstream dictionary_file(vm["dictionary"].as<std::string>());
    try
    {
        dictionary_file.exceptions(std::ifstream::failbit | std::ifstream::badbit); // throw on error OR failure
    }
    catch(std::system_error & e)
    {
        std::cerr<<"Error opening "<<vm["dictionary"].as<std::string>()<<": "<<std::strerror(errno)<<std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> dictionary;
    try
    {
        std::unordered_set<std::string> dictionary_set;
        dictionary_file.exceptions(std::ifstream::badbit); // only throw on error

        std::string word;
        while(std::getline(dictionary_file, word, '\n'))
        {
            for(auto &c: word)
                c = std::toupper(c);

            static const std::unordered_set<std::string> legal_small_words
            {
                "A", "I",
                "AH", "AM", "AN", "AS", "AT", "BE", "BY", "DC", "DO",
                "DR", "EX", "GO", "HA", "HE", "HI", "HO", "IF", "II",
                "IN", "IS", "IT", "LA", "LO", "MA", "ME", "MR", "MS",
                "MY", "NO", "OF", "OH", "OK", "ON", "OR", "OW", "OX",
                "PA", "PI", "SO", "ST", "TO",
                "UP", "US", "WE"
            };

            if(word.size() > 2 || legal_small_words.count(word))
            {
                dictionary_set.emplace(word);
            }
        }

        // put words into set to remove dupes. now put them into an array
        dictionary.insert(dictionary.end(), dictionary_set.begin(), dictionary_set.end());
    }
    catch(std::system_error & e)
    {
        std::cerr<<"Error reading "<<vm["dictionary"].as<std::string>()<<": "<<std::strerror(errno)<<std::endl;
        return EXIT_FAILURE;
    }

    std::unordered_set<std::string> seen_groups;
    find_words(ltrs, dictionary, std::vector<std::string>(), seen_groups, vm.count("show-partial") > 0, vm.count("permutations") > 0, start.size());

    return EXIT_SUCCESS;
}
