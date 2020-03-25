#include "NodeCompiler.h"
#include "../IO/JSONParser.h"
#include "../Core/Logger.h"
#include "../IO/IOService.h"
#include "../Core/Math.h"
#include "NodeDescriptorManager.h"
#include "NodeModelManager.h"

using namespace Waveless;

struct NodeOrderInfo
{
	NodeModel* Model;
	uint64_t Index;
};

namespace NodeCompilerNS
{
	NodeModel* m_StartNode;
	NodeModel* m_EndNode;
	uint64_t m_CurrentIndex = 0;
	std::vector<NodeOrderInfo> m_SortedNodes;
}

using namespace NodeCompilerNS;

void SortNodes()
{
	m_CurrentIndex = 0;
	m_SortedNodes.clear();

	auto l_modes = NodeModelManager::GetAllNodeModels();
	auto l_links = NodeModelManager::GetAllLinkModels();

	auto l_startNodeIt = std::find_if(l_modes.begin(), l_modes.end(), [](NodeModel* val) {
		return !strcmp(val->Desc->Name, "Input");
	});
	auto l_endNodeIt = std::find_if(l_modes.begin(), l_modes.end(), [](NodeModel* val) {
		return !strcmp(val->Desc->Name, "Output");
	});

	if (l_startNodeIt == l_modes.end())
	{
		Logger::Log(LogLevel::Error, "No Start Node.");
		return;
	}
	if (l_endNodeIt == l_modes.end())
	{
		Logger::Log(LogLevel::Error, "No End Node.");
		return;
	}

	auto l_startNode = *l_startNodeIt;
	auto l_endNode = *l_endNodeIt;

	auto l_currentNode = l_startNode;

	while (l_currentNode != l_endNode)
	{
		NodeOrderInfo l_nodeOrderInfo;
		l_nodeOrderInfo.Model = l_currentNode;
		l_nodeOrderInfo.Index = m_CurrentIndex;
		m_SortedNodes.emplace_back(l_nodeOrderInfo);

		auto l_link = *std::find_if(l_links.begin(), l_links.end(), [l_currentNode](LinkModel* val) { return val->StartPin->Owner == l_currentNode; });
		l_currentNode = l_link->EndPin->Owner;

		m_CurrentIndex++;
	}

	NodeOrderInfo l_nodeOrderInfo;
	l_nodeOrderInfo.Model = l_endNode;
	l_nodeOrderInfo.Index = m_CurrentIndex;
	m_SortedNodes.emplace_back(l_nodeOrderInfo);
}

void WriteIncludes(std::vector<char>& TU)
{
	std::string l_waveParser = "#include \"../../../Source/IO/WaveParser.h\"\n";
	std::string l_audioEngine = "#include \"../../../Source/Runtime/AudioEngine.h\"\n";
	std::string l_usingNS = "using namespace Waveless;\n\n";

	std::copy(l_waveParser.begin(), l_waveParser.end(), std::back_inserter(TU));
	std::copy(l_audioEngine.begin(), l_audioEngine.end(), std::back_inserter(TU));
	std::copy(l_usingNS.begin(), l_usingNS.end(), std::back_inserter(TU));
}

void WriteFunctionDefinitions(std::vector<char>& TU)
{
	for (auto node : m_SortedNodes)
	{
		if (node.Model->ConnectionState == NodeConnectionState::Connected)
		{
			if (node.Model->Desc->Type == NodeType::Function)
			{
				std::string l_sign;
				l_sign += "void ";
				l_sign += node.Model->Desc->FuncMetadata->Name;
				l_sign += "(";
				for (size_t i = 0; i < node.Model->Desc->FuncMetadata->ParamsCount; i++)
				{
					auto l_paramMetadata = NodeDescriptorManager::GetParamMetadata(int(i) + node.Model->Desc->FuncMetadata->ParamsIndexOffset);
					l_sign += l_paramMetadata->Type;
					l_sign += " ";
					l_sign += l_paramMetadata->Name;

					if (i < node.Model->Desc->FuncMetadata->ParamsCount - 1)
					{
						l_sign += ", ";
					}
				}

				l_sign += ")\n";

				std::copy(l_sign.begin(), l_sign.end(), std::back_inserter(TU));

				std::string l_defi = node.Model->Desc->FuncMetadata->Defi;
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

	for (auto node : m_SortedNodes)
	{
		if (node.Model->ConnectionState == NodeConnectionState::Connected)
		{
			// Functions without input could be executed at any time, or they are the constant declarations
			if (node.Model->InputPinCount == 0 && node.Model != m_StartNode)
			{
				if (node.Model->Desc->Type == NodeType::ConstVar)
				{
					WriteConstant(node.Model, TU);
				}
				else
				{
					WriteLocalVar(node.Model, TU);

					WriteFunctionInvocation(node.Model, TU);
				}
			}
		}
	}
	for (auto node : m_SortedNodes)
	{
		if (node.Model->ConnectionState == NodeConnectionState::Connected)
		{
			// Functions which has the execution order restriction
			if (node.Model->InputPinCount > 0 || node.Model == m_StartNode)
			{
				WriteFunctionInvocation(node.Model, TU);
			}
		}
	}
}

WsResult NodeCompiler::Compile(const char* inputFileName, const char* outputFileName)
{
	SortNodes();

	std::vector<char> l_TU;

	WriteIncludes(l_TU);

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