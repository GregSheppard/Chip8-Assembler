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
	f.write((char*)&ROM[0], ROM.size()*sizeof(uint8_t));
	f.close();
}

void Assembler::tokenizeLines(std::vector<std::string> lines) {
	for (std::string line : lines) {
		std::regex re("[,\t ]"); //WHY
		std::sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last; //split lines based on regex delimiters
		std::vector<std::string> tokenBuffer{ first, last }; //store in buffer
		std::vector<int> emptyTokenIndices; //store indices of empty tokens
		for (unsigned int i = 0; i < tokenBuffer.size(); i++) {
			tokenBuffer.at(i).erase(0, tokenBuffer.at(i).find_first_not_of(" \t\n\r\f\v")); //clean tokens
			tokenBuffer.at(i).erase(tokenBuffer.at(i).find_last_not_of(" \t\n\r\f\v") + 1);
			if (tokenBuffer.at(i).empty()) {
				emptyTokenIndices.push_back(i); //store indices of empty tokens for later, cleaning them on the fly will affect indices which is BAD
			}
		}
		int accumulator = 0; //store how many tokens have been removed
		for (unsigned int i = 0; i < emptyTokenIndices.size(); i++) {
			tokenBuffer.erase(tokenBuffer.begin() + emptyTokenIndices.at(i) - accumulator); //remove tokens with respect to removed indices
			accumulator++;
		}
		tokens.push_back(tokenBuffer); //put tokens into token line vector
	}
}

void Assembler::assembleROM(char const* filename) {
	loadASM(filename);
	tokenizeLines(assembly);
	std::stringstream ss;

	for (unsigned int lineNumber = 0; lineNumber < tokens.size(); lineNumber++) {
		int size = tokens.at(lineNumber).size();

		if(tokens.at(lineNumber).at(0) == "CLS") {
			if (size == 1) {
				ROM.push_back(0x00);
				ROM.push_back(0xE0);
			}
			else {
				std::cout << "Unexpected arguments on line " << lineNumber << ". CLS takes no args." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "RET") {
			if (size == 1) {
				ROM.push_back(0x00);
				ROM.push_back(0xEE);
			}
			else {
				std::cout << "Unexpected arguments on line " << lineNumber << ". RET takes no args." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "JP") {
			if (size == 2) {
				std::string num = processNum(true, tokens.at(lineNumber).at(1));
				ss << "0x1" << num[0];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << num[1] << num[2];
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else if (size == 3) {
				if (tokens.at(lineNumber).at(1) == "V0" || tokens.at(lineNumber).at(1) == "v0") {
					std::string num = processNum(true, tokens.at(lineNumber).at(2));
					ss << "0xB" << num[0];
					std::string hex1 = ss.str(); ss.str("");
					ss << "0x" << num[1] << num[2];
					std::string hex2 = ss.str(); ss.str("");
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else {
					std::cout << "Error on line " << lineNumber << ". JP can only be used with V0!" << std::endl;
				}
			}
			else {
				std::cout << "Error on line " << lineNumber << ". JP takes 1 or 2 args." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "CALL") {
			if (size == 2) {
				std::string num = processNum(true, tokens.at(lineNumber).at(1));
				ss << "0x2" << num[0];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << num[1] << num[2];
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". CALL takes 1 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SE") {
			if (size == 3) {
				if (tokens.at(lineNumber).at(2)[0] == 'V' || tokens.at(lineNumber).at(2)[0] == 'v') {
					ss << "0x5" << tokens.at(lineNumber).at(1)[1];
					std::string hex1 = ss.str(); ss.str("");
					ss << "0x" << tokens.at(lineNumber).at(2)[1] << "0";
					std::string hex2 = ss.str(); ss.str("");
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else {
					std::string num = processNum(false, tokens.at(lineNumber).at(2));
					ss << "0x3" << tokens.at(lineNumber).at(1)[1];
					std::string hex1 = ss.str(); ss.str("");
					ss << "0x" << num[0] << num[1];
					std::string hex2 = ss.str(); ss.str("");
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SE takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SNE") {
			if (size == 3) {
				if (tokens.at(lineNumber).at(2)[0] == 'V' || tokens.at(lineNumber).at(2)[0] == 'v') {
					ss << "0x9" << tokens.at(lineNumber).at(1)[1];
					std::string hex1 = ss.str(); ss.str("");
					ss << "0x" << tokens.at(lineNumber).at(2)[1] << "0";
					std::string hex2 = ss.str(); ss.str("");
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else {
					std::string num = processNum(false, tokens.at(lineNumber).at(2));
					ss << "0x4" << tokens.at(lineNumber).at(1)[1];
					std::string hex1 = ss.str(); ss.str("");
					ss << "0x" << num[0] << num[1];
					std::string hex2 = ss.str(); ss.str("");
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SNE takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "LD") {
			if (size == 3) {
				if (tokens.at(lineNumber).at(1)[0] == 'I') {
					std::string num = processNum(true, tokens.at(lineNumber).at(2));
					ss << "0xA" << num[0];
					std::string hex1 = ss.str(); ss.str("");
					ss << "0x" << num[1] << num[2];
					std::string hex2 = ss.str(); ss.str("");
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				} else if (tokens.at(lineNumber).at(1) == "DT") {
					ss << "0xF" << tokens.at(lineNumber).at(2)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x15";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else if (tokens.at(lineNumber).at(1) == "ST") {
					ss << "0xF" << tokens.at(lineNumber).at(2)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x18";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else if (tokens.at(lineNumber).at(1) == "F") {
					ss << "0xF" << tokens.at(lineNumber).at(2)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x29";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else if (tokens.at(lineNumber).at(1) == "B") {
					ss << "0xF" << tokens.at(lineNumber).at(2)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x33";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else if (tokens.at(lineNumber).at(1) == "[I]") {
					ss << "0xF" << tokens.at(lineNumber).at(2)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x55";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else if (tokens.at(lineNumber).at(2) == "DT") {
					ss << "0xF" << tokens.at(lineNumber).at(1)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x07";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else if (tokens.at(lineNumber).at(2) == "K") {
					ss << "0xF" << tokens.at(lineNumber).at(1)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x0A";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else if (tokens.at(lineNumber).at(2) == "[I]") {
					ss << "0xF" << tokens.at(lineNumber).at(1)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x65";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else {
					if (tokens.at(lineNumber).at(2)[0] == 'V' || tokens.at(lineNumber).at(2)[0] == 'v') {
						ss << "0x8" << tokens.at(lineNumber).at(1)[1];
						std::string hex1 = ss.str(); ss.str("");
						ss << "0x" << tokens.at(lineNumber).at(2)[1] << "0";
						std::string hex2 = ss.str(); ss.str("");
						ROM.push_back(std::stoi(hex1, 0, 16));
						ROM.push_back(std::stoi(hex2, 0, 16));
					}
					else {
						std::string num = processNum(false, tokens.at(lineNumber).at(2));
						ss << "0x6" << tokens.at(lineNumber).at(1)[1];
						std::string hex1 = ss.str(); ss.str("");
						ss << "0x" << num[0] << num[1];
						std::string hex2 = ss.str(); ss.str("");
						ROM.push_back(std::stoi(hex1, 0, 16));
						ROM.push_back(std::stoi(hex2, 0, 16));
					}
				}
			}
			else {
				std::cout << "Error on line " << lineNumber << ". LD takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "ADD") {
			if (size == 3) {
				if (tokens.at(lineNumber).at(1)[0] == 'I') {
					ss << "0xF" << tokens.at(lineNumber).at(2)[1];
					std::string hex1 = ss.str(); ss.str("");
					std::string hex2 = "0x1E";
					ROM.push_back(std::stoi(hex1, 0, 16));
					ROM.push_back(std::stoi(hex2, 0, 16));
				}
				else {
					if (tokens.at(lineNumber).at(2)[0] == 'V' || tokens.at(lineNumber).at(2)[0] == 'v') {
						ss << "0x8" << tokens.at(lineNumber).at(1)[1];
						std::string hex1 = ss.str(); ss.str("");
						ss << "0x" << tokens.at(lineNumber).at(2)[1] << "4";
						std::string hex2 = ss.str(); ss.str("");
						ROM.push_back(std::stoi(hex1, 0, 16));
						ROM.push_back(std::stoi(hex2, 0, 16));
					}
					else {
						std::string num = processNum(false, tokens.at(lineNumber).at(2));
						ss << "0x7" << tokens.at(lineNumber).at(1)[1];
						std::string hex1 = ss.str(); ss.str("");
						ss << "0x" << num[0] << num[1];
						std::string hex2 = ss.str(); ss.str("");
						ROM.push_back(std::stoi(hex1, 0, 16));
						ROM.push_back(std::stoi(hex2, 0, 16));
					}
				}
			}
			else {
				std::cout << "Error on line " << lineNumber << ". ADD takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "OR") {
			if (size == 3) {
				ss << "0x8" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << "1";
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". OR takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "AND") {
			if (size == 3) {
				ss << "0x8" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << "2";
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". AND takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "XOR") {
			if (size == 3) {
				ss << "0x8" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << "3";
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". XOR takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SUB") {
			if (size == 3) {
				ss << "0x8" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << "5";
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SUB takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SHR") {
			if (size == 3) {
				ss << "0x8" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << "6";
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SHR takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SUBN") {
			if (size == 3) {
				ss << "0x8" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << "7";
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SUBN takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SHL") {
			if (size == 3) {
				ss << "0x8" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << "E";
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SHL takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "RND") {
			if (size == 3) {
				std::string num = processNum(false, tokens.at(lineNumber).at(2));
				ss << "0xC" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << num[0] << num[1];
				std::string hex2 = ss.str(); ss.str("");
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". RND takes 2 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "DRW") {
			if (size == 4) {
				for (std::string token : tokens.at(lineNumber)) {
					std::cout << token << std::endl;
				}
				ss << "0xD" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				ss << "0x" << tokens.at(lineNumber).at(2)[1] << tokens.at(lineNumber).at(3)[2];
				std::string hex2 = ss.str(); ss.str("");
				std::cout << hex1 << " " << hex2 << std::endl;
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". DRW takes 3 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SKP") {
			if (size == 2) {
				ss << "0xE" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				std::string hex2 = "0x9E";
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SKP takes 1 arg." << std::endl;
				errorFlag = 1;
			}
		}
		else if (tokens.at(lineNumber).at(0) == "SKNP") {
			if (size == 2) {
				ss << "0xE" << tokens.at(lineNumber).at(1)[1];
				std::string hex1 = ss.str(); ss.str("");
				std::string hex2 = "0xA1";
				ROM.push_back(std::stoi(hex1, 0, 16));
				ROM.push_back(std::stoi(hex2, 0, 16));
			}
			else {
				std::cout << "Error on line " << lineNumber << ". SKP takes 1 arg." << std::endl;
				errorFlag = 1;
			}
		}

		/*std::cout << "Line number: " << lineNumber << std::endl;
		int hold;
		std::cin >> hold;*/
	}
}

std::string Assembler::processNum(bool pad3, std::string num) {
	int len = num.length();
	std::stringstream ss;
		if (len > 2 && num[1] == 'x') { //hex
			switch (len) {
			case 3:
				if (pad3) {
					ss << "00" << num[2];
				}
				else { ss << "0" << num[2]; }
				break;

			case 4:
				if (pad3) {
					ss << "0" << num[2] << num[3];
				}
				else { ss << num[2] << num[3]; }
				break;

			case 5:
				if (pad3) {
					ss << num[2] << num[3] << num[4];
				}
				else { std::cout << "cannot convert 3 digits to 2." << std::endl; }
				break;

			default:
				std::cout << "number too big!" << std::endl;
				break;
			}
		}
		else if (len > 0 && num[0] == '$') { //hex
			switch (len) {
			case 2:
				if (pad3) {
					ss << "00" << num[1];
				} else { ss << "0" << num[1]; }
				break;

			case 3:
				if (pad3) {
					ss << "0" << num[1] << num[2];
				}
				else { ss << num[1] << num[2]; }
				break;

			case 4:
				if (pad3) {
					ss << num[1] << num[2] << num[3];
				}
				else { std::cout << "cannot convert 3 digits to 2." << std::endl; }
				break;

			default:
				std::cout << "number too big!" << std::endl;
				break;
			}
		}
		else { //dec
			//implement later
			std::cout << "dec implemented later" << std::endl;
		}

		return ss.str();
}