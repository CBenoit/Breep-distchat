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

#include <iostream>

#include <IconsFontAwesome5.h>
#include <imgui.h>

#include "display/imgui_helper.hpp"

namespace {
	constexpr const char* DROID_SANS_MONO_FONT_PATH = "resources/fonts/DroidSans/DroidSansMono.ttf";
	constexpr const char* DEFAULT_FONT_PATH = DROID_SANS_MONO_FONT_PATH;
	constexpr const char* FONTAWESOME_FONT_PATH = "resources/fonts/fontawesome-free-5.4.0/" FONT_ICON_FILE_NAME_FAS;
}

ImFont* display::loadFonts(float pixel_size) {
	ImGuiIO& io = ImGui::GetIO();

	ImFont* default_font = io.Fonts->AddFontFromFileTTF(DEFAULT_FONT_PATH, pixel_size);
	if(!default_font) {
		io.Fonts->AddFontDefault();
		std::cerr << "Failed to load font " << DEFAULT_FONT_PATH << " default font is used instead\n";
	}

	static constexpr ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = pixel_size;
	ImFont* font = io.Fonts->AddFontFromFileTTF(FONTAWESOME_FONT_PATH, pixel_size, &icons_config, icons_ranges);

	if(!font) {
		std::cerr << "Failed to load fontawesome (" << FONTAWESOME_FONT_PATH << "): icons disabled\n";
		font = default_font;
	}

	return font;
}
