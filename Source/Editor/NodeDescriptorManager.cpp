#include "NodeDescriptorManager.h"
#include "../Core/stdafx.h"
#include "../Core/Logger.h"
#include "../Core/String.h"
#include "../IO/IOService.h"
#include "../IO/JSONParser.h"

namespace Waveless::NodeDescriptorManager
{
	std::vector<NodeDescriptor*> m_nodeDescriptors;
	std::vector<PinDescriptor> m_inputPinDescriptors;
	std::vector<PinDescriptor> m_outputPinDescriptors;
	std::vector<ParamMetadata> m_ParamMetadatas;
	std::vector<FunctionMetadata> m_FunctionMetadatas;
	std::unordered_map<std::string, NodeDescriptor*> m_nodeDescriptorsMap;

	PinType GetPinType(const char * pinType);
	NodeDescriptor* LoadNodeDescriptor(const char * nodeDescriptorPath);
	void ParseParams(FunctionMetadata* FuncMetadata, const std::string& params);
	void LoadFunctionDefinitions(NodeDescriptor* nodeDesc);
	void LoadNewNodeDescriptor(const char * nodeDescriptorPath);
}

using namespace Waveless;

Waveless::PinType Waveless::NodeDescriptorManager::GetPinType(const char * pinType)
{
	if (!strcmp(pinType, "Flow"))
	{
		return PinType::Flow;
	}
	else if (!strcmp(pinType, "Bool"))
	{
		return PinType::Bool;
	}
	else if (!strcmp(pinType, "Int"))
	{
		return PinType::Int;
	}
	else if (!strcmp(pinType, "Float"))
	{
		return PinType::Float;
	}
	else if (!strcmp(pinType, "String"))
	{
		return PinType::String;
	}
	else if (!strcmp(pinType, "Vector"))
	{
		return PinType::Vector;
	}
	else if (!strcmp(pinType, "Object"))
	{
		return PinType::Object;
	}
	else
	{
		Logger::Log(LogLevel::Warning, "Unknown pin type: ", pinType);

		return PinType::Flow;
	}
}

NodeDescriptor* Waveless::NodeDescriptorManager::LoadNodeDescriptor(const char * nodeDescriptorPath)
{
	auto l_nodeDesc = new NodeDescriptor();
	l_nodeDesc->RelativePath = StringManager::SpawnString(nodeDescriptorPath).value;
	l_nodeDesc->Name = StringManager::SpawnString(IOService::getFileName(nodeDescriptorPath).c_str()).value;

	json j;
	JSONParser::loadJsonDataFromDisk(nodeDescriptorPath, j);

	int nodeType = j["NodeType"];
	l_nodeDesc->Type = NodeType(nodeType);

	for (auto k : j["Parameters"])
	{
		int pinKind = k["PinKind"];
		std::string pinType = k["PinType"];
		std::string pinName = k["Name"];
		if (!pinName.size())
		{
			pinName = "NoName";
		}

		PinDescriptor pinDesc;
		pinDesc.Kind = PinKind(pinKind);
		pinDesc.Name = StringManager::SpawnString(pinName.c_str()).value;
		pinDesc.Type = GetPinType(pinType.c_str());

		if (pinKind == 0)
		{
			l_nodeDesc->OutputPinCount++;
			m_outputPinDescriptors.emplace_back(pinDesc);
		}
		else
		{
			l_nodeDesc->InputPinCount++;
			m_inputPinDescriptors.emplace_back(pinDesc);
		}
	}

	if (l_nodeDesc->InputPinCount)
	{
		l_nodeDesc->InputPinIndexOffset = (int)m_inputPinDescriptors.size() - l_nodeDesc->InputPinCount;
	}

	if (l_nodeDesc->OutputPinCount)
	{
		l_nodeDesc->OutputPinIndexOffset = (int)m_outputPinDescriptors.size() - l_nodeDesc->OutputPinCount;
	}

	l_nodeDesc->Color[0] = j["Color"]["R"];
	l_nodeDesc->Color[1] = j["Color"]["G"];
	l_nodeDesc->Color[2] = j["Color"]["B"];
	l_nodeDesc->Color[3] = j["Color"]["A"];

	return l_nodeDesc;
}

void Waveless::NodeDescriptorManager::ParseParams(FunctionMetadata* FuncMetadata, const std::string& params)
{
	std::stringstream ss(params);
	std::string s;
	int index = 0;

	while (std::getline(ss, s, ','))
	{
		ParamMetadata p;
		auto l_endPos = s.find("in_");
		auto l_startPos = 0;
		if (l_endPos == std::string::npos)
		{
			l_endPos = s.find("out_");
		}
		if (!s.compare(0, 1, " "))
		{
			l_startPos = 1;
		}
		auto l_type = s.substr(l_startPos, l_endPos - 1 - l_startPos);
		p.Type = StringManager::SpawnString(l_type.c_str()).value;

		auto l_name = s.substr(l_endPos, std::string::npos);
		p.Name = StringManager::SpawnString(l_name.c_str()).value;

		if (l_name.find("in_") != std::string::npos)
		{
			p.Kind = PinKind::Input;
		}
		else
		{
			p.Kind = PinKind::Output;
		}

		m_ParamMetadatas.emplace_back(p);

		index++;
	}

	if (index)
	{
		FuncMetadata->ParamsCount = index;
		FuncMetadata->ParamsIndexOffset = (int)m_ParamMetadatas.size() - index;
	}
}

void Waveless::NodeDescriptorManager::LoadFunctionDefinitions(NodeDescriptor* nodeDesc)
{
	auto l_fileName = std::string(nodeDesc->RelativePath);
	l_fileName = l_fileName.substr(0, l_fileName.rfind("."));
	auto l_filePath = l_fileName + ".h";

	std::vector<char> l_code;

	if (IOService::loadFile(l_filePath.c_str(), l_code, IOService::IOMode::Text) == WsResult::Success)
	{
		std::string l_codeStr = &l_code[0];

		auto l_funcName = l_fileName;
		std::replace(l_funcName.begin(), l_funcName.end(), '/', '_');
		l_funcName = "Execute_" + l_funcName;
		auto l_funcNameCStr = StringManager::SpawnString(l_funcName.c_str()).value;

		FunctionMetadata l_funcMetadata;
		l_funcMetadata.Name = l_funcNameCStr;

		auto l_signEndPos = l_codeStr.find_first_of("\n");
		auto l_funcDefi = l_codeStr.substr(l_signEndPos + 1, std::string::npos);
		l_funcMetadata.Defi = StringManager::SpawnString(l_funcDefi.c_str()).value;

		auto l_params = l_codeStr.substr(13, l_signEndPos - 14);
		ParseParams(&l_funcMetadata, l_params);

		l_codeStr.replace(l_codeStr.begin() + 5, l_codeStr.begin() + 12, l_funcName);
		l_codeStr.append("\n\n");

		m_FunctionMetadatas.emplace_back(l_funcMetadata);
		nodeDesc->FuncMetadata = &m_FunctionMetadatas.back();
	}
}

void Waveless::NodeDescriptorManager::LoadNewNodeDescriptor(const char * nodeDescriptorPath)
{
	auto l_nodeDesc = LoadNodeDescriptor(nodeDescriptorPath);

	LoadFunctionDefinitions(l_nodeDesc);

	m_nodeDescriptors.emplace_back(l_nodeDesc);
	m_nodeDescriptorsMap.emplace(l_nodeDesc->Name, m_nodeDescriptors.back());
}

void Waveless::NodeDescriptorManager::LoadAllNodeDescriptors(const char * nodeTemplateDirectoryPath)
{
	// @TODO: Pool them
	m_nodeDescriptors.reserve(512);
	m_inputPinDescriptors.reserve(2048);
	m_outputPinDescriptors.reserve(2048);
	m_ParamMetadatas.reserve(2048);
	m_FunctionMetadatas.reserve(1024);

	auto l_filePaths = IOService::getAllFilePaths(nodeTemplateDirectoryPath);

	for (auto& i : l_filePaths)
	{
		if (IOService::getFileExtension(i.c_str()) == ".json")
		{
			LoadNewNodeDescriptor((nodeTemplateDirectoryPath + i).c_str());
		}
	}
}

const std::vector<NodeDescriptor*>& Waveless::NodeDescriptorManager::GetAllNodeDescriptors()
{
	return m_nodeDescriptors;
}

Waveless::PinDescriptor* Waveless::NodeDescriptorManager::GetPinDescriptor(int pinIndex, PinKind pinKind)
{
	if (pinKind == PinKind::Input)
	{
		return &m_inputPinDescriptors[pinIndex];
	}
	else
	{
		return &m_outputPinDescriptors[pinIndex];
	}
}

Waveless::NodeDescriptor* Waveless::NodeDescriptorManager::GetNodeDescriptor(const char * nodeTemplateName)
{
	auto l_result = m_nodeDescriptorsMap.find(nodeTemplateName);

	if (l_result != m_nodeDescriptorsMap.end())
	{
		return l_result->second;
	}
	else
	{
		Logger::Log(LogLevel::Error, "Can't find node descriptor!");
		return nullptr;
	}
}

Waveless::ParamMetadata* Waveless::NodeDescriptorManager::GetParamMetadata(int paramMetadataIndex)
{
	return &m_ParamMetadatas[paramMetadataIndex];
}