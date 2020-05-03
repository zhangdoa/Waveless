#include "NodeCompiler.h"
#include "../IO/JSONParser.h"
#include "../Core/Logger.h"
#include "../IO/IOService.h"
#include "../Core/Math.h"
#include "../Core/String.h"

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
	NodeModel* m_StartNode = 0;
	NodeModel* m_EndNode = 0;
	uint64_t m_CurrentIndex = 0;
	std::vector<NodeOrderInfo> m_NodeOrderInfos;
	std::vector<NodeDescriptor*> m_NodeDescriptors;
}

using namespace NodeCompilerNS;

void AddAuxFunctionNodes(NodeModel* node, const std::vector<LinkModel*>& links)
{
	for (size_t i = 0; i < node->InputPinCount; i++)
	{
		PinModel* l_endPin;
		NodeModelManager::GetPinModel(node->InputPinIndexOffset + (int)i, l_endPin);

		auto l_linkIt = std::find_if(links.begin(), links.end(), [l_endPin](LinkModel* val)
		{
			return val->EndPin == l_endPin;
		});

		if (l_linkIt != links.end())
		{
			auto l_link = *l_linkIt;

			if (l_link->LinkType == LinkType::Param)
			{
				auto l_startPin = l_link->StartPin;

				// Still has dependencies
				if (l_startPin->Owner->InputPinCount)
				{
					AddAuxFunctionNodes(l_startPin->Owner, links);
				}

				if (l_startPin->Owner->Desc->Type != NodeType::FlowFunc)
				{
					NodeOrderInfo l_nodeOrderInfo;
					l_nodeOrderInfo.Model = l_startPin->Owner;
					l_nodeOrderInfo.Index = m_CurrentIndex;
					m_NodeOrderInfos.emplace_back(l_nodeOrderInfo);
					m_CurrentIndex++;
				}
			}
		}
	}
}

void GetNodeOrderInfos()
{
	m_StartNode = 0;
	m_EndNode = 0;
	m_CurrentIndex = 0;
	m_NodeOrderInfos.clear();

	std::vector<NodeModel*>* l_modes;
	NodeModelManager::GetAllNodeModels(l_modes);
	std::vector<LinkModel*>* l_links;
	NodeModelManager::GetAllLinkModels(l_links);

	auto l_startNodeIt = std::find_if(l_modes->begin(), l_modes->end(), [](NodeModel* val) {
		return strstr(val->Desc->Name, "Input");
	});
	auto l_endNodeIt = std::find_if(l_modes->begin(), l_modes->end(), [](NodeModel* val) {
		return strstr(val->Desc->Name, "Output");
	});

	if (l_startNodeIt == l_modes->end())
	{
		Logger::Log(LogLevel::Error, "No Start Node.");
		return;
	}
	if (l_endNodeIt == l_modes->end())
	{
		Logger::Log(LogLevel::Error, "No End Node.");
		return;
	}

	m_StartNode = *l_startNodeIt;
	m_EndNode = *l_endNodeIt;

	auto l_startNode = m_StartNode;
	auto l_endNode = m_EndNode;

	auto l_currentNode = l_startNode;

	// Iterite over the execution flow
	while (l_currentNode)
	{
		// Parameter dependencies
		AddAuxFunctionNodes(l_currentNode, *l_links);

		NodeOrderInfo l_nodeOrderInfo;
		l_nodeOrderInfo.Model = l_currentNode;
		l_nodeOrderInfo.Index = m_CurrentIndex;
		m_NodeOrderInfos.emplace_back(l_nodeOrderInfo);

		// Next node
		auto l_linkIt = std::find_if(l_links->begin(), l_links->end(), [l_currentNode](LinkModel* val) {
			return (val->StartPin->Owner == l_currentNode) && (val->LinkType == LinkType::Flow);
		});

		if (l_linkIt != l_links->end())
		{
			auto l_link = *l_linkIt;
			l_currentNode = l_link->EndPin->Owner;
			m_CurrentIndex++;
		}
		else
		{
			l_currentNode = 0;
		}
	}

	// eliminate duplication
	std::sort(m_NodeOrderInfos.begin(), m_NodeOrderInfos.end(), [](const NodeOrderInfo& lhs, const NodeOrderInfo& rhs)
	{
		return lhs.Model < rhs.Model;
	});

	auto it = std::unique(m_NodeOrderInfos.begin(), m_NodeOrderInfos.end(), [](const NodeOrderInfo& A, const NodeOrderInfo& B)
	{
		return A.Model == B.Model;
	});
	m_NodeOrderInfos.resize(std::distance(m_NodeOrderInfos.begin(), it));

	// sort in correct flow index
	std::sort(m_NodeOrderInfos.begin(), m_NodeOrderInfos.end(), [](const NodeOrderInfo& lhs, const NodeOrderInfo& rhs)
	{
		return lhs.Index < rhs.Index;
	});
}

void GetNodeDescriptors()
{
	m_NodeDescriptors.clear();

	for (auto i : m_NodeOrderInfos)
	{
		m_NodeDescriptors.emplace_back(i.Model->Desc);
	}

	std::sort(m_NodeDescriptors.begin(), m_NodeDescriptors.end());

	auto it = std::unique(m_NodeDescriptors.begin(), m_NodeDescriptors.end());

	m_NodeDescriptors.resize(std::distance(m_NodeDescriptors.begin(), it));
}

void WriteIncludes(std::vector<char>& TU)
{
	std::string l_APIExport = "#define WS_CANVAS_EXPORTS\n#include \"../../Source/Core/WsCanvasAPIExport.h\"\n";
	std::string l_math = "#include \"../../Source/Core/Math.h\"\n";
	std::string l_waveParser = "#include \"../../Source/IO/WaveParser.h\"\n";
	std::string l_audioEngine = "#include \"../../Source/Runtime/AudioEngine.h\"\n";
	std::string l_usingNS = "using namespace Waveless;\n\n";

	std::copy(l_APIExport.begin(), l_APIExport.end(), std::back_inserter(TU));
	std::copy(l_math.begin(), l_math.end(), std::back_inserter(TU));
	std::copy(l_waveParser.begin(), l_waveParser.end(), std::back_inserter(TU));
	std::copy(l_audioEngine.begin(), l_audioEngine.end(), std::back_inserter(TU));
	std::copy(l_usingNS.begin(), l_usingNS.end(), std::back_inserter(TU));
}

void WriteFunctionDefinitions(std::vector<char>& TU)
{
	for (auto Desc : m_NodeDescriptors)
	{
		if (Desc->Type == NodeType::FlowFunc || Desc->Type == NodeType::AuxFunc)
		{
			std::string l_sign;
			l_sign += "void ";
			l_sign += Desc->FuncMetadata->Name;
			l_sign += "(";
			for (size_t i = 0; i < Desc->FuncMetadata->ParamsCount; i++)
			{
				ParamMetadata* l_paramMetadata;
				NodeDescriptorManager::GetParamMetadata(int(i) + Desc->FuncMetadata->ParamsIndexOffset, l_paramMetadata);

				l_sign += l_paramMetadata->Type;
				l_sign += " ";
				l_sign += l_paramMetadata->Name;

				if (i < Desc->FuncMetadata->ParamsCount - 1)
				{
					l_sign += ", ";
				}
			}

			l_sign += ")\n";

			std::copy(l_sign.begin(), l_sign.end(), std::back_inserter(TU));

			std::string l_defi = Desc->FuncMetadata->Defi;
			l_defi += "\n\n";

			std::copy(l_defi.begin(), l_defi.end(), std::back_inserter(TU));
		}
	}
}

void WriteConstant(NodeModel* node, std::vector<char> & TU)
{
	std::string l_constDecl;

	for (size_t i = 0; i < node->Desc->FuncMetadata->ParamsCount; i++)
	{
		ParamMetadata* l_paramMetadata;
		NodeDescriptorManager::GetParamMetadata(int(i) + node->Desc->FuncMetadata->ParamsIndexOffset, l_paramMetadata);

		l_constDecl += "\t";

		std::string l_type = l_paramMetadata->Type;
		if (!strcmp(&l_type.back(), "&"))
		{
			l_type = l_type.substr(0, l_type.size() - 1);
		}

		PinModel* l_pin;
		NodeModelManager::GetPinModel(node->OutputPinIndexOffset + (int)i, l_pin);

		l_constDecl += l_type;
		l_constDecl += " ";
		l_constDecl += l_pin->InstanceName;
		l_constDecl += " = ";
		if (l_pin->Desc->Type == PinType::String)
		{
			auto l_string = StringManager::FindString(l_pin->Value);
			l_constDecl += "\"";
			l_constDecl += l_string.value;
			l_constDecl += "\"";
		}
		else if (l_pin->Desc->Type == PinType::Bool)
		{
			bool value = *reinterpret_cast<bool*>(&l_pin->Value);
			l_constDecl += std::to_string(value);
		}
		else if (l_pin->Desc->Type == PinType::Int)
		{
			int32_t value = *reinterpret_cast<int32_t*>(&l_pin->Value);
			l_constDecl += std::to_string(value);
		}
		else if (l_pin->Desc->Type == PinType::Float)
		{
			float value = *reinterpret_cast<float*>(&l_pin->Value);
			l_constDecl += std::to_string(value);
		}
		else
		{
			l_constDecl += std::to_string(l_pin->Value);
		}
		l_constDecl += ";\n";
	}

	std::copy(l_constDecl.begin(), l_constDecl.end(), std::back_inserter(TU));
}

void WriteLocalVar(NodeModel* node, std::vector<char> & TU)
{
	std::string l_localVarDecl;

	int l_offset = 0;
	for (size_t i = 0; i < node->Desc->FuncMetadata->ParamsCount; i++)
	{
		ParamMetadata* l_paramMetadata;
		NodeDescriptorManager::GetParamMetadata(int(i) + node->Desc->FuncMetadata->ParamsIndexOffset, l_paramMetadata);

		if (l_paramMetadata->Kind == PinKind::Input)
		{
			l_offset++;
		}
		if (l_paramMetadata->Kind == PinKind::Output)
		{
			l_localVarDecl += "\t";

			std::string l_type = l_paramMetadata->Type;
			if (!strcmp(&l_type.back(), "&"))
			{
				l_type = l_type.substr(0, l_type.size() - 1);
			}

			PinModel* l_pin;
			NodeModelManager::GetPinModel(node->OutputPinIndexOffset + (int)i - l_offset, l_pin);

			l_localVarDecl += l_type;
			l_localVarDecl += " ";
			l_localVarDecl += l_pin->InstanceName;
			l_localVarDecl += ";\n";
		}
	}

	std::copy(l_localVarDecl.begin(), l_localVarDecl.end(), std::back_inserter(TU));
}

void WriteFunctionInvocation(NodeModel* node, std::vector<char> & TU)
{
	std::vector<LinkModel*>* l_links;
	NodeModelManager::GetAllLinkModels(l_links);

	std::string l_funcInvocation;

	l_funcInvocation += "\t";
	l_funcInvocation += node->Desc->FuncMetadata->Name;
	l_funcInvocation += "(";

	int l_index = 0;
	for (size_t i = 0; i < node->InputPinCount; i++)
	{
		PinModel* l_inputPin;
		NodeModelManager::GetPinModel(node->InputPinIndexOffset + (int)i, l_inputPin);

		if (l_inputPin->Desc->Type != PinType::Flow)
		{
			auto l_linkIt = std::find_if(l_links->begin(), l_links->end(), [&](LinkModel* val) {
				return val->EndPin == l_inputPin;
			});

			if (l_linkIt != l_links->end())
			{
				auto l_link = *l_linkIt;

				if (l_link->StartPin->Owner == m_StartNode)
				{
					l_funcInvocation += "in_";
					l_funcInvocation += l_link->StartPin->Desc->Name;
				}
				else
				{
					l_funcInvocation += l_link->StartPin->InstanceName;
				}
			}
			else // pin not connected, use default value
			{
				l_funcInvocation += std::to_string(l_inputPin->Desc->DefaultValue);
			}

			if (l_index < node->Desc->FuncMetadata->ParamsCount - 1)
			{
				l_funcInvocation += ", ";
			}

			l_index++;
		}
	}

	for (size_t i = 0; i < node->OutputPinCount; i++)
	{
		PinModel* l_outputPin;
		NodeModelManager::GetPinModel(node->OutputPinIndexOffset + (int)i, l_outputPin);

		if (l_outputPin->Desc->Type != PinType::Flow)
		{
			l_funcInvocation += l_outputPin->InstanceName;

			if (l_index < node->Desc->FuncMetadata->ParamsCount - 1)
			{
				l_funcInvocation += ", ";
			}

			l_index++;
		}
	}

	l_funcInvocation += ");\n";

	std::copy(l_funcInvocation.begin(), l_funcInvocation.end(), std::back_inserter(TU));
}

void WriteStartNode(std::vector<char>& TU)
{
	int l_index = 0;
	std::string l_funcInvocation;

	l_funcInvocation += "\t";
	l_funcInvocation += m_StartNode->Desc->FuncMetadata->Name;
	l_funcInvocation += "(";

	for (size_t i = 0; i < m_StartNode->Desc->FuncMetadata->ParamsCount; i++)
	{
		ParamMetadata* l_paramMetadata;
		NodeDescriptorManager::GetParamMetadata(int(i) + m_StartNode->Desc->FuncMetadata->ParamsIndexOffset, l_paramMetadata);

		l_funcInvocation += l_paramMetadata->Name;

		if (l_index < m_StartNode->Desc->FuncMetadata->ParamsCount - 1)
		{
			l_funcInvocation += ", ";
		}

		l_index++;
	}

	l_funcInvocation += ");\n";

	std::copy(l_funcInvocation.begin(), l_funcInvocation.end(), std::back_inserter(TU));
}

void WriteExecutionFlows(std::vector<char>& TU)
{
	WriteStartNode(TU);

	for (auto node : m_NodeOrderInfos)
	{
		if (node.Model != m_StartNode)
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

void WriteScriptSignature(const char * inputFileName, std::vector<char> &l_TU)
{
	int l_index = 0;
	auto l_scriptSign = "WS_CANVAS_API void EventScript_" + IOService::getFileName(inputFileName);
	l_scriptSign += "(";

	for (size_t i = 0; i < m_StartNode->Desc->FuncMetadata->ParamsCount; i++)
	{
		ParamMetadata* l_paramMetadata;
		NodeDescriptorManager::GetParamMetadata(int(i) + m_StartNode->Desc->FuncMetadata->ParamsIndexOffset, l_paramMetadata);

		std::string l_type = l_paramMetadata->Type;

		l_scriptSign += l_type;
		l_scriptSign += " ";
		l_scriptSign += l_paramMetadata->Name;

		if (l_index < m_StartNode->Desc->FuncMetadata->ParamsCount - 1)
		{
			l_scriptSign += ", ";
		}

		l_index++;
	}
	l_scriptSign += ")";
	std::copy(l_scriptSign.begin(), l_scriptSign.end(), std::back_inserter(l_TU));
}

WsResult GenerateCPPFile(const char* inputFileName, const char* outputFileName)
{
	GetNodeOrderInfos();
	GetNodeDescriptors();

	std::vector<char> l_TU;

	WriteIncludes(l_TU);

	WriteFunctionDefinitions(l_TU);

	WriteScriptSignature(inputFileName, l_TU);

	std::string l_scriptBodyBegin = "\n{\n";
	std::copy(l_scriptBodyBegin.begin(), l_scriptBodyBegin.end(), std::back_inserter(l_TU));

	WriteExecutionFlows(l_TU);

	std::string l_scriptBodyEnd = "}";
	std::copy(l_scriptBodyEnd.begin(), l_scriptBodyEnd.end(), std::back_inserter(l_TU));

	auto l_outputPath = "..//..//Asset//Canvas//" + std::string(inputFileName) + ".cpp";

	if (IOService::saveFile(l_outputPath.c_str(), l_TU, IOService::IOMode::Text) != WsResult::Success)
	{
		return WsResult::Fail;
	}
	return WsResult::Success;
}

WsResult BuildDLL()
{
	auto l_cd = IOService::getWorkingDirectory();

	std::string l_command = "mkdir \"" + l_cd + "../../Build-Canvas\"";
	std::system(l_command.c_str());

	l_command = "cmake -DCMAKE_BUILD_TYPE=Debug -G \"Visual Studio 15 Win64\" -S ../../Asset/Canvas -B ../../Build-Canvas";
	std::system(l_command.c_str());

	l_command = "\"\"%VS2017INSTALLDIR%/MSBuild/15.0/Bin/msbuild.exe\" ../../Build-Canvas/Waveless-Canvas.sln\"";

	std::system(l_command.c_str());

	return WsResult::Success;
}

WsResult NodeCompiler::Compile(const char* inputFileName, const char* outputFileName)
{
	if (GenerateCPPFile(inputFileName, outputFileName) != WsResult::Success)
	{
		return WsResult::Fail;
	}
	if (BuildDLL() != WsResult::Success)
	{
		return WsResult::Fail;
	}
	return WsResult::Success;
}