
#include <display.hpp>
#include <imgui.h>
#include <atomic>
#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>

namespace {
	std::atomic<bool> instantiaded{false};
}

namespace {
#define IMGUIGUARD(x) struct x { \
	template <typename... Args>\
	explicit x(Args&&... args) : valid{ImGui::Begin##x(args...)}{}\
	template <typename FuncT>\
	void operator+(FuncT&& f) { if (valid) f();}\
	~x() { if (valid) ImGui::End##x(); }\
	bool valid;\
}

IMGUIGUARD(MenuBar);
IMGUIGUARD(Menu);

#undef IMGUIGUARD
#define Scoped(x) x + [&]()
}

bool display::is_intiantiated() {
	return instantiaded;
}

display::gui::gui()
		: window{sf::VideoMode(1280, 720), "Clonnect"}, clk{}
		, frame_flags{ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar}
{
	if (instantiaded.exchange(true)) {
		throw std::logic_error("Tried to create more than one gui.");
	}

	window.setVerticalSyncEnabled(true);

	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr; // disable .ini saving
}

display::gui::~gui() {
	ImGui::SFML::Shutdown();
	instantiaded = false;
}

bool display::gui::display() {
	if (!is_open()) {
		return false;
	}

	sf::Event ev{};
	while (window.pollEvent(ev)) {
		if (ev.type == sf::Event::Closed) {
			window.close();
			return false;
		}

		ImGui::SFML::ProcessEvent(ev);
	}

	ImGui::SFML::Update(window, clk.restart());

	ImGui::SetNextWindowSize(ImVec2{static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)});
	ImGui::SetNextWindowPos({});
	ImGui::Begin("Main", nullptr, frame_flags);

	update_frame();

	ImGui::End();
	window.clear();
	ImGui::SFML::Render(window);
	window.display();
	return true;
}

void display::gui::update_frame() {

	Scoped(MenuBar()) {
		Scoped(Menu("Options")) {
			Scoped(Menu("Color theme")) {

			};
			if(ImGui::MenuItem("Quit")) {
				ImGui::SFML::Shutdown();
				exit(0);
			}
		};
	};

	static bool inputCustomBases, outputDefaultBases, outputCustomBases;
	if(ImGui::CollapsingHeader("Config", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Input custom bases", &inputCustomBases);
		ImGui::Checkbox("Output default bases", &outputDefaultBases);
		ImGui::Checkbox("Output custom bases", &outputCustomBases);
	}

}
