#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Assembler {
public:
	std::vector<std::string> assembly;
	void assembleROM(char const* filename);
	void toFile(char const* filename);
private:
	std::vector<uint8_t> ROM;
	void loadASM(char const* filename);
};