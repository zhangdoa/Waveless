#include "NodeCompiler.h"
#include "../IO/JSONParser.h"
#include "../IO/IOService.h"
#include "NodeDescriptorManager.h"

using namespace Waveless;

struct NodeModel;

using PinValue = uint64_t;

struct PinModel : public Object
{
	PinDescriptor* Desc;

	PinValue Value;
	NodeModel* NodeModel;
};

struct NodeModel : public Object
{
	NodeDescriptor* Desc;

	std::vector<PinModel> Inputs;
	std::vector<PinModel> Outputs;
};

struct LinkModel : public Object
{
	PinModel* StartPin = 0;
	PinModel* EndPin = 0;
};

namespace NodeCompilerNS
{
	std::vector<NodeModel> s_Nodes;
	std::vector<PinModel> s_InputPins;
	std::vector<PinModel> s_OutputPins;
	std::vector<LinkModel> s_Links;
}

using namespace NodeCompilerNS;

NodeModel* SpawnNodeModel(const char* nodeDescriptorName)
{
	auto l_nodeDesc = NodeDescriptorManager::GetNodeDescriptor(nodeDescriptorName);

	s_Nodes.emplace_back();
	s_Nodes.back().Desc = l_nodeDesc;

	for (int i = 0; i < l_nodeDesc->InputPinCount; i++)
	{
		auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->InputPinIndexOffset + i, PinKind::Input);
		s_Nodes.back().Inputs.emplace_back();
		s_Nodes.back().Inputs.back().Desc = l_pinDesc;
		s_Nodes.back().Inputs.back().NodeModel = &s_Nodes.back();
	}
	for (int i = 0; i < l_nodeDesc->OutputPinCount; i++)
	{
		auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->OutputPinIndexOffset + i, PinKind::Output);
		s_Nodes.back().Outputs.emplace_back();
		s_Nodes.back().Outputs.back().Desc = l_pinDesc;
		s_Nodes.back().Outputs.back().NodeModel = &s_Nodes.back();
	}

	return &s_Nodes.back();
};

LinkModel* SpawnLinkModel(PinModel* startPin, PinModel* endPin)
{
	s_Links.emplace_back();
	s_Links.back().StartPin = startPin;
	s_Links.back().EndPin = endPin;

	return &s_Links.back();
}

static PinModel* FindPinByUUID(uint64_t id)
{
	if (!id)
		return nullptr;

	for (auto& node : s_Nodes)
	{
		for (auto& pin : node.Inputs)
			if (pin.UUID == id)
				return &pin;

		for (auto& pin : node.Outputs)
			if (pin.UUID == id)
				return &pin;
	}

	return nullptr;
}

WsResult NodeCompiler::Compile(const char* inputFileName, const char* outputFileName)
{
	json j;

	JSONParser::loadJsonDataFromDisk(("..//..//Asset//Canvas//" + std::string(inputFileName)).c_str(), j);

	std::vector<char> l_codes;

	for (auto& j_node : j["Nodes"])
	{
		auto node = SpawnNodeModel(std::string(j_node["Name"]).c_str());
		node->UUID = j_node["ID"];

		for (auto& j_input : j_node["Inputs"])
		{
			for (auto& pin : node->Inputs)
			{
				if (pin.Desc->Name == j_input["Name"])
				{
					pin.UUID = j_input["ID"];
					pin.Value = j_input["Value"];
				}
			}
		}

		for (auto& j_output : j_node["Outputs"])
		{
			for (auto& pin : node->Outputs)
			{
				if (pin.Desc->Name == j_output["Name"])
				{
					pin.UUID = j_output["ID"];
					pin.Value = j_output["Value"];
				}
			}
		}

		auto l_fileName = std::string(j_node["Name"]);
		auto l_filePath = "..//..//Asset//Nodes//" + l_fileName + ".h";

		std::vector<char> l_code;

		if (IOService::loadFile(l_filePath.c_str(), l_code, IOService::IOMode::Text) == WsResult::Success)
		{
			auto l_codeStr = std::string(&l_code[0]);

			l_codeStr.insert(l_codeStr.find("Execute") + 7, "_" + l_fileName);
			l_codeStr.append("\n\n");
			std::move(l_codeStr.begin(), l_codeStr.end(), std::back_inserter(l_codes));
		}
	}

	for (auto& j_link : j["Links"])
	{
		auto l_startPin = FindPinByUUID(j_link["StartPinID"]);
		auto l_endPin = FindPinByUUID(j_link["EndPinID"]);

		SpawnLinkModel(l_startPin, l_endPin);
	}

	auto l_outputPath = "..//..//Asset//Canvas//" + std::string(inputFileName) + ".h";

	if (IOService::saveFile(l_outputPath.c_str(), l_codes, IOService::IOMode::Text) == WsResult::Success)
	{
		return WsResult::Success;
	}
	else
	{
		return WsResult::Fail;
	}
}