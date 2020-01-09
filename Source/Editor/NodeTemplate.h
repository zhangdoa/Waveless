#pragma once
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
		Blueprint,
		Comment
	};

	struct PinDescriptor
	{
		const char* name;
		PinType type;
		PinKind kind;
	};

	struct NodeDescriptor
	{
		const char* name;
		NodeType Type = NodeType::Blueprint;
		int inputPinCount = 0;
		int outputPinCount = 0;
		int inputPinIndexOffset = 0;
		int outputPinIndexOffset = 0;
		int size[2] = { 0 };
		float color[4] = { 0.0f };
	};
}
