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

namespace megrez {
namespace cpp {

static std::string GenTypeBasic(const Type &type) {
	static const char *ctypename[] = {
		#define MEGREZ_TD(ENUM, IDLTYPE, CTYPE) #CTYPE,
			MEGREZ_GEN_TYPES(MEGREZ_TD)
		#undef MEGREZ_TD
	};
	return ctypename[type.base_type];
}

static std::string GenTypeWire(const Type &type, const char *postfix);

static std::string GenTypePointer(const Type &type) {
	switch (type.base_type) {
		case BASE_TYPE_STRING:
			return "megrez::String";
		case BASE_TYPE_VECTOR:
			return "megrez::Vector<" + GenTypeWire(type.VectorType(), "") + ">";
		case BASE_TYPE_STRUCT:
			return type.struct_def->name;
		case BASE_TYPE_UNION:
		default:
			return "void";
	}
}
static std::string GenTypeWire(const Type &type, const char *postfix) {
	return IsScalar(type.base_type)
		? GenTypeBasic(type) + postfix
		: IsStruct(type)
			? "const " + GenTypePointer(type) + " *"
			: "megrez::Offset<" + GenTypePointer(type) + ">" + postfix;
}
static std::string GenTypeGet(const Type &type, const char *afterbasic,
							  const char *beforeptr, const char *afterptr) {
	return IsScalar(type.base_type)
		? GenTypeBasic(type) + afterbasic
		: beforeptr + GenTypePointer(type) + afterptr;
}

static void GenComment(const std::string &dc,
					   std::string *code_ptr,
					   const char *prefix = "") {
	std::string &code = *code_ptr;
	if (dc.length()) {
		code += std::string(prefix) + "///" + dc + "\n";
	}
}
static void GenEnum(EnumDef &enum_def, std::string *code_ptr) {
	if (enum_def.generated) return;
	std::string &code = *code_ptr;
	GenComment(enum_def.doc_comment, code_ptr);
	code += "enum {\n";
	for (auto it = enum_def.vals.vec.begin();
			 it != enum_def.vals.vec.end();
			 ++it) {
		auto &ev = **it;
		GenComment(ev.doc_comment, code_ptr, "\t");
		code += "\t" + enum_def.name + "_" + ev.name + " = ";
		code += NumToString(ev.value) + ",\n";
	}
	code += "};\n\n";
	int range = enum_def.vals.vec.back()->value - enum_def.vals.vec.front()->value + 1;
	static const int kMaxSparseness = 5;
	if (range / static_cast<int>(enum_def.vals.vec.size()) < kMaxSparseness) {
		code += "inline const char **EnumNames" + enum_def.name + "() {\n";
		code += "\tstatic const char *names[] = { ";
		int val = enum_def.vals.vec.front()->value;
		for (auto it = enum_def.vals.vec.begin();
				 it != enum_def.vals.vec.end();
				 ++it) {
			while (val++ != (*it)->value) code += "\"\", ";
			code += "\"" + (*it)->name + "\", ";
		}
		code += "nullptr };\n\treturn names;\n}\n\n";
		code += "inline const char *EnumName" + enum_def.name;
		code += "(int e) { return EnumNames" + enum_def.name + "()[e";
		if (enum_def.vals.vec.front()->value)
			code += " - " + enum_def.name + "_" + enum_def.vals.vec.front()->name;
		code += "]; }\n\n";
	}
}
static void GenInfo(StructDef &struct_def, std::string *code_ptr) {
	if (struct_def.generated) return;
	std::string &code = *code_ptr;
	GenComment(struct_def.doc_comment, code_ptr);
	code += "class " + struct_def.name + " : private megrez::Info";
	code += " {\n public:\n";
	for (auto it = struct_def.fields.vec.begin();
			 it != struct_def.fields.vec.end();
			 ++it) {
		auto &field = **it;
		if (!field.deprecated) {  // Deprecated fields won't be accessible.
			GenComment(field.doc_comment, code_ptr, "\t");
			code += "\t" + GenTypeGet(field.value.type, " ", "const ", " *");
			code += field.name + "() const { return ";
			// Call a different accessor for pointers, that indirects.
			code += IsScalar(field.value.type.base_type)
				? "GetField<"
				: (IsStruct(field.value.type) ? "GetStruct<" : "GetPointer<");
			code += GenTypeGet(field.value.type, "", "const ", " *") + ">(";
			code += NumToString(field.value.offset);
			// Default value as second arg for non-pointer types.
			if (IsScalar(field.value.type.base_type))
				code += ", " + field.value.constant;
			code += "); }\n";
		}
	}
	code += "};\n\n";
	code += "struct " + struct_def.name;
	code += "Builder {\n\tmegrez::MegrezBuilder &mb_;\n";
	code += "\tmegrez::uofs_t start_;\n";
	for (auto it = struct_def.fields.vec.begin();
		 it != struct_def.fields.vec.end();
		 ++it) {
		auto &field = **it;
		if (!field.deprecated) {
			code += "\tvoid add_" + field.name + "(";
			code += GenTypeWire(field.value.type, " ") + field.name + ") { mb_.Add";
			if (IsScalar(field.value.type.base_type))
				code += "Element<" + GenTypeWire(field.value.type, "") + ">";
			else if (IsStruct(field.value.type))
				code += "Struct";
			else
				code += "Offset";
			code += "(" + NumToString(field.value.offset) + ", " + field.name;
			if (IsScalar(field.value.type.base_type))
				code += ", " + field.value.constant;
			code += "); }\n";
		}
	}
	code += "\t" + struct_def.name;
	code += "Builder(megrez::MegrezBuilder &_mb) : mb_(_mb) ";
	code += "{ start_ = mb_.StartInfo(); }\n";
	code += "\tmegrez::Offset<" + struct_def.name;
	code += "> Finish() { return megrez::Offset<" + struct_def.name;
	code += ">(mb_.EndInfo(start_, ";
	code += NumToString(struct_def.fields.vec.size()) + ")); }\n};\n\n";
	code += "inline megrez::Offset<" + struct_def.name + "> Create";
	code += struct_def.name;
	code += "(\n\t  megrez::MegrezBuilder &_mb";
	for (auto it = struct_def.fields.vec.begin();
			 it != struct_def.fields.vec.end();
			 ++it) {
		auto &field = **it;
		if (!field.deprecated) {
			code += ",\n\t  " + GenTypeWire(field.value.type, " ") + field.name;
		}
	}
	code += ") {\n\n\t" + struct_def.name + "Builder builder_(_mb);\n";
	for (size_t size = struct_def.sortbysize ? sizeof(max_scalar_t) : 1;
			 size;
			 size /= 2) {
		for (auto it = struct_def.fields.vec.rbegin();
				 it != struct_def.fields.vec.rend();
				 ++it) {
			auto &field = **it;
			if (!field.deprecated &&
					(!struct_def.sortbysize ||
					 size == SizeOf(field.value.type.base_type))) {
				code += "\tbuilder_.add_" + field.name + "(" + field.name + ");\n";
			}
		}
	}
	code += "\treturn builder_.Finish();\n}\n\n";
}

// Generate an accessor struct with constructor for a megrez struct.
static void GenStruct(StructDef &struct_def, std::string *code_ptr) {
	if (struct_def.generated) return;
	std::string &code = *code_ptr;
	GenComment(struct_def.doc_comment, code_ptr);
	code += "MANUALLY_ALIGNED_STRUCT(" + NumToString(struct_def.minalign) + ") ";
	code += struct_def.name + " {\n private:\n";
	int padding_id = 0;
	for (auto it = struct_def.fields.vec.begin();
			 it != struct_def.fields.vec.end();
			 ++it) {
		auto &field = **it;
		code += "\t" + GenTypeGet(field.value.type, " ", "", " ");
		code += field.name + "_;\n";
		if (field.padding) {
			for (int i = 0; i < 4; i++)
				if (static_cast<int>(field.padding) & (1 << i))
					code += "\tint" + NumToString((1 << i) * 8) +
									"_t __padding" + NumToString(padding_id++) + ";\n";
			assert(!(field.padding & ~0xF));
		}
	}
	code += "\n public:\n\t" + struct_def.name + "(";
	for (auto it = struct_def.fields.vec.begin();
			 it != struct_def.fields.vec.end();
			 ++it) {
		auto &field = **it;
		if (it != struct_def.fields.vec.begin()) code += ", ";
		code += GenTypeGet(field.value.type, " ", "const ", " &") + field.name;
	}
	code += ")\n\t\t: ";
	padding_id = 0;
	for (auto it = struct_def.fields.vec.begin();
			 it != struct_def.fields.vec.end();
			 ++it) {
		auto &field = **it;
		if (it != struct_def.fields.vec.begin()) code += ", ";
		code += field.name + "_(";
		if (IsScalar(field.value.type.base_type))
			code += "megrez::EndianScalar(" + field.name + "))";
		else
			code += field.name + ")";
		if (field.padding)
			code += ", __padding" + NumToString(padding_id++) + "(0)";
	}
	code += " {}\n\n";
	for (auto it = struct_def.fields.vec.begin();
			 it != struct_def.fields.vec.end();
			 ++it) {
		auto &field = **it;
		GenComment(field.doc_comment, code_ptr, "\t");
		code += "\t" + GenTypeGet(field.value.type, " ", "const ", " &");
		code += field.name + "() const { return ";
		if (IsScalar(field.value.type.base_type))
			code += "megrez::EndianScalar(" + field.name + "_)";
		else
			code += field.name + "_";
		code += "; }\n";
	}
	code += "};\nSTRUCT_END(" + struct_def.name + ", ";
	code += NumToString(struct_def.bytesize) + ");\n\n";
}

}  // namespace cpp

std::string GenerateCPP(const Parser &parser) {
	using namespace cpp;
	std::string enum_code;
	for (auto it = parser.enums_.vec.begin();
			 it != parser.enums_.vec.end(); ++it) {
		GenEnum(**it, &enum_code);
	}
	std::string forward_decl_code;
	for (auto it = parser.structs_.vec.begin();
			 it != parser.structs_.vec.end(); ++it) {
		if (!(*it)->generated)
			forward_decl_code += "struct " + (*it)->name + ";\n";
	}
	std::string decl_code;
	for (auto it = parser.structs_.vec.begin();
			 it != parser.structs_.vec.end(); ++it) {
		if ((**it).fixed) GenStruct(**it, &decl_code);
	}
	for (auto it = parser.structs_.vec.begin();
			 it != parser.structs_.vec.end(); ++it) {
		if (!(**it).fixed) GenInfo(**it, &decl_code);
	}
	if (enum_code.length() || forward_decl_code.length() || decl_code.length()) {
		std::string code;
		code = "// Automatically generated by MegrezCompiler, DO NOT MODIFY!\n\n";
		code += "#include <megrez/basic.h>\n";
		code += "#include <megrez/builder.h>\n";
		code += "#include <megrez/info.h>\n";
		code += "#include <megrez/string.h>\n";
		code += "#include <megrez/struct.h>\n";
		code += "#include <megrez/vector.h>\n\n";

		for (auto it = parser.name_space_.begin();
				 it != parser.name_space_.end(); ++it) {
			code += "namespace " + *it + " {\n";
		}
		code += "\n";
		code += enum_code;
		code += forward_decl_code;
		code += "\n";
		code += decl_code;
		if (parser.root_struct_def) {
			code += "inline const " + parser.root_struct_def->name + " *Get";
			code += parser.root_struct_def->name;
			code += "(const void *buf) { return megrez::GetRoot<";
			code += parser.root_struct_def->name + ">(buf); }\n\n";
		}
		for (auto it = parser.name_space_.begin();
				 it != parser.name_space_.end(); ++it) {
			code += "}; // namespace " + *it + "\n";
		}

		return code;
	}

	return std::string();
}

bool GenerateCPP(const Parser &parser, const std::string &path, const std::string &file_name) {
	auto code = GenerateCPP(parser);
	return !code.length() ||
			SaveFile((path + file_name + ".mgz.h").c_str(), code, false);
}

}  // namespace megrez