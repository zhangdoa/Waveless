void Execute(bool in_Start, bool in_Stop, uint64_t in_eventPrototypeID, float in_Gain, float in_LPFCutOff, float in_HPFCutOff, uint64_t& out_eventInstanceID)
{
	if (in_Start)
	{
		out_eventInstanceID = AudioEngine::Trigger(in_eventPrototypeID);
		if (in_LPFCutOff != 0.0f)
		{
			AudioEngine::ApplyLPF(out_eventInstanceID, in_LPFCutOff);
		}
		if (in_HPFCutOff != 0.0f)
		{
			AudioEngine::ApplyHPF(out_eventInstanceID, in_HPFCutOff);
		}
	}
}