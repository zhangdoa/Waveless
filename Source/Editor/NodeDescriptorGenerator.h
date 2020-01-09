#pragma once
#include "NodeTemplate.h"

namespace Waveless
{
	namespace NodeDescriptorGenerator
	{
		void GenerateNodeDescriptors(const char* nodeTemplateDirectoryPath);
		PinDescriptor* GetPinDescriptor(int pinIndex, PinKind pinKind);
		NodeDescriptor* GetNodeDescriptor(const char* nodeTemplateName);
	};
}