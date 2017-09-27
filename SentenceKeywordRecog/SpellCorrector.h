/*
 * SpellCorrector.h
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
 
#ifndef _SPELLCORRECTOR_H_
#define _SPELLCORRECTOR_H_
 
#include <vector>
#include <unordered_map>
 
#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>

class SpellCorrector
{
private:
    typedef std::vector<std::string> Vector;
    typedef std::unordered_map<std::string, int> Dictionary;
 
    Dictionary dictionary;
 
    void edits(const std::string& word, Vector& result);
    void known(Vector& results, Dictionary& candidates);
public:
    void load(const std::string& filename);
    std::string correct(const std::string& word);
};
 
#endif