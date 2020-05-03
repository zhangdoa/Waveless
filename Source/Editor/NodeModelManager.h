#pragma once
#include "NodeTemplate.h"
#include "../Core/stdafx.h"

namespace Waveless
{
	struct NodeModel;

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
		WsResult SpawnNodeModel(const char* nodeDescriptorName, NodeModel*& result);
		WsResult SpawnLinkModel(PinModel* startPin, PinModel* endPin, LinkModel*& result);

		WsResult GetPinModel(int index, PinModel*& result);
		WsResult GetNodeModel(int index, NodeModel*& result);
		WsResult GetLinkModel(int index, LinkModel*& result);

		WsResult GetAllNodeModels(std::vector<NodeModel*>*& result);
		WsResult GetAllLinkModels(std::vector<LinkModel*>*& result);

		WsResult LoadCanvas(const char * inputFileName);
	};
}