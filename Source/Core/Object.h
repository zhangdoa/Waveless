#pragma once
#include "stdafx.h"
#include "Typedef.h"

namespace Waveless
{
	enum class ObjectState
	{
		Created,
		Activated,
		Terminated
	};

	struct Object
	{
		uint64_t UUID = 0;
		ObjectState objectState = ObjectState::Created;
	};
}