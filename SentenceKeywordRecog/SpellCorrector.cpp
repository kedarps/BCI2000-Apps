/*
 * SpellCorrector.cpp
 *
 * Copyright  (C)  2007  Felipe Farinon <felipe.farinon@gmail.com>
 *
 * Version: 1.4
 * Author: Felipe Farinon <felipe.farinon@gmail.com>
 * Maintainer: Felipe Farinon <felipe.farinon@gmail.com>
 * URL: http://scarvenger.wordpress.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Commentary:
 *
 * See http://scarvenger.wordpress.com/.
 *
 * Code:
 */

#include "SpellCorrector.h"
 
using namespace std;
 
bool sortBySecond(const pair<std::string, int>& left, const pair<std::string, int>& right)
{
    return left.second < right.second;
}
 
char filterNonAlphabetic(char& letter)
{
    if (isalpha(letter))
        return tolower(letter);
    return '-';
}
 
void SpellCorrector::load(const std::string& filename)
{
	// Build Hash-Table
	std::ifstream wordsFile;
	wordsFile.open(filename.c_str());

	std::string currLine;

	while(std::getline(wordsFile, currLine))
	{
		std::string thisWord = currLine.substr(0,currLine.find_first_of(" "));
		// This will find unique entries in the list of words
		dictionary[thisWord]++;
	}
	wordsFile.close();
	wordsFile.clear();
}
 
string SpellCorrector::correct(const std::string& word)
{
    Vector result;
    Dictionary candidates;
 
    if (dictionary.find(word) != dictionary.end()) { return word; }
 
    edits(word, result);
    known(result, candidates);
 
    if (candidates.size() > 0) { return max_element(candidates.begin(), candidates.end(), sortBySecond)->first; }
 
    for (unsigned int i = 0;i < result.size();i++)
    {
        Vector subResult;
 
        edits(result[i], subResult);
        known(subResult, candidates);
    }
 
    if (candidates.size() > 0) { return max_element(candidates.begin(), candidates.end(), sortBySecond)->first; }
 
    return "";
}
 
void SpellCorrector::known(Vector& results, Dictionary& candidates)
{
    Dictionary::iterator end = dictionary.end();
 
    for (unsigned int i = 0;i < results.size();i++)
    {
        Dictionary::iterator value = dictionary.find(results[i]);
 
        if (value != end) candidates[value->first] = value->second;
    }
}
 
void SpellCorrector::edits(const std::string& word, Vector& result)
{
    for (string::size_type i = 0;i < word.size();    i++) result.push_back(word.substr(0, i)             + word.substr(i + 1)); //deletions
    for (string::size_type i = 0;i < word.size() - 1;i++) result.push_back(word.substr(0, i) + word[i+1] + word.substr(i + 2)); //transposition
 
    for (char j = 'a';j <= 'z';++j)
    {
        for (string::size_type i = 0;i < word.size();    i++) result.push_back(word.substr(0, i) + j + word.substr(i + 1)); //alterations
        for (string::size_type i = 0;i < word.size() + 1;i++) result.push_back(word.substr(0, i) + j + word.substr(i)    ); //insertion
    }
}