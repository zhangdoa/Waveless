#pragma once
#include "NodeTemplate.h"

namespace Waveless
{
	namespace NodeDescriptorManager
	{
		void LoadAllNodeDescriptors(const char* nodeTemplateDirectoryPath);
		const std::vector<NodeDescriptor*>& GetAllNodeDescriptors();
		PinDescriptor* GetPinDescriptor(int pinIndex, PinKind pinKind);
		NodeDescriptor* GetNodeDescriptor(const char* nodeTemplateName);
		ParamMetadata* GetParamMetadata(int paramMetadataIndex);
	};
}