#include "assembler.h"
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <regex>

void Assembler::loadASM(char const* filename) {
	std::string nextLine;
	std::ifstream file(filename, std::ios::in);
	if (file.is_open()) {
		while (getline(file, nextLine)) {
			assembly.push_back(nextLine);
		}
		file.close();
	}
	else { "Could not open file " + (std::string)filename; }
}

void Assembler::toFile(char const * filename) {
	std::ofstream f(filename, std::ios::out | std::ios::binary);
	f.write((char*)&ROM[0], sizeof(ROM));
}

void Assembler::assembleROM(char const* filename) {
	int hold;

	loadASM(filename);
	std::vector<std::vector<std::string>> tokens;
	for (std::string line : assembly) {
		std::regex re("[,\t ]"); //WHY
		std::sregex_token_iterator first{line.begin(), line.end(), re, -1}, last; //split lines based on regex delimiters
		std::vector<std::string> tokenBuffer{ first, last }; //store in buffer
		std::vector<int> emptyTokenIndices; //store indices of empty tokens
		for (int i = 0; i < tokenBuffer.size(); i++) {
			tokenBuffer.at(i).erase(0, tokenBuffer.at(i).find_first_not_of(" \t\n\r\f\v")); //clean tokens
			tokenBuffer.at(i).erase(tokenBuffer.at(i).find_last_not_of(" \t\n\r\f\v") + 1); 
			if (tokenBuffer.at(i).empty()) {
				emptyTokenIndices.push_back(i); //store indices of empty tokens for later, cleaning them on the fly will affect indices which is BAD
			}
		}
		int accumulator = 0; //store how many tokens have been removed
		for (int i = 0; i < emptyTokenIndices.size(); i++) {
			tokenBuffer.erase(tokenBuffer.begin() + emptyTokenIndices.at(i)-accumulator); //remove tokens with respect to removed indices
			accumulator++;
		}

		tokens.push_back(tokenBuffer); //put tokens into token line vector
	}
}

