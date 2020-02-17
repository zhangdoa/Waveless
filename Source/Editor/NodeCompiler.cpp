#include "NodeCompiler.h"
#include "../IO/JSONParser.h"
#include "../IO/IOService.h"
#include "../Core/Math.h"
#include "NodeDescriptorManager.h"

using namespace Waveless;

struct NodeModel;

using PinValue = uint64_t;

struct PinModel : public Object
{
	PinDescriptor* Desc;

	NodeModel* Owner;
	PinValue Value;
};

enum class NodeConnectionState { Isolate, Connected };

struct NodeModel : public Object
{
	NodeDescriptor* Desc;
	NodeConnectionState ConnectionState = NodeConnectionState::Isolate;

	std::vector<PinModel> Inputs;
	std::vector<PinModel> Outputs;
};

enum class LinkType { Flow, Param };

struct LinkModel : public Object
{
	LinkType LinkType = LinkType::Flow;
	PinModel* StartPin = 0;
	PinModel* EndPin = 0;
};

namespace NodeCompilerNS
{
	NodeModel* StartNode;
	NodeModel* EndNode;
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
		l_NodeModel->Inputs.back().Owner = l_NodeModel;
	}
	for (int i = 0; i < l_nodeDesc->OutputPinCount; i++)
	{
		auto l_pinDesc = NodeDescriptorManager::GetPinDescriptor(l_nodeDesc->OutputPinIndexOffset + i, PinKind::Output);
		l_NodeModel->Outputs.emplace_back();
		l_NodeModel->Outputs.back().Desc = l_pinDesc;
		l_NodeModel->Outputs.back().Owner = l_NodeModel;
	}

	return l_NodeModel;
};

LinkModel* SpawnLinkModel(PinModel* startPin, PinModel* endPin)
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

static void LoadCanvas(const char * inputFileName)
{
	json j;

	JSONParser::loadJsonDataFromDisk(("..//..//Asset//Canvas//" + std::string(inputFileName)).c_str(), j);

	for (auto& j_node : j["Nodes"])
	{
		auto l_nodeName = std::string(j_node["Name"]);
		auto node = SpawnNodeModel(l_nodeName.c_str());
		node->UUID = j_node["ID"];

		if (l_nodeName == "Input")
		{
			StartNode = node;
		}
		else if (l_nodeName == "Output")
		{
			EndNode = node;
		}

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

void SortNodes()
{
	for (auto link : s_Links)
	{
		auto l_startNode = std::find(s_Nodes.begin(), s_Nodes.end(), link->StartPin->Owner);
		auto l_endNode = std::find(s_Nodes.begin(), s_Nodes.end(), link->EndPin->Owner);

		if (link->LinkType == LinkType::Flow)
		{
			if (l_startNode > l_endNode)
			{
				std::iter_swap(l_startNode, l_endNode);
			}
		}
	}
}

void WriteFunctionDefinitions(std::vector<char>& TU)
{
	for (auto node : s_Nodes)
	{
		if (node->ConnectionState == NodeConnectionState::Connected)
		{
			if (node->Desc->Type == NodeType::Function)
			{
				std::string l_sign;
				l_sign += "void ";
				l_sign += node->Desc->FuncMetadata->Name;
				l_sign += "(";
				for (size_t i = 0; i < node->Desc->FuncMetadata->ParamsCount; i++)
				{
					auto l_paramMetadata = NodeDescriptorManager::GetParamMetadata(int(i) + node->Desc->FuncMetadata->ParamsIndexOffset);
					l_sign += l_paramMetadata->Type;
					l_sign += " ";
					l_sign += l_paramMetadata->Name;

					if (i < node->Desc->FuncMetadata->ParamsCount - 1)
					{
						l_sign += ", ";
					}
				}

				l_sign += ");\n";

				std::copy(l_sign.begin(), l_sign.end(), std::back_inserter(TU));

				std::string l_defi = node->Desc->FuncMetadata->Defi;
				l_defi += "\n\n";

				std::copy(l_defi.begin(), l_defi.end(), std::back_inserter(TU));
			}
		}
	}
}

void WriteConstant(NodeModel* node, std::vector<char> & TU)
{
	std::string l_constDecl;

	for (size_t i = 0; i < node->Desc->FuncMetadata->ParamsCount; i++)
	{
		auto l_paramMetadata = NodeDescriptorManager::GetParamMetadata(int(i) + node->Desc->FuncMetadata->ParamsIndexOffset);
		l_constDecl += "\tconst ";

		std::string l_type = l_paramMetadata->Type;
		if (!strcmp(&l_type.back(), "&"))
		{
			l_type = l_type.substr(0, l_type.size() - 1);
		}

		l_constDecl += l_type;
		l_constDecl += " ";
		l_constDecl += node->Desc->FuncMetadata->Name;
		l_constDecl += "_";
		l_constDecl += l_paramMetadata->Name;
		l_constDecl += "_";
		l_constDecl += std::to_string(Waveless::Math::GenerateUUID());
		l_constDecl += " =";
		l_constDecl += " value";
		l_constDecl += ";\n";
	}

	std::copy(l_constDecl.begin(), l_constDecl.end(), std::back_inserter(TU));
}

void WriteLocalVar(NodeModel* node, std::vector<char> & TU)
{
	std::string l_localVarDecl;

	for (size_t i = 0; i < node->Desc->FuncMetadata->ParamsCount; i++)
	{
		auto l_paramMetadata = NodeDescriptorManager::GetParamMetadata(int(i) + node->Desc->FuncMetadata->ParamsIndexOffset);
		l_localVarDecl += "\t";

		std::string l_type = l_paramMetadata->Type;
		if (!strcmp(&l_type.back(), "&"))
		{
			l_type = l_type.substr(0, l_type.size() - 1);
		}

		l_localVarDecl += l_type;
		l_localVarDecl += " ";
		l_localVarDecl += node->Desc->FuncMetadata->Name;
		l_localVarDecl += "_";
		l_localVarDecl += l_paramMetadata->Name;
		l_localVarDecl += "_";
		l_localVarDecl += std::to_string(Waveless::Math::GenerateUUID());
		l_localVarDecl += ";\n";
	}

	std::copy(l_localVarDecl.begin(), l_localVarDecl.end(), std::back_inserter(TU));
}

void WriteFunctionInvocation(NodeModel* node, std::vector<char> & TU)
{
	std::string l_funcInvocation;

	l_funcInvocation += "\t";
	l_funcInvocation += node->Desc->FuncMetadata->Name;
	l_funcInvocation += "(";

	for (size_t i = 0; i < node->Desc->FuncMetadata->ParamsCount; i++)
	{
		auto l_paramMetadata = NodeDescriptorManager::GetParamMetadata(int(i) + node->Desc->FuncMetadata->ParamsIndexOffset);
		l_funcInvocation += node->Desc->FuncMetadata->Name;
		l_funcInvocation += "_";
		l_funcInvocation += l_paramMetadata->Name;

		if (i < node->Desc->FuncMetadata->ParamsCount - 1)
		{
			l_funcInvocation += ", ";
		}
	}

	l_funcInvocation += ");\n";

	std::copy(l_funcInvocation.begin(), l_funcInvocation.end(), std::back_inserter(TU));
}

void WriteExecutionFlows(std::vector<char>& TU)
{
	std::unordered_map<uint64_t, std::string> l_localVarDecls;

	for (auto node : s_Nodes)
	{
		if (node->ConnectionState == NodeConnectionState::Connected)
		{
			// Functions without input could be executed at any time, or they are the constant declarations
			if (node->Inputs.size() == 0 && node != StartNode)
			{
				if (node->Desc->Type == NodeType::ConstVar)
				{
					WriteConstant(node, TU);
				}
				else
				{
					WriteLocalVar(node, TU);

					WriteFunctionInvocation(node, TU);
				}
			}
		}
	}
	for (auto node : s_Nodes)
	{
		if (node->ConnectionState == NodeConnectionState::Connected)
		{
			// Functions which has the execution order restriction
			if (node->Inputs.size() > 0 || node == StartNode)
			{
				WriteFunctionInvocation(node, TU);
			}
		}
	}
}

WsResult NodeCompiler::Compile(const char* inputFileName, const char* outputFileName)
{
	for (auto node : s_Nodes)
	{
		node->Inputs.clear();
		node->Outputs.clear();
		node->Inputs.shrink_to_fit();
		node->Outputs.shrink_to_fit();
		delete node;
	}
	for (auto& link : s_Links)
	{
		delete link;
	}

	s_Nodes.clear();
	s_Links.clear();
	s_Nodes.shrink_to_fit();
	s_Links.shrink_to_fit();

	LoadCanvas(inputFileName);

	SortNodes();

	std::vector<char> l_TU;

	WriteFunctionDefinitions(l_TU);

	auto l_scriptSign = "void EventScript_" + IOService::getFileName(inputFileName);
	std::string l_scriptBodyBegin = "()\n{\n";
	std::string l_scriptBodyEnd = "}";

	std::copy(l_scriptSign.begin(), l_scriptSign.end(), std::back_inserter(l_TU));
	std::copy(l_scriptBodyBegin.begin(), l_scriptBodyBegin.end(), std::back_inserter(l_TU));

	WriteExecutionFlows(l_TU);

	std::copy(l_scriptBodyEnd.begin(), l_scriptBodyEnd.end(), std::back_inserter(l_TU));

	auto l_outputPath = "..//..//Asset//Canvas//" + std::string(inputFileName) + ".cpp";

	if (IOService::saveFile(l_outputPath.c_str(), l_TU, IOService::IOMode::Text) == WsResult::Success)
	{
		return WsResult::Success;
	}
	else
	{
		return WsResult::Fail;
	}
}