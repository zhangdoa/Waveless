#pragma once
#include "../Core/Object.h"

namespace Waveless
{
	enum class PinType
	{
		Flow,
		Bool,
		Int,
		Float,
		String,
		Vector,
		Object,
		Function,
		Delegate,
	};

	enum class PinKind
	{
		Output,
		Input
	};

	enum class NodeType
	{
		ConstVar,
		FlowFunc,
		AuxFunc,
		Comment
	};

	struct PinDescriptor : public Object
	{
		const char* Name;
		PinType Type;
		PinKind Kind;
	};

	struct ParamMetadata
	{
		const char* Type;
		const char* Name;
	};

	struct FunctionMetadata
	{
		const char* Name;
		const char* Defi;
		int ParamsCount = 0;
		int ParamsIndexOffset = 0;
	};

	struct NodeDescriptor : public Object
	{
		const char* RelativePath;
		const char* Name;
		NodeType Type = NodeType::ConstVar;
		int InputPinCount = 0;
		int OutputPinCount = 0;
		int InputPinIndexOffset = 0;
		int OutputPinIndexOffset = 0;
		FunctionMetadata* FuncMetadata;
		int Size[2] = { 0 };
		int Color[4] = { 0 };
	};
}
