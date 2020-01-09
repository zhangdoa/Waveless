void Execute(std::string& in_FilePath, WavObject& out_WaveObject, uint64_t& out_eventID)
{
	out_WaveObject = WaveParser::LoadFile(in_FilePath.c_str());
	out_eventID = AudioEngine::AddEventPrototype(out_WaveObject);
}