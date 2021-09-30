#include <imgui.h>
#include <chrono>
#include <string>

class Checkbox 
{
public:
	void Draw(std::string title, bool* value);
protected:
	int anim_step;
	int last_step;
	bool first_draw;
};