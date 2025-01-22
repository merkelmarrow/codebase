// json-create-directories.cpp
#include "nlohmann/json.hpp"
#include "process.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

namespace {
	namespace nm = nlohmann;
	namespace tpl = TinyProcessLib;
}

void extract_paths(const nm::json& j, std::vector<std::string>& paths) {
	for (auto it = j.begin(); it != j.end(); it++) {
		if (it->is_object()) {
			extract_paths(*it, paths);
		}
		else {
			paths.push_back(it->get<std::string>());
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) return 1;

	std::ifstream file (argv[1]);
	if (!file.is_open()) {
		std::cerr << "Unable to open file.";
		return 1;
	}

	std::string json_content ((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	nm::json j;
	try {
		j = nm::json::parse(json_content);
	}
	catch (const nm::json::parse_error& e) {
		std::cerr << "Failed to parse json: " << e.what() << std::endl;
		return 1;
	}

	std::vector<std::string> paths;
	extract_paths(j, paths);

	for (const auto& it : paths) {
		tpl::Process create_dir(std::vector<std::string> {"cmd /C", "if not exist", "\"" + it + "\"", "mkdir", "\"" + it + "\""}, "");
	}

	std::getc;
	return 0;

}

