#include "imgui.h"

namespace c { // colors

	inline ImVec4 general_color = ImColor(118, 130, 255, 255);

	namespace bg {

		inline ImVec4 background_ = ImColor(5, 5, 5, 210);
		inline ImVec4 circle = ImColor(100, 100, 100, 110);

		inline ImVec4 background = ImColor(17, 17, 17, 255);
		inline ImVec4 background_shadow = ImColor(17, 17, 17, 0);

		inline ImVec4 border = ImColor(29, 29, 29, 255);

		inline ImVec4 text = ImColor(64, 64, 64, 255);
	}

	namespace tabs {

		inline ImVec4 background_active = ImColor(27, 27, 28, 255);
		inline ImVec4 background_inactive = ImColor(17, 17, 17, 0);

		inline ImVec4 text_hov = ImColor(118, 118, 118, 255);
		inline ImVec4 text = ImColor(64, 64, 64, 255);
	}

	namespace checkbox {

		inline ImVec4 hint_text = ImColor(64, 64, 64, 255);

		inline ImVec4 circle_inactive = ImColor(65, 65, 65, 255);

		inline ImVec4 background = ImColor(36, 36, 37, 255);

	}

	namespace slider {

		inline ImVec4 background = ImColor(36, 36, 37, 255);

	}

	namespace combo {

		inline ImVec4 text = ImColor(255, 255, 255, 255);
		inline ImVec4 hint_text = ImColor(64, 64, 64, 255);

		inline ImVec4 background = ImColor(36, 36, 37, 255);
		inline ImVec4 background_selectable = ImColor(26, 26, 27, 255);

	}

	namespace selectable {

		inline ImVec4 hint_text = ImColor(150, 150, 150, 255);
		inline ImVec4 line = ImColor(150, 150, 150, 0);
	}

	namespace keybind {

		inline ImVec4 text = ImColor(255, 255, 255, 255);
		inline ImVec4 hint_text = ImColor(64, 64, 64, 255);

		inline ImVec4 background = ImColor(36, 36, 37, 255);

	}

	namespace picker {

		inline ImVec4 text = ImColor(255, 255, 255, 255);
		inline ImVec4 hint_text = ImColor(64, 64, 64, 255);

		inline ImVec4 background = ImColor(26, 26, 27, 255);
	}

	namespace button {

		inline ImVec4 background = ImColor(36, 36, 37, 255);
	}

	namespace input {

		inline ImVec4 text = ImColor(255, 255, 255, 255);
		inline ImVec4 hint_text = ImColor(64, 64, 64, 255);
		inline ImVec4 text_selected = ImColor(20, 20, 20, 100);

		inline ImVec4 background = ImColor(36, 36, 37, 255);
	}
}

namespace s { // settings

	namespace bg {

		inline ImVec2 size = ImVec2(1024, 620);
		inline float rounding = 0.f;

	}

	namespace tabs {

		inline ImVec2 size = ImVec2(1024, 620);
		inline float rounding = 0.f;

	}

}