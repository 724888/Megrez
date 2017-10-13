/* =====================================================================
Copyright 2017 The Megrez Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
========================================================================*/

#include "megrez/basic.h"
#include "megrez/builder.h"
#include "megrez/info.h"
#include "megrez/string.h"
#include "megrez/struct.h"
#include "megrez/vector.h"
#include "megrez/util.h"
#include "compiler/idl.h"

void Error(const char *err, const char *obj = nullptr, bool usage = false);
/*
namespace megrez {

bool GenerateBinary(const Parser &parser,
					const std::string &path,
					const std::string &file_name) {
	return !parser.builder_.GetSize() ||
			megrez::SaveFile(
				(path + file_name + "_wire.bin").c_str(),
				reinterpret_cast<char *>(parser.builder_.GetBufferPointer()),
				parser.builder_.GetSize(),
				true);
}

bool GenerateTextFile(const Parser &parser,
					  const std::string &path,
					  const std::string &file_name) {
	if (!parser.builder_.GetSize()) return true;
	if (!parser.root_struct_def) Error("root_type not set");
	std::string text;
	GenerateText(parser, parser.builder_.GetBufferPointer(), 2, &text);
	return megrez::SaveFile((path + file_name + "_wire.txt").c_str(), text, false);
}

}
*/
struct Generator {
	bool (*generate)(const megrez::Parser &parser,
					 const std::string &path,
					 const std::string &file_name);
	const char *extension;
	const char *name;
	const char *help;
};

const Generator generators[] = {
	//{ megrez::GenerateBinary, "b", "binary", "Generate wire format binaries for any data definitions" },
	//{ megrez::GenerateTextFile, "t", "text", "Generate text output for any data definitions" },
	{ megrez::GenerateCPP, "c", "C++", "Generate C++ headers for infos/structs" },

};

const char *program_name = NULL;

void Error(const char *err, const char *obj, bool usage) {
	printf("%s: %s\n", program_name, err);
	if (obj) printf(": %s", obj);
	printf("\n");
	if (usage) {
		printf("Usage: %s [OPTION] [FILE]\n", program_name);
		for (size_t i = 0; i < sizeof(generators) / sizeof(generators[0]); ++i)
			printf("  -%s      %s.\n", generators[i].extension, generators[i].help);
		printf("  -o      PATH Prefix PATH to all generated files.\n"
			   "FILEs may depend on declarations in earlier files.\n"
			   "Output files are named using the base file name of the input,"
			   "and written to the current directory or the path given by -o.\n"
			   "example: %s -c -b schema1.mgz\n",
			   program_name);
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
		if (arg[0] == '-') {
			if (filenames.size()) { Error("Invalid option location", arg, true); }
			if (strlen(arg) != 2) { Error("Invalid commandline argument", arg, true); }
			switch (arg[1]) {
				case 'o':
					if (++i >= argc) { Error("Missing path following", arg, true); }
					output_path = argv[i];
					break;
				default:
					for (size_t i = 0; i < num_generators; ++i) {
						if(!strcmp(arg+1, generators[i].extension)) {
							generator_enabled[i] = true;
							any_generator = true;
							goto found;
						}
					}
					Error("Unknown commandline argument", arg, true);
					found:
					break;
			}
		} else { filenames.push_back(argv[i]); }
	}

	if (!filenames.size()) {Error("Missing input files", nullptr, true);}
	if (!any_generator) { 
		Error("No options: no output files generated.",
			  "Specify one of -c.", true); 
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

			for (size_t i = 0; i < num_generators; ++i) {
				if (generator_enabled[i]) {
					if (!generators[i].generate(parser, output_path, filebase)) {
						Error((std::string("Unable to generate ") +
							   generators[i].name + " for " +
							   filebase).c_str());
					}
				}
			}

			for (auto it = parser.enums_.vec.begin(); it != parser.enums_.vec.end(); ++it)
				{ (*it)->generated = true; }
			
			for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end(); ++it)
				{ (*it)->generated = true; }
	}

	return 0;
}