#pragma once
#include "NodeTemplate.h"
#include "../Core/stdafx.h"

namespace Waveless
{
	struct NodeModel;

	using PinValue = uint64_t;

	struct PinModel : public Object
	{
		PinDescriptor* Desc;

		NodeModel* Owner = 0;
		const char* InstanceName = 0;
		PinValue Value;
	};

	enum class NodeConnectionState { Isolate, Connected };

	struct NodeModel : public Object
	{
		NodeDescriptor* Desc;

		NodeConnectionState ConnectionState = NodeConnectionState::Isolate;
		int InputPinCount = 0;
		int OutputPinCount = 0;
		int InputPinIndexOffset = 0;
		int OutputPinIndexOffset = 0;
		float InitialPosition[2];
	};

	enum class LinkType { Flow, Param };

	struct LinkModel : public Object
	{
		LinkType LinkType = LinkType::Flow;
		PinModel* StartPin = 0;
		PinModel* EndPin = 0;
	};

	namespace NodeModelManager
	{
		NodeModel* SpawnNodeModel(const char* nodeDescriptorName);
		LinkModel* SpawnLinkModel(PinModel* startPin, PinModel* endPin);

		PinModel* GetPinModel(int index);
		NodeModel* GetNodeModel(int index);
		LinkModel* GetLinkModel(int index);

		const std::vector<NodeModel*>& GetAllNodeModels();
		const std::vector<LinkModel*>& GetAllLinkModels();

		WsResult LoadCanvas(const char * inputFileName);
	};
}