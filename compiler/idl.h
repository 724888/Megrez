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

#ifndef MEGREZ_IDL_H_
#define MEGREZ_IDL_H_

#include <map>
#include <memory>
#include <vector>
#include <string.h>
#include <assert.h>

#include "megrez/basic.h"
#include "megrez/builder.h"
#include "megrez/info.h"
#include "megrez/string.h"
#include "megrez/struct.h"
#include "megrez/vector.h"
#include "megrez/util.h"

namespace megrez {

#define MEGREZ_GEN_TYPES_SCALAR(TD) \
	TD(NONE,   "",       uint8_t)   \
	TD(UTYPE,  "",       uint8_t)   \
	TD(BOOL,   "bool",   uint8_t)   \
	TD(CHAR,   "byte",   int8_t)    \
	TD(UCHAR,  "ubyte",  uint8_t)   \
	TD(SHORT,  "short",  int16_t)   \
	TD(USHORT, "ushort", uint16_t)  \
	TD(INT,    "int",    int32_t)   \
	TD(UINT,   "uint",   uint32_t)  \
	TD(LONG,   "long",   int64_t)   \
	TD(ULONG,  "ulong",  uint64_t)  \
	TD(FLOAT,  "float",  float)     \
	TD(DOUBLE, "double", double)
#define MEGREZ_GEN_TYPES_POINTER(TD) \
	TD(STRING, "string", Offset<void>/*, int*/) \
	TD(VECTOR, "",       Offset<void>/*, int*/) \
	TD(STRUCT, "",       Offset<void>/*, int*/) \
	TD(UNION,  "",       Offset<void>/*, int*/)
#define MEGREZ_GEN_TYPES(TD) \
		MEGREZ_GEN_TYPES_SCALAR(TD) \
		MEGREZ_GEN_TYPES_POINTER(TD)
enum BaseType {
	#define MEGREZ_TD(ENUM, IDLTYPE, CTYPE) BASE_TYPE_ ## ENUM,
		MEGREZ_GEN_TYPES(MEGREZ_TD)
	#undef MEGREZ_TD
};
#define MEGREZ_TD(ENUM, IDLTYPE, CTYPE) \
		static_assert(sizeof(CTYPE) <= sizeof(max_scalar_t), \
									"define max_scalar_t as " #CTYPE);
	MEGREZ_GEN_TYPES(MEGREZ_TD)
#undef MEGREZ_TD

inline bool IsScalar (BaseType t) { return t >= BASE_TYPE_UTYPE &&	t <= BASE_TYPE_DOUBLE; }
inline bool IsInteger(BaseType t) { return t >= BASE_TYPE_UTYPE &&	t <= BASE_TYPE_ULONG; }
inline bool IsFloat  (BaseType t) { return t == BASE_TYPE_FLOAT ||	t == BASE_TYPE_DOUBLE; }

extern const char *const kTypeNames[];
extern const char kTypeSizes[];

inline size_t SizeOf(BaseType t) { return kTypeSizes[t]; }

struct StructDef;
struct EnumDef;
struct Type {
	explicit Type(BaseType _base_type = BASE_TYPE_NONE, StructDef *_sd = nullptr)
		: base_type(_base_type),
		  element(BASE_TYPE_NONE),
		  struct_def(_sd),
		  enum_def(nullptr) {}

	Type VectorType() const { return Type(element, struct_def); }
	BaseType base_type;
	BaseType element;       // only set if t == BASE_TYPE_VECTOR
	StructDef *struct_def;  // only set if t or element == BASE_TYPE_STRUCT
	EnumDef *enum_def;      // only set if t == BASE_TYPE_UNION / BASE_TYPE_UTYPE
};

struct Value {
	Value() : constant("0"), offset(-1) {}

	Type type;
	std::string constant;
	int offset;
};

template<typename T> 
class SymbolInfo {
 private:
	std::map<std::string, T *> dict;
 public:
	std::vector<T *> vec;
 public:
	~SymbolInfo() { for (auto it = vec.begin(); it != vec.end(); ++it) { delete *it; } }
	bool Add(const std::string &name, T *e) {
		vec.emplace_back(e);
		auto it = dict.find(name);
		if (it != dict.end()) return true;
		dict[name] = e;
		return false;
	}

	T *Lookup(const std::string &name) const {
		auto it = dict.find(name);
		return it == dict.end() ? nullptr : it->second;
	}
};

struct Definition {
	Definition() : generated(false) {}
	std::string name;
	std::string doc_comment;
	SymbolInfo<Value> attributes;
	bool generated;  // did we already output code for this definition?
};

struct FieldDef : public Definition {
	FieldDef() : deprecated(false), padding(0) {}
	Value value;
	bool deprecated;
	size_t padding;  // bytes to always pad after this field
};

struct StructDef : public Definition {
	StructDef()
	: fixed(false),
	  predecl(true),
	  sortbysize(true),
	  minalign(1),
	  bytesize(0) {}

	void PadLastField(size_t minalign) {
		auto padding = PaddingBytes(bytesize, minalign);
		bytesize += padding;
		if (fields.vec.size()) fields.vec.back()->padding = padding;
	}

	SymbolInfo<FieldDef> fields;
	bool fixed;       // If it's struct, not a info.
	bool predecl;     // If it's used before it was defined.
	bool sortbysize;  // Whether fields come in the declaration or size order.
	size_t minalign;  // What the whole object needs to be aligned to.
	size_t bytesize;  // Size if fixed.
};

inline bool IsStruct(const Type &type) {
	return type.base_type == BASE_TYPE_STRUCT && type.struct_def->fixed;
}

inline size_t InlineSize(const Type &type) {
	return IsStruct(type) ? type.struct_def->bytesize : SizeOf(type.base_type);
}

inline size_t InlineAlignment(const Type &type) {
	return IsStruct(type) ? type.struct_def->minalign : SizeOf(type.base_type);
}

struct EnumVal {
	EnumVal(const std::string &_name, int _val) 
	: name(_name), value(_val), struct_def(nullptr) {}
	std::string name;
	std::string doc_comment;
	int value;
	StructDef *struct_def;  // only set if this is a union
};

struct EnumDef : public Definition {
	EnumDef() : is_union(false) {}
	StructDef *ReverseLookup(int enum_idx) {
		assert(is_union);
		for (auto it = vals.vec.begin() + 1; it != vals.vec.end(); ++it) {
			if ((*it)->value == enum_idx) { return (*it)->struct_def; }
		}
		return nullptr;
	}
	SymbolInfo<EnumVal> vals;
	bool is_union;
	Type underlying_type;
};

class Parser {
 public:
	Parser() :
		root_struct_def(nullptr),
		source_(nullptr),
		cursor_(nullptr),
		line_(1) {}
	bool Parse(const char *_source);
	bool SetRootType(const char *name);

 private:
	void Next();
	bool IsNext(int t);
	void Expect(int t);
	void ParseType(Type &type);
	FieldDef &AddField(StructDef &struct_def, const std::string &name, const Type &type);
	void ParseField(StructDef &struct_def);
	void ParseAnyValue(Value &val, FieldDef *field);
	uofs_t ParseInfo(const StructDef &struct_def);
	void SerializeStruct(const StructDef &struct_def, const Value &val);
	void AddVector(bool sortbysize, int count);
	uofs_t ParseVector(const Type &type);
	void ParseMetaData(Definition &def);
	bool TryTypedValue(int dtoken, bool check, Value &e, BaseType req);
	void ParseSingleValue(Value &e);
	StructDef *LookupCreateStruct(const std::string &name);
	void ParseEnum(bool is_union);
	void ParseDecl();

 public:
	SymbolInfo<StructDef> structs_;
	SymbolInfo<EnumDef> enums_;
	std::vector<std::string> name_space_;  // As set in the schema.
	std::string error_;         // User readable error_ if Parse() == false
	MegrezBuilder builder_;  // any data contained in the file
	StructDef *root_struct_def;

 private:
	const char *source_, *cursor_;
	int line_;  // the current line being parsed
	int token_;
	std::string attribute_, doc_comment_;
	std::vector<std::pair<Value, FieldDef *>> field_stack_;
	std::vector<uint8_t> struct_stack_;
};

extern void GenerateText(const Parser &parser, const void *Megrez, int indent_step, std::string *text);

extern std::string GenerateCPP(const Parser &parser);
extern bool GenerateCPP(const Parser &parser, const std::string &path, const std::string &file_name);

}  // namespace megrez

#endif  // MEGREZ_IDL_H_