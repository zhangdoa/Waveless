#include "NodeModelManager.h"
#include "NodeDescriptorManager.h"
#include "../Core/Math.h"
#include "../Core/String.h"
#include "../IO/JSONParser.h"

using namespace Waveless;

namespace NodeModelManagerNS
{
	NodeModel* m_StartNode;
	NodeModel* m_EndNode;

	std::vector<NodeModel*> s_Nodes;
	std::vector<PinModel*> s_Pins;
	std::vector<LinkModel*> s_Links;
}

using namespace NodeModelManagerNS;

NodeModel* Waveless::NodeModelManager::SpawnNodeModel(const char* nodeDescriptorName)
{
	auto l_nodeDesc = NodeDescriptorManager::GetNodeDescriptor(nodeDescriptorName);
	auto l_NodeModel = new NodeModel();

	s_Nodes.emplace_back(l_NodeModel);
	l_NodeModel->Desc = l_nodeDesc;

	if (l_nodeDesc->InputPinCount)
	{
		l_NodeModel->InputPinCount = l_nodeDesc->InputPinCount;
		l_NodeModel->InputPinIndexOffset = (int)s_Pins.size();

		for (int i = 0; i < l_nodeDesc->InputPinCount; i++)
		{
			auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->InputPinIndexOffset + i, PinKind::Input);
			auto l_pin = new PinModel();

			l_pin->Desc = l_pinDesc;
			l_pin->Owner = l_NodeModel;
			auto l_instanceName = std::string(l_pinDesc->Name) + "_" + std::to_string(Waveless::Math::GenerateUUID());
			l_pin->InstanceName = StringManager::SpawnString(l_instanceName.c_str()).value;

			s_Pins.emplace_back(l_pin);
		}
	}

	if (l_nodeDesc->OutputPinCount)
	{
		l_NodeModel->OutputPinCount = l_nodeDesc->OutputPinCount;
		l_NodeModel->OutputPinIndexOffset = (int)s_Pins.size();

		for (int i = 0; i < l_nodeDesc->OutputPinCount; i++)
		{
			auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->OutputPinIndexOffset + i, PinKind::Output);
			auto l_pin = new PinModel();

			l_pin->Desc = l_pinDesc;
			l_pin->Owner = l_NodeModel;
			auto l_instanceName = std::string(l_pinDesc->Name) + "_" + std::to_string(Waveless::Math::GenerateUUID());
			l_pin->InstanceName = StringManager::SpawnString(l_instanceName.c_str()).value;

			s_Pins.emplace_back(l_pin);
		}
	}

	return l_NodeModel;
};

LinkModel* Waveless::NodeModelManager::SpawnLinkModel(PinModel* startPin, PinModel* endPin)
{
	auto l_link = new LinkModel();
	s_Links.emplace_back(l_link);

	if (startPin->Desc->Type == PinType::Flow && endPin->Desc->Type == PinType::Flow)
	{
		l_link->LinkType = LinkType::Flow;
	}
	else
	{
		l_link->LinkType = LinkType::Param;
	}

	l_link->StartPin = startPin;
	l_link->EndPin = endPin;

	startPin->Owner->ConnectionState = NodeConnectionState::Connected;
	endPin->Owner->ConnectionState = NodeConnectionState::Connected;

	return l_link;
}

PinModel * Waveless::NodeModelManager::GetPinModel(int index)
{
	return s_Pins[index];
}

NodeModel * Waveless::NodeModelManager::GetNodeModel(int index)
{
	return s_Nodes[index];
}

LinkModel * Waveless::NodeModelManager::GetLinkModel(int index)
{
	return s_Links[index];
}

const std::vector<NodeModel*>& Waveless::NodeModelManager::GetAllNodeModels()
{
	return s_Nodes;
}

const std::vector<LinkModel*>& Waveless::NodeModelManager::GetAllLinkModels()
{
	return s_Links;
}

static PinModel* FindPinByUUID(uint64_t id)
{
	if (!id)
		return nullptr;

	for (auto pin : s_Pins)
	{
		if (pin->UUID == id)
			return pin;
	}

	return nullptr;
}

WsResult Waveless::NodeModelManager::LoadCanvas(const char * inputFileName)
{
	for (auto node : s_Nodes)
	{
		delete node;
	}
	for (auto pin : s_Pins)
	{
		delete pin;
	}
	for (auto link : s_Links)
	{
		delete link;
	}

	s_Nodes.clear();
	s_Pins.clear();
	s_Links.clear();

	s_Nodes.shrink_to_fit();
	s_Pins.shrink_to_fit();
	s_Links.shrink_to_fit();

	auto l_filePath = "..//..//Asset//Canvas//" + std::string(inputFileName);
	json j;

	if (JSONParser::loadJsonDataFromDisk(l_filePath.c_str(), j) != WsResult::Success)
	{
		return WsResult::Fail;
	};

	for (auto& j_node : j["Nodes"])
	{
		auto l_nodeName = std::string(j_node["Name"]);
		auto node = SpawnNodeModel(l_nodeName.c_str());
		node->UUID = j_node["ID"];

		if (l_nodeName == "Input")
		{
			m_StartNode = node;
		}
		else if (l_nodeName == "Output")
		{
			m_EndNode = node;
		}

		for (auto& j_input : j_node["Inputs"])
		{
			for (uint64_t i = 0; i < node->InputPinCount; i++)
			{
				auto l_pin = GetPinModel(node->InputPinIndexOffset + (int)i);
				if (l_pin->Desc->Name == j_input["Name"])
				{
					l_pin->UUID = j_input["ID"];
					if (l_pin->Desc->Type == PinType::String)
					{
						std::string l_stringTemp = j_input["Value"];
						auto l_string = StringManager::SpawnString(l_stringTemp.c_str());
						l_pin->Value = l_string.UUID;
					}
					else
					{
						l_pin->Value = j_input["Value"];
					}
				}
			}
		}

		for (auto& j_output : j_node["Outputs"])
		{
			for (uint64_t i = 0; i < node->OutputPinCount; i++)
			{
				auto l_pin = GetPinModel(node->OutputPinIndexOffset + (int)i);
				if (l_pin->Desc->Name == j_output["Name"])
				{
					l_pin->UUID = j_output["ID"];
					if (l_pin->Desc->Type == PinType::String)
					{
						std::string l_stringTemp = j_output["Value"];
						auto l_string = StringManager::SpawnString(l_stringTemp.c_str());
						l_pin->Value = l_string.UUID;
					}
					else
					{
						l_pin->Value = j_output["Value"];
					}
				}
			}
		}

		node->InitialPosition[0] = j_node["Position"]["X"];
		node->InitialPosition[1] = j_node["Position"]["Y"];
	}

	for (auto& j_link : j["Links"])
	{
		auto l_startPin = FindPinByUUID(j_link["StartPinID"]);
		auto l_endPin = FindPinByUUID(j_link["EndPinID"]);

		auto l_link = SpawnLinkModel(l_startPin, l_endPin);
		l_link->UUID = j_link["ID"];
	}

	return WsResult::Success;
}