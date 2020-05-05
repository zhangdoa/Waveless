#include "../../Source/Core/Logger.h"
#include "../../GitSubmodules/cr/cr.h"
#include "PluginName.h"

static unsigned int CR_STATE version = 1;

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation)
{
	if (operation == CR_UNLOAD)
	{
		Logger::Log(LogLevel::Verbose, "Plugin unloaded version ", ctx->version);
		return 0;
	}
	if (operation == CR_LOAD)
	{
		Logger::Log(LogLevel::Verbose, "Plugin loaded version ", ctx->version);
		return 0;
	}

	if (ctx->version < version)
	{
		Logger::Log(LogLevel::Verbose, "A rollback happened due to failure: ", ctx->failure);
	}

	version = ctx->version;

	auto l_inputData = reinterpret_cast<PluginName_InputData*>(ctx->userdata);

	if (l_inputData != nullptr)
	{
		EventScript_PluginName(*l_inputData);
	}

	return 0;
}