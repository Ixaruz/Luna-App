/*
 * Copyright (c) Adubbz
 * Copyright (c) Ixaruz
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, or any later version, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <nanovg.h>
#include <switch.h>
#include <string>
#include "luna.h"

namespace dbk {

    enum class ButtonStyle {
        Standard,
        StandardSelected,
        StandardDisabled,
        FileSelect,
        FileSelectSelected,
    };

    enum class TextStyle {
        LeftAlign,
        CenterAlign,
        RightAlign,
    };

    void DrawStar(NVGcontext* vg, float w, float h, float x, float y, float width, float luminosity);
    void DrawBackground(NVGcontext* vg, float w, float h, u64 ns);
    void DrawWindow(NVGcontext *vg, const char *title, float x, float y, float w, float h);
    void DrawDumpWindow(NVGcontext* vg, const char* title, float x, float y, float w, float h, float box_x, float box_y, float box_w, float box_h, float prog_x, float prog_y, float prog_w, float prog_h, float progress, u64 ns);
    void DrawAlertWindow(NVGcontext* vg, const char* title, float x, float y, float w, float h);
    void DrawButton(NVGcontext *vg, const char *text, float x, float y, float w, float h, ButtonStyle style, u64 ns);
    void DrawTextBackground(NVGcontext *vg, float x, float y, float w, float h);
    void DrawText(NVGcontext *vg, float x, float y, float w, const char *text);
    void DrawTextAlignWhite(NVGcontext* vg, float x, float y, float w, const char* text, TextStyle style);
    void DrawTextAlign(NVGcontext* vg, float x, float y, float w, const char* text, TextStyle style);
    void DrawHeadline(NVGcontext* vg, float x, float y, float w, const char* text);
    void DrawMeta(NVGcontext* vg, float x, float y, const char* text, TextStyle style);
    void DrawProgressText(NVGcontext *vg, float x, float y, float progress);
    void DrawProgressBar(NVGcontext *vg, float x, float y, float w, float h, float progress);
    void DrawScrollbar(NVGcontext* vg, float x, float y, float h);
    void DrawTextBlock(NVGcontext *vg, const char *text, float x, float y, float w, float h);

}
