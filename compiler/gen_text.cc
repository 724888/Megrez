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

static void GenStruct(const StructDef &struct_def, const Info *info,
					  int indent, int indent_step, std::string *_text);
template<typename T> 
void Print(T val, Type type, int indent, int indent_step,
		   StructDef * /*union_sd*/, std::string *_text) {
	std::string &text = *_text;
	text += NumToString(val);
}

template<typename T> 
void PrintVector(const Vector<T> &v, Type type, int indent, int indent_step, std::string *_text) {
	std::string &text = *_text;
	text += "[\n";
	for (uofs_t i = 0; i < v.Length(); i++) {
		if (i) text += ",\n";
		text.append(indent + indent_step, ' ');
		if (IsStruct(type))
			Print(v.GetStructFromOffset(i * type.struct_def->bytesize), type,
						indent + indent_step, indent_step, nullptr, _text);
		else
			Print(v.Get(i), type, indent + indent_step, indent_step, nullptr, _text);
	}
	text += "\n";
	text.append(indent, ' ');
	text += "]";
}

static void EscapeString(const String &s, std::string *_text) {
	std::string &text = *_text;
	text += "\"";
	for (uofs_t i = 0; i < s.Length(); i++) {
		char c = s.Get(i);
		switch (c) {
			case '\n': text += "\\n"; break;
			case '\t': text += "\\t"; break;
			case '\r': text += "\\r"; break;
			case '\"': text += "\\\""; break;
			case '\\': text += "\\\\"; break;
			default:
				if (c >= ' ' && c <= '~') {
					text += c;
				} else {
					auto u = static_cast<unsigned char>(c);
					text += "\\x" + IntToStringHex(u);
				}
				break;
		}
	}
	text += "\"";
}

template<> 
void Print<const void *>(const void *val,
						 Type type, int indent, int indent_step,
						 StructDef *union_sd, std::string *_text) {
	switch (type.base_type) {
		case BASE_TYPE_UNION:
			assert(union_sd);
			GenStruct(*union_sd,
					  reinterpret_cast<const Info *>(val),
					  indent, indent_step, _text);
			break;
		case BASE_TYPE_STRUCT:
			GenStruct(*type.struct_def,
					  reinterpret_cast<const Info *>(val),
					  indent, indent_step, _text);
			break;
		case BASE_TYPE_STRING: {
			EscapeString(*reinterpret_cast<const String *>(val), _text);
			break;
		}
		case BASE_TYPE_VECTOR:
			type = type.VectorType();
			// Call PrintVector above specifically for each element type:
			switch (type.base_type) {
				#define MEGREZ_TD(ENUM, IDLTYPE, CTYPE, JTYPE) \
					case BASE_TYPE_ ## ENUM: \
						PrintVector<CTYPE>( \
							*reinterpret_cast<const Vector<CTYPE> *>(val), \
							type, indent, indent_step, _text); break;
					MEGREZ_GEN_TYPES(MEGREZ_TD)
				#undef MEGREZ_TD
			}
			break;
		default: assert(0);
	}
}

// Generate text for a scalar field.
template<typename T> 
static void GenField(const FieldDef &fd,
					 const Info *info, bool fixed,
					 int indent_step, int indent,
					 std::string *_text) {
	Print(fixed ?
		reinterpret_cast<const Struct *>(info)->GetField<T>(fd.value.offset) :
		info->GetField<T>(fd.value.offset, 0), 
		fd.value.type, indent, indent_step,
		nullptr, _text);
}

// Generate text for non-scalar field.
static void GenFieldOffset(const FieldDef &fd, const Info *info, bool fixed,
						   int indent, int indent_step, StructDef *union_sd,
						   std::string *_text) {
	const void *val = nullptr;
	if (fixed) {
		// The only non-scalar fields in structs are structs.
		assert(IsStruct(fd.value.type));
		val = reinterpret_cast<const Struct *>(info)->
						GetStruct<const void *>(fd.value.offset);
	} else {
		val = IsStruct(fd.value.type)
			? info->GetStruct<const void *>(fd.value.offset)
			: info->GetPointer<const void *>(fd.value.offset);
	}
	Print(val, fd.value.type, indent, indent_step, union_sd, _text);
}

// Generate text for a struct or info, values separated by commas, indented,
// and bracketed by "{}"
static void GenStruct(const StructDef &struct_def, const Info *info,
					  int indent, int indent_step, std::string *_text) {
	std::string &text = *_text;
	text += "{\n";
	int fieldout = 0;
	StructDef *union_sd = nullptr;
	for (auto it = struct_def.fields.vec.begin();
		 it != struct_def.fields.vec.end();
		 ++it) {
		FieldDef &fd = **it;
		if (struct_def.fixed || info->CheckField(fd.value.offset)) {
			// The field is present.
			if (fieldout++) text += ",\n";
			text.append(indent + indent_step, ' ');
			text += fd.name;
			text += ": ";
			switch (fd.value.type.base_type) {
				 #define MEGREZ_TD(ENUM, IDLTYPE, CTYPE, JTYPE) \
					 case BASE_TYPE_ ## ENUM: \
							GenField<CTYPE>(fd, info, struct_def.fixed, \
											indent + indent_step, indent_step, _text); \
							break;
					MEGREZ_GEN_TYPES_SCALAR(MEGREZ_TD)
				#undef MEGREZ_TD
				#define MEGREZ_TD(ENUM, IDLTYPE, CTYPE, JTYPE) \
					case BASE_TYPE_ ## ENUM:
					MEGREZ_GEN_TYPES_POINTER(MEGREZ_TD)
				#undef MEGREZ_TD
						GenFieldOffset(fd, info, struct_def.fixed, indent + indent_step,
									   indent_step, union_sd, _text);
						break;
			}
			if (fd.value.type.base_type == BASE_TYPE_UTYPE) {
				union_sd = fd.value.type.enum_def->ReverseLookup(
					info->GetField<uint8_t>(fd.value.offset, 0));
			}
		}
	}
	text += "\n";
	text.append(indent, ' ');
	text += "}";
}

void GenerateText(const Parser &parser, const void *megrez, int indent_step, std::string *_text) {
	std::string &text = *_text;
	assert(parser.root_struct_def);
	text.reserve(1024); 
	GenStruct(*parser.root_struct_def, GetRoot<Info>(megrez), 0, indent_step, _text);
	text += "\n";
}

}  // namespace megrez