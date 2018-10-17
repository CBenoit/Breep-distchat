/*************************************************************************************
 * MIT License                                                                       *
 *                                                                                   *
 * Copyright (c) 2018 TiWinDeTea                                                     *
 *                                                                                   *
 * Permission is hereby granted, free of charge, to any person obtaining a copy      *
 * of this software and associated documentation files (the "Software"), to deal     *
 * in the Software without restriction, including without limitation the rights      *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell         *
 * copies of the Software, and to permit persons to whom the Software is             *
 * furnished to do so, subject to the following conditions:                          *
 *                                                                                   *
 * The above copyright notice and this permission notice shall be included in all    *
 * copies or substantial portions of the Software.                                   *
 *                                                                                   *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR        *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,          *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE       *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER            *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,     *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE     *
 * SOFTWARE.                                                                         *
 *                                                                                   *
 *************************************************************************************/

#include <imgui/imgui.h>
#include <imgui_impl_glfw_gl3.hpp>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <gui_context.hpp>
#include <chat_window.hpp>

gui::chat_window::chat_window() noexcept
        : input_buf(), scroll_to_bottom(), items() {
    memset(input_buf, 0, sizeof(input_buf));
    clear_log();
    add_log("Welcome to SysDist-chat!");
}

void gui::chat_window::clear_log() {
    items.clear();
    scroll_to_bottom = true;
}

void gui::chat_window::add_log(const std::string &str) {
    items.push_back(str);
    scroll_to_bottom = true;
}

void gui::chat_window::draw(const char *title, bool *p_open) {
    ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_MenuBar
            | ImGuiWindowFlags_NoMove
            //| ImGuiWindowFlags_NoResize
            ;

    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, p_open, window_flags)) {
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("This example implements a console with basic coloring, completion and history."
                       "A more elaborate implementation may want to store entries along with extra data"
                       "such as timestamp, emitter, etc.");

    if (ImGui::SmallButton("Add Dummy Text")) {
        add_log("%d some text" + std::to_string(items.size()));
        add_log("some more text");
        add_log("display very important message here!");
    }
    ImGui::SameLine();

    if (ImGui::SmallButton("Add Dummy Error")) {
        add_log("[error] something went wrong");
    }
    ImGui::SameLine();

    if (ImGui::SmallButton("Clear")) { clear_log(); }
    ImGui::SameLine();

    bool copy_to_clipboard = ImGui::SmallButton("Copy");
    ImGui::SameLine();

    if (ImGui::SmallButton("Scroll to bottom")) {
        scroll_to_bottom = true;
    }

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    static ImGuiTextFilter filter;
    filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
    ImGui::PopStyleVar();
    ImGui::Separator();

    const float footer_height_to_reserve =
            ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
                      ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::Selectable("Clear")) clear_log();
        ImGui::EndPopup();
    }

    // Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
    // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
    // You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
    // To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
    //     ImGuiListClipper clipper(Items.Size);
    //     while (clipper.Step())
    //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    // However take note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
    // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
    // and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
    // If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
    if (copy_to_clipboard)
        ImGui::LogToClipboard();
    ImVec4 col_default_text = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    for (auto i = 0u; i < items.size(); i++) {
        const char *item = items[i].data();
        if (!filter.PassFilter(item))
            continue;
        ImVec4 col = col_default_text;
        if (strstr(item, "[error]")) col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
        else if (strncmp(item, "# ", 2) == 0) col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(item);
        ImGui::PopStyleColor();
    }
    if (copy_to_clipboard)
        ImGui::LogFinish();
    if (scroll_to_bottom)
        ImGui::SetScrollHere();
    scroll_to_bottom = false;
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    // input area
    bool reclaim_focus = false;
    if (ImGui::InputText("Input", input_buf, IM_ARRAYSIZE(input_buf), ImGuiInputTextFlags_EnterReturnsTrue, nullptr, (void *) this)) {
        char *input_end = input_buf + strlen(input_buf);
        while (input_end > input_buf && input_end[-1] == ' ') { input_end--; }
        *input_end = '\n';
        if (input_buf[0])
            add_log(std::string(input_buf));
        strcpy(input_buf, "\0");
        reclaim_focus = true;
    }

    ImGui::SetItemDefaultFocus();
    if (reclaim_focus) {
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
    }

    ImGui::End();
}
