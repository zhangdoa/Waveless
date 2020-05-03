void Execute(std::string& in_FilePath, WavObject& out_WaveObject, uint64_t& out_eventPrototypeID)
{
	out_WaveObject = WaveParser::LoadFile(in_FilePath.c_str());
	out_eventPrototypeID = AudioEngine::AddEventPrototype(out_WaveObject);
}