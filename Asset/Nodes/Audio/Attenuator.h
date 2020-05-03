void Execute(Vector& in_ListenerWorldPosition, Vector& in_EmitterWorldPosition, float& out_gain)
{
	auto l_distanceFromListener = (in_ListenerWorldPosition - in_EmitterWorldPosition).Length();
	if (l_distanceFromListener)
	{
		out_gain = Math::Linear2dBAmp(1.0f / (l_distanceFromListener * l_distanceFromListener));
	}
	else
	{
		out_gain = 0.0f;
	}
}