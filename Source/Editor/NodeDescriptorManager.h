#pragma once
#include "NodeTemplate.h"

namespace Waveless
{
	namespace NodeDescriptorManager
	{
		WsResult LoadAllNodeDescriptors(const char* nodeTemplateDirectoryPath);
		WsResult GetAllNodeDescriptors(std::vector<NodeDescriptor*>*& result);
		WsResult GetPinDescriptor(int pinIndex, PinKind pinKind, PinDescriptor*& result);
		WsResult GetNodeDescriptor(const char* nodeTemplateName, NodeDescriptor*& result);
		WsResult GetParamMetadata(int paramMetadataIndex, ParamMetadata*& result);
	};
}