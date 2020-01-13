#pragma once
#include "stdafx.h"
#include "Typedef.h"

namespace Waveless
{
	enum class TimeUnit { Microsecond, Millisecond, Second, Minute, Hour, Day, Month, Year };

	struct Timestamp
	{
		uint32_t Year;
		uint32_t Month;
		uint32_t Day;
		uint32_t Hour;
		uint32_t Minute;
		uint32_t Second;
		uint32_t Millisecond;
		uint32_t Microsecond;
	};

	class Timer
	{
	public:
		static WsResult Initialize();
		static WsResult Terminate();

		static const uint64_t GetCurrentTimeFromEpoch(TimeUnit time_unit);
		static const Timestamp GetCurrentTime(uint32_t time_zone_adjustment);
	};
}
