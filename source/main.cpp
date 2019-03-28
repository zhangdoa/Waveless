#include "WaveParser.h"

int main()
{
	WaveParser parser;
	auto l_data = parser.loadFile("..//res//test.wav");
	auto l_processed = parser.gain(-4.5, l_data);
	parser.writeFile("..//res//test_Processed.wav", l_processed);
	return 0;
}