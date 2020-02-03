#pragma once
#include "NodeTemplate.h"

namespace Waveless
{
	namespace NodeDescriptorManager
	{
		void GenerateNodeDescriptors(const char* nodeTemplateDirectoryPath);
		PinDescriptor* GetPinDescriptor(int pinIndex, PinKind pinKind);
		NodeDescriptor* GetNodeDescriptor(const char* nodeTemplateName);
		ParamMetadata* GetParamMetadata(int paramMetadataIndex);
	};
}