#include "WaveParser.h"
#include "Math.h"

int main()
{
	WaveParser parser;
	auto l_data = parser.loadFile("..//res//test.wav");
	auto l_processed = parser.gain(-4.5, l_data);
	parser.writeFile("..//res//test_Processed.wav", l_processed);

	Math l_math;
	SignalArray x = { 1.0, 2.0, 3.0, 4.0 };
	FrequencyArray X = l_math.DFT(x);
	SignalArray x_syn = l_math.IDFT(X);

	return 0;
}