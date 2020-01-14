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
	std::unordered_map<std::string, NodeDescriptor> m_nodeDescriptors;

	PinType GetPinType(const char * pinType);
}

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

void Waveless::NodeDescriptorManager::GenerateNodeDescriptors(const char * nodeTemplateDirectoryPath)
{
	// @TODO: A string pool shouldn't be managed by this module
	m_stringPool.reserve(8192);

	auto l_filePaths = IOService::getAllFilePaths(nodeTemplateDirectoryPath);

	for (auto& i : l_filePaths)
	{
		if (IOService::getFileExtension(i.c_str()) == ".json")
		{
			NodeDescriptor l_nodeDesc;
			m_stringPool.emplace_back(IOService::getFileName(i.c_str()));
			l_nodeDesc.Name = m_stringPool[m_stringPool.size() - 1].c_str();

			json j;
			JSONParser::loadJsonDataFromDisk((std::string(nodeTemplateDirectoryPath) + i).c_str(), j);

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