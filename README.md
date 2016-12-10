# Anagram generators
An anagram generator in C, C++, and Python.

By default, it generates all combinations of words forming an anagram of the
input text. This can be changed to generate all permutations, and / or to
generate partial anagrams (where not every letter is used) by setting the
appropriate command line switches. Note that generating permutations is
considerably slower than generating combinations, but it also uses much less
memory.

For practical use, the C++ implementation is preferred, due to being the fastest
of the three. C is only slightly slower than C++ (most likely due to
std::unordered map vs Glib HashTable). Python is several times slower than both.
YMMV.

# Dependencies

* All
    * word list (default location is /usr/share/dict/words. Use -d flag to
    change)
* C++
    * C++11 compiler
    * Boost Program options (although this could be avoided by replacing it with
      the getopts_long options parser from the C implementation)
* C
    * C99 compiler
    * POSIX compliant libc (for getopts_long, strdup, getline)
        * fallback implementations of strdup and getline are provided
    * Glib 2.0
* Python
    * Python 3

# Building

A CMake CMakeLists.txt file is provided. It has options to disable building
either the C++ or C implementation. The default is to use both.

If you know what compiler options you need to link boost program options or
glib-2.0, you should be able to build without difficulty without using CMake.

For example (GCC on Debian):

    g++ -O3 -std=c++11 -o anagram anagram.cpp -lboost_program_options
    gcc -O3 -std=gnu99 -o anagram_c anagram.c $(pkg-config --cflags --libs glib-2.0)

No build step is required for the python implementation.
