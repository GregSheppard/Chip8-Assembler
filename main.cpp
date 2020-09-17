#include "assembler.h"
#include <sstream>
#include <iostream>

int main() {
	Assembler assembler;
	assembler.assembleROM("asm/pong.asm");
	assembler.toFile("ROMs/pong.ch8");
	
	/*int hold;
	std::string test = "this is,	 a  test";
		std::vector<std::string> tokenBuffer;
		std::string token;
		std::istringstream iss(test);
		while (getline(iss, token, ' ')) {
			token.erase(token.find_last_not_of(" \n\r\t,")+1);
			if (!token.empty()) {
				tokenBuffer.push_back(token);
			}
		}
	for (std::string token : tokenBuffer) {
		std::cout << token << std::endl;
		std::cin >> hold;
	}*/

	/*std::string test = "   ";
	test.erase(test.find_last_not_of(" \n\r\t,") + 1); //clean tokens
	test.erase(0, test.find_first_not_of(" \n\r\t"));
	if (!test.empty()) {
		std::cout << test << std::endl;
	}
	else { std::cout << "empty" << std::endl; }
	int hold;
	std::cin >> hold;*/
	
}