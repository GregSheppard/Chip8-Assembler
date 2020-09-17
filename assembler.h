#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Assembler {
public:
	std::vector<std::string> assembly;
	std::vector<std::vector<std::string>> tokens;
	void assembleROM(char const* filename);
	void toFile(char const* filename);
private:
	std::vector<uint8_t> ROM;
	bool errorFlag = 0;
	void loadASM(char const* filename);
	void tokenizeLines(std::vector<std::string> lines);
	std::string processNum(bool pad3, std::string num);
};