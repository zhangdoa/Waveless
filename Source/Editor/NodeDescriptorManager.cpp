#include "NodeDescriptorManager.h"
#include "../Core/stdafx.h"
#include "../Core/Logger.h"
#include "../IO/IOService.h"
#include "../IO/JSONParser.h"

namespace Waveless::NodeDescriptorManager
{
	std::vector<std::string> m_stringPool;
	std::vector<PinDescriptor> m_inputPinDescriptors;
	std::vector<PinDescriptor> m_outputPinDescriptors;
	std::vector<ParamMetadata> m_ParamMetadatas;
	std::vector<FunctionMetadata> m_FunctionMetadatas;
	std::unordered_map<std::string, NodeDescriptor> m_nodeDescriptors;

	PinType GetPinType(const char * pinType);
	void ParseParams(FunctionMetadata* FuncMetadata, const std::string& params);
	void LoadFunctionDefinitions(NodeDescriptor* nodeDesc);
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

void Waveless::NodeDescriptorManager::ParseParams(FunctionMetadata* FuncMetadata, const std::string& params)
{
	std::stringstream ss(params);
	std::string s;
	int index = 0;

	while (std::getline(ss, s, ','))
	{
		ParamMetadata p;
		auto l_spacePos = s.find_last_of(" ");
		auto l_type = s.substr(0, l_spacePos);
		l_type.erase(std::remove_if(l_type.begin(), l_type.end(), isspace), l_type.end());

		m_stringPool.emplace_back(l_type);
		p.Type = m_stringPool.back().c_str();

		auto l_name = s.substr(l_spacePos + 1, std::string::npos);
		m_stringPool.emplace_back(l_name);
		p.Name = m_stringPool.back().c_str();

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
	l_fileName = l_fileName.substr(0, l_fileName.find("."));
	auto l_filePath = "..//..//Asset//Nodes//" + l_fileName + ".h";

	std::vector<char> l_code;

	if (IOService::loadFile(l_filePath.c_str(), l_code, IOService::IOMode::Text) == WsResult::Success)
	{
		std::string l_codeStr = &l_code[0];
		auto l_funcName = l_fileName;
		std::replace(l_funcName.begin(), l_funcName.end(), '/', '_');
		l_funcName = "Execute_" + l_funcName;
		m_stringPool.emplace_back(l_funcName);
		FunctionMetadata l_funcMetadata;

		l_funcMetadata.Name = m_stringPool.back().c_str();

		auto l_signEndPos = l_codeStr.find_first_of("\n");
		auto l_funcDefi = l_codeStr.substr(l_signEndPos + 1, std::string::npos);
		m_stringPool.emplace_back(l_funcDefi);
		l_funcMetadata.Defi = m_stringPool.back().c_str();

		auto l_params = l_codeStr.substr(13, l_signEndPos - 14);
		ParseParams(&l_funcMetadata, l_params);

		l_codeStr.replace(l_codeStr.begin() + 5, l_codeStr.begin() + 12, l_funcName);
		l_codeStr.append("\n\n");

		m_FunctionMetadatas.emplace_back(l_funcMetadata);
		nodeDesc->FuncMetadata = &m_FunctionMetadatas.back();
	}
}

void Waveless::NodeDescriptorManager::GenerateNodeDescriptors(const char * nodeTemplateDirectoryPath)
{
	// @TODO: A string pool shouldn't be managed by this module
	m_stringPool.reserve(8192);

	// @TODO: Pool them
	m_ParamMetadatas.reserve(8192);
	m_FunctionMetadatas.reserve(8192);

	auto l_filePaths = IOService::getAllFilePaths(nodeTemplateDirectoryPath);

	for (auto& i : l_filePaths)
	{
		if (IOService::getFileExtension(i.c_str()) == ".json")
		{
			NodeDescriptor l_nodeDesc;
			m_stringPool.emplace_back(i);
			l_nodeDesc.RelativePath = m_stringPool.back().c_str();
			m_stringPool.emplace_back(IOService::getFileName(i.c_str()));
			l_nodeDesc.Name = m_stringPool.back().c_str();

			json j;
			JSONParser::loadJsonDataFromDisk((std::string(nodeTemplateDirectoryPath) + i).c_str(), j);

			int nodeType = j["NodeType"];
			l_nodeDesc.Type = NodeType(nodeType);

			for (auto k : j["Parameters"])
			{
				int pinKind = k["PinKind"];
				std::string pinType = k["PinType"];
				std::string pinName = k["Name"];

				PinDescriptor pinDesc;
				pinDesc.Kind = PinKind(pinKind);
				m_stringPool.emplace_back(pinName);
				pinDesc.Name = m_stringPool.back().c_str();
				pinDesc.Type = GetPinType(pinType.c_str());

				if (pinKind == 0)
				{
					l_nodeDesc.OutputPinCount++;
					m_outputPinDescriptors.emplace_back(pinDesc);
				}
				else
				{
					l_nodeDesc.InputPinCount++;
					m_inputPinDescriptors.emplace_back(pinDesc);
				}
			}

			if (l_nodeDesc.InputPinCount)
			{
				l_nodeDesc.InputPinIndexOffset = (int)m_inputPinDescriptors.size() - l_nodeDesc.InputPinCount;
			}

			if (l_nodeDesc.OutputPinCount)
			{
				l_nodeDesc.OutputPinIndexOffset = (int)m_outputPinDescriptors.size() - l_nodeDesc.OutputPinCount;
			}

			l_nodeDesc.Color[0] = j["Color"]["R"];
			l_nodeDesc.Color[1] = j["Color"]["G"];
			l_nodeDesc.Color[2] = j["Color"]["B"];
			l_nodeDesc.Color[3] = j["Color"]["A"];

			LoadFunctionDefinitions(&l_nodeDesc);

			m_nodeDescriptors.emplace(l_nodeDesc.Name, l_nodeDesc);
		}
	}
}

Waveless::PinDescriptor * Waveless::NodeDescriptorManager::GetPinDescriptor(int pinIndex, PinKind pinKind)
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

Waveless::NodeDescriptor * Waveless::NodeDescriptorManager::GetNodeDescriptor(const char * nodeTemplateName)
{
	auto l_result = m_nodeDescriptors.find(nodeTemplateName);

	if (l_result != m_nodeDescriptors.end())
	{
		return &l_result->second;
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