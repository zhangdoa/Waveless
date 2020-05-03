void Execute(bool& in_Start, bool& in_Stop, uint64_t& in_eventPrototypeID, uint64_t& out_eventInstanceID)
{
	if (in_Start)
	{
		out_eventInstanceID = AudioEngine::Trigger(in_eventPrototypeID);
	}
}