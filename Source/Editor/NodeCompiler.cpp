#include "NodeCompiler.h"
#include "../IO/JSONParser.h"
#include "../IO/IOService.h"
#include "../Core/Math.h"
#include "NodeDescriptorManager.h"
#include "NodeModelManager.h"

using namespace Waveless;

namespace NodeCompilerNS
{
	NodeModel* StartNode;
	NodeModel* EndNode;
}

using namespace NodeCompilerNS;

void SortNodes()
{
	auto l_nodes = NodeModelManager::GetAllNodeModels();
	auto& l_links = NodeModelManager::GetAllLinkModels();

	for (auto link : l_links)
	{
		auto l_startNode = std::find(l_nodes.begin(), l_nodes.end(), link->StartPin->Owner);
		auto l_endNode = std::find(l_nodes.begin(), l_nodes.end(), link->EndPin->Owner);

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
	auto& l_nodes = NodeModelManager::GetAllNodeModels();

	for (auto node : l_nodes)
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

		auto l_pin = NodeModelManager::GetPinModel(node->OutputPinIndexOffset + (int)i);

		l_constDecl += l_type;
		l_constDecl += " ";
		l_constDecl += node->Desc->FuncMetadata->Name;
		l_constDecl += "_";
		l_constDecl += l_paramMetadata->Name;
		l_constDecl += "_";
		l_constDecl += l_pin->InstanceName;
		l_constDecl += " = ";
		l_constDecl += std::to_string(l_pin->Value);
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

		auto l_pin = NodeModelManager::GetPinModel(node->InputPinIndexOffset + (int)i);

		l_localVarDecl += l_type;
		l_localVarDecl += " ";
		l_localVarDecl += node->Desc->FuncMetadata->Name;
		l_localVarDecl += "_";
		l_localVarDecl += l_paramMetadata->Name;
		l_localVarDecl += "_";
		l_localVarDecl += l_pin->InstanceName;
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
	auto& l_nodes = NodeModelManager::GetAllNodeModels();

	for (auto node : l_nodes)
	{
		if (node->ConnectionState == NodeConnectionState::Connected)
		{
			// Functions without input could be executed at any time, or they are the constant declarations
			if (node->InputPinCount == 0 && node != StartNode)
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
	for (auto node : l_nodes)
	{
		if (node->ConnectionState == NodeConnectionState::Connected)
		{
			// Functions which has the execution order restriction
			if (node->InputPinCount > 0 || node == StartNode)
			{
				WriteFunctionInvocation(node, TU);
			}
		}
	}
}

WsResult NodeCompiler::Compile(const char* inputFileName, const char* outputFileName)
{
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