#pragma once
#include "stdafx.h"

namespace Waveless
{
	enum class ObjectState
	{
		Created,
		Active,
		Terminated
	};

	struct Object
	{
		uint64_t UUID;
		ObjectState objectState = ObjectState::Created;
	};
}