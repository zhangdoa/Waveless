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
	std::string FuncSign;
	std::string FuncBody;

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
	std::vector<NodeModel*> s_Nodes;
	std::vector<LinkModel*> s_Links;
}

using namespace NodeCompilerNS;

NodeModel* SpawnNodeModel(const char* nodeDescriptorName)
{
	auto l_nodeDesc = NodeDescriptorManager::GetNodeDescriptor(nodeDescriptorName);
	auto l_NodeModel = new NodeModel();

	s_Nodes.emplace_back(l_NodeModel);
	l_NodeModel->Desc = l_nodeDesc;

	for (int i = 0; i < l_nodeDesc->InputPinCount; i++)
	{
		auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->InputPinIndexOffset + i, PinKind::Input);
		l_NodeModel->Inputs.emplace_back();
		l_NodeModel->Inputs.back().Desc = l_pinDesc;
		l_NodeModel->Inputs.back().NodeModel = l_NodeModel;
	}
	for (int i = 0; i < l_nodeDesc->OutputPinCount; i++)
	{
		auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->OutputPinIndexOffset + i, PinKind::Output);
		l_NodeModel->Outputs.emplace_back();
		l_NodeModel->Outputs.back().Desc = l_pinDesc;
		l_NodeModel->Outputs.back().NodeModel = l_NodeModel;
	}

	return l_NodeModel;
};

LinkModel* SpawnLinkModel(PinModel* startPin, PinModel* endPin)
{
	auto l_link = new LinkModel();
	s_Links.emplace_back(l_link);

	l_link->StartPin = startPin;
	l_link->EndPin = endPin;

	return l_link;
}

static PinModel* FindPinByUUID(uint64_t id)
{
	if (!id)
		return nullptr;

	for (auto node : s_Nodes)
	{
		for (auto& pin : node->Inputs)
			if (pin.UUID == id)
				return &pin;

		for (auto& pin : node->Outputs)
			if (pin.UUID == id)
				return &pin;
	}

	return nullptr;
}

void LoadModels(const char * inputFileName)
{
	json j;

	JSONParser::loadJsonDataFromDisk(("..//..//Asset//Canvas//" + std::string(inputFileName)).c_str(), j);

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
	}

	for (auto& j_link : j["Links"])
	{
		auto l_startPin = FindPinByUUID(j_link["StartPinID"]);
		auto l_endPin = FindPinByUUID(j_link["EndPinID"]);

		SpawnLinkModel(l_startPin, l_endPin);
	}
}

void SortModels()
{
	for (auto link : s_Links)
	{
		auto l_startNode = std::find(s_Nodes.begin(), s_Nodes.end(), link->StartPin->NodeModel);
		auto l_endNode = std::find(s_Nodes.begin(), s_Nodes.end(), link->EndPin->NodeModel);

		if (l_startNode > l_endNode)
		{
			std::swap(l_startNode, l_endNode);
		}
	}
}

WsResult NodeCompiler::Compile(const char* inputFileName, const char* outputFileName)
{
	LoadModels(inputFileName);
	SortModels();

	std::vector<char> l_TU;

	for (auto node : s_Nodes)
	{
		auto l_fileName = std::string(node->Desc->RelativePath);
		l_fileName = l_fileName.substr(0, l_fileName.find("."));
		auto l_filePath = "..//..//Asset//Nodes//" + l_fileName + ".h";

		std::vector<char> l_code;

		if (IOService::loadFile(l_filePath.c_str(), l_code, IOService::IOMode::Text) == WsResult::Success)
		{
			std::string l_func = &l_code[0];
			auto l_funcName = l_fileName;
			std::replace(l_funcName.begin(), l_funcName.end(), '/', '_');

			l_func.insert(l_func.find("Execute") + 7, "_" + l_funcName);
			l_func.append("\n\n");

			auto l_signEndPos = l_func.find_first_of("\n");

			node->FuncSign = l_func.substr(0, l_signEndPos);
			node->FuncBody = l_func.substr(l_signEndPos + 1, std::string::npos);

			std::copy(l_func.begin(), l_func.end(), std::back_inserter(l_TU));
		}
	}

	auto l_scriptSign = "void EventScript_" + IOService::getFileName(inputFileName);

	auto l_scriptBody = "()\n{\n}";

	auto l_script = l_scriptSign + l_scriptBody;

	std::copy(l_script.begin(), l_script.end(), std::back_inserter(l_TU));

	auto l_outputPath = "..//..//Asset//Canvas//" + std::string(inputFileName) + ".h";

	if (IOService::saveFile(l_outputPath.c_str(), l_TU, IOService::IOMode::Text) == WsResult::Success)
	{
		return WsResult::Success;
	}
	else
	{
		return WsResult::Fail;
	}
}