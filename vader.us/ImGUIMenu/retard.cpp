#include "retard.h"
#include <algorithm>
void Checkbox::Draw(std::string title, bool* value)
{
	using namespace std::chrono;
	auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 10;

	auto balls = *value;

	if (!first_draw)
	{
		anim_step = balls ? 200.f : 0.f;
		last_step = ms;
		first_draw = true;
	}
	
	ImGui::CheckBoxAnimated((title).c_str(), value, anim_step);


	auto delta = ms - last_step;
	auto mod = 5 * delta; //9 / (1000 / 60) = 0.54
	anim_step += balls ? mod : -mod;
	last_step = ms;

	if (anim_step <= 0) {
		anim_step = 0;
		last_step = ms;
	}
	else if (anim_step >= 200) {
		anim_step = 200;
		last_step = ms;
	}
}


