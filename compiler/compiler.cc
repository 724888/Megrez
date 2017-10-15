
#include <cstring>
#include <iostream>
#include "compiler/idl.h"

const char *program_name = NULL;
struct Generator {
	bool (*generate)(
	  const megrez::Parser &parser,
	  const std::string &path,
	  const std::string &file_name);
	const char *ext_s;
	const char *ext_l;
	const char *name;
	const char *help;
};

const Generator generators[] = {
	{ megrez::GenerateCPP, "c", "cpp", "C++", "     Generate C++ header files;" }
};

int get_max_len() {
	int max_len, len;
	for(size_t i = 0; i< sizeof(generators)/ sizeof(generators[0]); i++) {
		len = sizeof(generators[i].ext_l) / sizeof(generators[i].ext_l[0]);
		if (max_len < len) { max_len = len; }
	}
	max_len += 10;
	return max_len;
}
void PrintHelp() {
	int max_len = get_max_len();
	for(size_t i = 0; i < sizeof(generators)/ sizeof(generators[0]); i++) {
		std::cout << "  -" << generators[i].ext_s
		   << "  --" << generators[i].ext_l;
		std::cout << generators[i].help << "\n";
	}
	std::cout << "\n"

	   << "  -o [PATH]     Prefix PATH to all generated files\n\n"

	   << "FILEs may depend on declarations in earlier files.\n"
	   << "Output files are named using the base file name of the input,\n"
	   << "and written to the current directory or the path given by -o.\n"
	   << "example: MegrezC -c schema1.mgz\n";
}
void Error(const char *err, const char *obj = nullptr, bool usage = false);
void Error(const char *err, const char *obj, bool usage) {
	printf("%s: %s\n", program_name, err);
	if (obj) printf(": %s", obj);
	printf("\n");
	if (usage) {
		printf("Usage: %s [OPTION]... FILE...\n\n", program_name);
		PrintHelp();
	}
	exit(1);
}

std::string StripExtension(const std::string &filename) {
	size_t i = filename.find_last_of(".");
	return i != std::string::npos ? filename.substr(0, i) : filename;
}

int main(int argc, const char *argv[]) {
	program_name = argv[0];
	megrez::Parser parser;
	std::string output_path;
	const size_t num_generators = sizeof(generators) / sizeof(generators[0]);
	bool generator_enabled[num_generators] = { false };
	bool any_generator = false;
	std::vector<std::string> filenames;
	for (int i = 1; i < argc; i++) {
		const char *arg = argv[i];
		if (arg[0] == '-' && arg[1] != '-') {
			if (filenames.size()) { Error("Invalid option location", arg, true); }
			if (strlen(arg) != 2) { Error("Invalid commandline argument", arg, true); }
			switch (arg[1]) {
				case 'o':
					if (++i >= argc) { Error("Missing path following", arg, true); }
					output_path = argv[i];
					break;
				default:
					for (size_t i = 0; i < num_generators; ++i) 
						if(!strcmp(arg+1, generators[i].ext_s)) {
							generator_enabled[i] = true;
							any_generator = true;
							goto found;
						}
					Error("Unknown commandline argument", arg, true);
					found:
					break;
			}

		} else if (arg[0] == '-' && arg[1] == '-') {
			std::string arg_ = arg;
			for (size_t i = 0; i < num_generators; ++i) 
				if(arg_.substr(2, arg_.length()-2) == generators[i].ext_l) {
					generator_enabled[i] = true;
					any_generator = true;
					
				} else {
					Error("Unknown commandline argument", arg, true);
				}

		} else { filenames.push_back(argv[i]); }
	}

	if (!filenames.size()) {Error("Missing input files", nullptr, true);}
	if (!any_generator) { 
		Error("No options: no output files generated.",
			  "Specify one of -c --cpp etc.", true); 
	}

	// Now process the files:
	for (auto file_it = filenames.begin();
		 file_it != filenames.end();
		 ++file_it) {
			std::string contents;
			if (!megrez::LoadFile(file_it->c_str(), true, &contents))
				{ Error("Unable to load file", file_it->c_str()); }
			if (!parser.Parse(contents.c_str()))
				{ Error(parser.error_.c_str()); }

			std::string filebase = StripExtension(*file_it);

			for (size_t i = 0; i < num_generators; ++i) 
				if (generator_enabled[i]) 
					if (!generators[i].generate(parser, output_path, filebase)) {
						Error((std::string("Unable to generate ") +
							   generators[i].name + " for " +
							   filebase).c_str());
					}

			for (auto it = parser.enums_.vec.begin(); it != parser.enums_.vec.end(); ++it)
				{ (*it)->generated = true; }
			
			for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end(); ++it)
				{ (*it)->generated = true; }
	}

	return 0;
}