// $Id$

/*
 * CryptoTE PasswordGenerator v0.5.390
 * Copyright (C) 2008-2009 Timo Bingmann
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __FIPS181_H__
#define __FIPS181_H__

#include <string>

/**
 * FIPS181 pronounceable random password generator class based on FIPS-181.
 * This class is a C++ wrapper implementation of the Automated Password
 * Generator standard. Most code is from Appendix A of FIPS Publication
 * 181. However the DES random "generator" is not included and an arbitrary
 * random generator can be used instead. The class returns the passwords in
 * std::string object.
 */

class FIPS181
{
public:

    /// Typedef of the random integer generation function 
    typedef unsigned int	(*randfunc_type)();

    /// Construct a new password generator using given random function.
    explicit FIPS181(unsigned int (*randfunc)());

    /// Randomword will generate a random word and place it in the buffer word.
    int 	randomword(std::string& word, std::string& hyphenated_word, unsigned int minlen, unsigned int maxlen);

private:

    /// Hold information about the syllable units
    struct unit
    {
	char	unit_code[5];
	int	flags;
    };

    static const unsigned int NUM_RULES = 34;
    static const unsigned int MAX_UNACCEPTABLE = 20;

    /// Basic phoneme set
    static struct unit	rules[NUM_RULES];

    /// Two-dimensional matrix holding information about which phoneme units
    /// may be combined.
    static unsigned int	digram[NUM_RULES][NUM_RULES];

    enum {
	NO_SPECIAL_RULE = 0,
	ALTERNATE_VOWEL = 1,
	VOWEL = 2,
	NO_FINAL_SPLIT = 4,
	NOT_BEGIN_SYLLABLE = 8
    };

    enum {
	ANY_COMBINATION = 0,
	NOT_END = 1,
	END = 2,
	SUFFIX = 4,
	ILLEGAL_PAIR = 8,
	PREFIX = 16,
	BREAK = 32,
	NOT_BEGIN = 64,
	BEGIN = 128
    };

protected:

    /// Used within get_syllable(). Saves last working combination.
    unsigned int   saved_unit;

    /// Used within get_syllable(). Saves last working combination.
    unsigned int   saved_pair[2];

    /// This is the routine that returns a random word.
    int		get_word(std::string& word, std::string& hyphenated_word, unsigned int pwlen);

    /// Check that the word does not contain illegal combinations that may span syllables.
    bool	improper_word(unsigned int* units, unsigned int word_size);

    /// Treating y as a vowel is sometimes a problem.
    bool	have_initial_y(unsigned int* units, unsigned int unit_size);

    /// Besides the problem with the letter y, there is one with a silent e at the end of words, like face or nice.
    bool	have_final_split(unsigned int* units, unsigned int unit_size);

    /// Generate next unit to password.
    void	get_syllable(std::string& syllable, unsigned int pwlen, unsigned int* units_in_syllable, unsigned int* syllable_length);

    /// This routine goes through an individual syllable and checks for illegal combinations of letters that go beyond looking at digrams.
    bool	illegal_placement(unsigned int* units, unsigned int pwlen);

    /// Select a unit (a letter or a consonant group).
    unsigned int random_unit(unsigned int type);
    
    /// Random number generation function
    randfunc_type	randfunc_;

    /// Return a uniformly distributed random number between minlen and maxlen inclusive.
    unsigned int get_random(unsigned int minlen, unsigned int maxlen);
};

#endif // __FIPS181_H__
