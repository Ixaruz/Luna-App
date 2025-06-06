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
#include "ui_util.hpp"
#include <stdio.h>
#include <cstring>
#include <math.h>
#include <vector>

namespace dbk {

    namespace {

        constexpr const char *SwitchStandardFont = "switch-standard";
        constexpr const char *SwitchExtFont = "switch-ext";
        constexpr float WindowCornerRadius = 10.0f;
        constexpr float TextAreaCornerRadius = 10.0f;
        constexpr float ButtonCornerRaidus = 6.0f;

        float g_progress = 0;

        std::vector<int>g_image_handle;
        unsigned char *gif = nullptr;
        int g_gif_w, g_gif_h, g_gif_frames;
        static int g_framecounter = 0;

        NVGcolor GetSelectionRGB2(u64 ns) {
            /* Calculate the rgb values for the breathing colour effect. */
            const double t = static_cast<double>(ns) / 1'000'000'000.0d;
            const float d = -0.5 * cos(3.0f*t) + 0.5f;
            const int r2 = 79 + (float)(180 - 79) * (d * 0.7f + 0.3f);
            const int g2 = 55 + (float)(126 - 55) * (d * 0.7f + 0.3f);
            const int b2 = 128 + (float)(230 - 128) * (d * 0.7f + 0.3f);
            return nvgRGB(r2, g2, b2);
        }
    }

    void DrawStar(NVGcontext *vg, float w, float h, float x, float y, float width, float luminosity) {
        /*
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, w, h);
        nvgFillPaint(vg, nvgRadialGradient(vg, x, y, 0, width * 3.0f, nvgRGBA(160, 255, 255, 80 + luminosity * 80), nvgRGBA(0, 0, 0, 0)));
        nvgFill(vg);
        */
        nvgBeginPath(vg);
        nvgEllipse(vg, x, y, width, width * 3.0f);
        nvgEllipse(vg, x, y, width * 3.0f, width);
        nvgFillColor(vg, nvgRGBA(205, 255, 255, 100 + luminosity * 40));
        nvgFill(vg);
    }

    void DrawBackground(NVGcontext *vg, float w, float h, u64 ns) {
        /* Draw the background gradient. */
        const NVGpaint bg_paint = nvgLinearGradient(vg, w / 2.0f, 0, w / 2.0f, h, nvgRGB(26, 34, 109), nvgRGB(84, 121, 196));
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, w, h);
        nvgFillPaint(vg, bg_paint);
        nvgFill(vg);
        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, w, h);
        //the first multiplier of the cosine is the deviation, the + at the end is the "mean"
        float outr = w * (-0.125f * cos(ns / 1'000'000'000.0d) + 0.7f);
        nvgFillPaint(vg, nvgRadialGradient(vg, w / 2.0f, h + (h * 0.1f), 10, outr, nvgRGBA(248, 95, 240, 170), nvgRGBA(0, 0, 0, 0)));
        nvgFill(vg);
    }

    void DrawWindow(NVGcontext *vg, const char *title, float x, float y, float w, float h) {
        /* Draw the window background. */
        const NVGpaint window_bg_paint = nvgLinearGradient(vg, x + w / 2.0f, y, x + w / 2.0f, y + h + h / 4.0f, nvgRGB(255, 255, 255), nvgRGB(216, 210, 247));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, WindowCornerRadius);
        nvgFillPaint(vg, window_bg_paint);
        nvgFill(vg);

        /* Draw the shadow surrounding the window. */
        NVGpaint shadowPaint = nvgBoxGradient(vg, x, y + 2, w, h, WindowCornerRadius * 1.2f, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
        nvgBeginPath(vg);
        nvgRect(vg, x - 10, y - 10, w + 20, h + 30);
        nvgRoundedRect(vg, x, y, w, h, WindowCornerRadius);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);

        /* Setup the font. */
        nvgFontSize(vg, 32.0f);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, nvgRGB(0, 0, 0));

        /* Draw the title. */
        const float tw = nvgTextBounds(vg, 0, 0, title, nullptr, nullptr);
        nvgText(vg, x + w * 0.5f - tw * 0.5f, y + 40.0f, title, nullptr);
    }

    void DrawDumpWindow(NVGcontext* vg, const char* title, float x, float y, float w, float h, float box_x, float box_y, float box_w, float box_h, float prog_x, float prog_y, float prog_w, float prog_h, float progress, u64 ns) 
    {    
        /* Draw the window background. */
        const NVGpaint window_bg_paint = nvgLinearGradient(vg, x + w / 2.0f, y, x + w / 2.0f, y + h + h / 4.0f, nvgRGB(255, 255, 255), nvgRGB(216, 210, 247));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, WindowCornerRadius);
        nvgFillPaint(vg, window_bg_paint);
        /* log box */
        nvgRoundedRect(vg, box_x, box_y, box_w, box_h, TextAreaCornerRadius * 1.2f);
        nvgPathWinding(vg, NVG_HOLE);
        /*progess bar */
        nvgRoundedRect(vg, prog_x, prog_y, prog_w, prog_h, WindowCornerRadius * 2);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFill(vg);

        /* Draw the shadow surrounding the window. */
        NVGpaint shadowPaint = nvgBoxGradient(vg, x, y + 2, w, h, WindowCornerRadius * 2 * 1.2f, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
        nvgBeginPath(vg);
        nvgRect(vg, x - 10, y - 10, w + 20, h + 30);
        nvgRoundedRect(vg, x, y, w, h, WindowCornerRadius);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);

        /* Draw the shadow inside the window. */
        shadowPaint = nvgBoxGradient(vg, box_x, box_y + 2, box_w, box_h, TextAreaCornerRadius, 10, nvgRGBA(0, 0, 0, 0), nvgRGBA(0, 0, 0, 180));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, box_x, box_y, box_w, box_h, TextAreaCornerRadius);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);
        
        /* log box background */
        nvgBeginPath(vg);
        nvgRoundedRect(vg, box_x, box_y, box_w, box_h, TextAreaCornerRadius);
        nvgFillPaint(vg, nvgLinearGradient(vg, box_x + box_w / 2.0f, box_y, box_x + box_w / 2.0f, box_y + box_h, nvgRGBA(255, 255, 255, 220), nvgRGBA(242, 245, 255, 190)));
        nvgFill(vg);

        /* progess bar background */
        nvgBeginPath(vg);
        nvgRoundedRect(vg, prog_x, prog_y, prog_w, prog_h, WindowCornerRadius * 2 * 0.8f);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 220));
        nvgFill(vg);

        /* Setup the font. */
        nvgFontSize(vg, 32.0f);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, nvgRGB(0, 0, 0));

        /* Draw the title. */
        const float tw = nvgTextBounds(vg, 0, 0, title, nullptr, nullptr);
        nvgText(vg, x + w * 0.5f - tw * 0.5f, y + 40.0f, title, nullptr);

        /* Draw the progress bar fill. */
        if (progress > 0.0f) {
            g_progress += (progress - g_progress) * 0.1f;
            NVGpaint progress_fill_paint = nvgLinearGradient(vg, prog_x, prog_y + 0.5f * prog_h, prog_x + prog_w * g_progress, prog_y + 0.5f * prog_h, nvgRGB(46, 64, 139), GetSelectionRGB2(ns));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, prog_x, prog_y, WindowCornerRadius * 2 + (prog_w - WindowCornerRadius * 2) * g_progress, prog_h, WindowCornerRadius * 2);
            nvgFillPaint(vg, progress_fill_paint);
            nvgFill(vg);
        }
    }

    void DrawAlertWindow(NVGcontext* vg, const char* title, float x, float y, float w, float h) {
        /* Draw the window background. */
        const NVGpaint window_bg_paint = nvgLinearGradient(vg, x + w / 2.0f, y, x + w / 2.0f, y + h + h / 4.0f, nvgRGB(255, 255, 255), nvgRGB(247, 210, 245));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, WindowCornerRadius);
        nvgFillPaint(vg, window_bg_paint);
        nvgFill(vg);

        /* Draw the shadow surrounding the window. */
        NVGpaint shadowPaint = nvgBoxGradient(vg, x, y + 2, w, h, WindowCornerRadius * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
        nvgBeginPath(vg);
        nvgRect(vg, x - 10, y - 10, w + 20, h + 30);
        nvgRoundedRect(vg, x, y, w, h, WindowCornerRadius);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);

        /* Setup the font. */
        nvgFontSize(vg, 50.0f);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, nvgRGBA(46, 64, 139, 170));

        /* Draw the title. */
        const float tw = nvgTextBounds(vg, 0, 0, title, nullptr, nullptr);
        nvgText(vg, x + w * 0.5f - tw * 0.5f, y + 45.0f, title, nullptr);
    }

    void DrawButton(NVGcontext *vg, const char *text, float x, float y, float w, float h, ButtonStyle style, u64 ns, float fontsize) {
        /* Fill the background if selected. */
        if (style == ButtonStyle::StandardSelected || style == ButtonStyle::FileSelectSelected) {
            NVGpaint bg_paint = nvgLinearGradient(vg, x, y + h / 2.0f, x + w, y + h / 2.0f, nvgRGB(46, 64, 139), GetSelectionRGB2(ns));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, y, w, h, ButtonCornerRaidus);
            nvgFillPaint(vg, bg_paint);
            nvgFill(vg);
        }

        /* Draw the shadow surrounding the button. */
        if (style == ButtonStyle::Standard || style == ButtonStyle::StandardSelected || style == ButtonStyle::StandardDisabled || style == ButtonStyle::FileSelectSelected) {
            const unsigned char shadow_color = style == ButtonStyle::Standard ? 128 : 64;
            NVGpaint shadow_paint = nvgBoxGradient(vg, x, y, w, h, ButtonCornerRaidus, 5, nvgRGBA(0, 0, 0, shadow_color), nvgRGBA(0, 0, 0, 0));
            nvgBeginPath(vg);
            nvgRect(vg, x - 10, y - 10, w + 20, h + 30);
            nvgRoundedRect(vg, x, y, w, h, ButtonCornerRaidus);
            nvgPathWinding(vg, NVG_HOLE);
            nvgFillPaint(vg, shadow_paint);
            nvgFill(vg);
        }

        /* Setup the font. */
        nvgFontSize(vg, fontsize);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        /* Set the text colour. */
        if (style == ButtonStyle::StandardSelected || style == ButtonStyle::FileSelectSelected) {
            nvgFillColor(vg, nvgRGB(255, 255, 255));
        } else {
            const unsigned char alpha = style == ButtonStyle::StandardDisabled ? 64 : 255;
            nvgFillColor(vg, nvgRGBA(0, 0, 0, alpha));
        }

        /* Draw the button text. */
        const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);

        if (style == ButtonStyle::Standard || style == ButtonStyle::StandardSelected || style == ButtonStyle::StandardDisabled) {
            nvgText(vg, x + w * 0.5f - tw * 0.5f, y + h * 0.5f, text, nullptr);
        } else {
            nvgText(vg, x + 10.0f, y + h * 0.5f, text, nullptr);
        }
    }

    void DrawTextBackground(NVGcontext *vg, float x, float y, float w, float h) {
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, TextAreaCornerRadius);
        nvgFillColor(vg, nvgRGBA(0, 0, 0, 16));
        nvgFill(vg);
    }

    void DrawText(NVGcontext *vg, float x, float y, float w, const char *text) {
        float fontsize = 20.0f;
        nvgFontSize(vg, fontsize);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFillColor(vg, nvgRGB(0, 0, 0));

        const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);
        nvgText(vg, x + w * 0.5f - tw * 0.5f, y, text, nullptr);
    }

    void DrawTextAlignWhite(NVGcontext* vg, float x, float y, float w, const char* text, TextStyle style) {
        float fontsize = 20.0f;
        /*
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y - 3.0f, w, fontsize + 4.0f, ButtonCornerRaidus);
        nvgFillColor(vg, nvgRGBA(0, 0, 0, 16));
        nvgFill(vg);
        */

        nvgFontSize(vg, fontsize);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFillColor(vg, nvgRGB(255, 255, 255));

        const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);

        if (style == TextStyle::CenterAlign) {
            nvgText(vg, x + w * 0.5f - tw * 0.5f, y, text, nullptr);
        }
        else if (style == TextStyle::LeftAlign) {
            nvgText(vg, x + 10.0f, y, text, nullptr);
        }
        else if (style == TextStyle::RightAlign) {
            nvgText(vg, x + w * 0.5f + tw * 0.5f, y, text, nullptr);
        }
    }

    void DrawTextAlign(NVGcontext* vg, float x, float y, float w, const char* text, TextStyle style) {
        float fontsize = 20.0f;
        /*
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y - 3.0f, w, fontsize + 4.0f, ButtonCornerRaidus);
        nvgFillColor(vg, nvgRGBA(0, 0, 0, 16));
        nvgFill(vg);
        */

        nvgFontSize(vg, fontsize);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFillColor(vg, nvgRGB(0, 0, 0));

        const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);

        if (style == TextStyle::CenterAlign) {
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);
            nvgText(vg, x + 10.0f + tw, y, text, nullptr);
        }
        else if (style == TextStyle::LeftAlign) {
            nvgText(vg, x + 10.0f, y, text, nullptr);
        }
        else if (style == TextStyle::RightAlign) {
            nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);
            nvgText(vg, x + 10.0f, y, text, nullptr);
        }
    }

    void DrawHeadline(NVGcontext* vg, float x, float y, float w, const char* text) {
        nvgFontSize(vg, 24.0f);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFillColor(vg, nvgRGB(0, 0, 0));

        const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);
        nvgText(vg, x + w * 0.5f - tw * 0.5f, y, text, nullptr);
    }

    void DrawMeta(NVGcontext* vg, float x, float y, const char *text, TextStyle style) {
        if (style == TextStyle::LeftAlign) {
            nvgFontSize(vg, 24.0f);
            nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgBeginPath(vg);
            const float tw = nvgTextBounds(vg, x, y, text, nullptr, nullptr);
            nvgRect(vg, x - 8.0f, y - 8.0f, tw + 8.0f * 2.0f, 24.0f + 8.0f + 4.0f);
            nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
            nvgFill(vg);
            nvgFillColor(vg, nvgRGB(255, 255, 255));
            nvgText(vg, x, y, text, nullptr);
        }

        if (style == TextStyle::RightAlign) {
            nvgFontSize(vg, 24.0f);
            nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
            nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            nvgBeginPath(vg);
            const float tw = nvgTextBounds(vg, 0, 0, text, nullptr, nullptr);
            nvgRect(vg, x - 8.0f - tw, y - 8.0f, tw + 8.0f * 2.0f, 24.0f + 8.0f + 4.0f);
            nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
            nvgFill(vg);
            nvgFillColor(vg, nvgRGB(255, 255, 255));
            nvgText(vg, x, y, text, nullptr);
        }
    }

    void DrawProgressText(NVGcontext *vg, float x, float y, float progress) {
        char progress_text[32] = {};
        snprintf(progress_text, sizeof(progress_text)-1, "%d%% complete", static_cast<int>(progress * 100.0f));

        nvgFontSize(vg, 24.0f);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, nvgRGB(0, 0, 0));
        nvgText(vg, x, y, progress_text, nullptr);
    }

    void DrawProgressBar(NVGcontext *vg, float x, float y, float w, float h, float progress) {
        /* Draw the progress bar background. */
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, WindowCornerRadius);
        nvgFillColor(vg, nvgRGBA(0, 0, 0, 56));
        nvgFill(vg);

        /* Draw the progress bar fill. */
        if (progress > 0.0f) {
            NVGpaint progress_fill_paint = nvgLinearGradient(vg, x, y + 0.5f * h, (x + w) * progress, y + 0.5f * h, nvgRGB(77, 136, 255), nvgRGB(181, 102, 255));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, y, WindowCornerRadius + (w - WindowCornerRadius) * progress, h, WindowCornerRadius);
            nvgFillPaint(vg, progress_fill_paint);
            nvgFill(vg);
        }
    }

    void DrawScrollbar(NVGcontext* vg, float x, float y, float h) {
        /* Draw the shadow surrounding the window. */
        NVGpaint shadowPaint = nvgBoxGradient(vg, x, y, 15.0f, h, TextAreaCornerRadius * 1.2f, 30, nvgRGBA(0, 0, 0, 20), nvgRGBA(0, 0, 0, 0));
        nvgBeginPath(vg);
        nvgRect(vg, x - 20, y - 20, 15.0f + 40, h + 40);
        nvgRoundedRect(vg, x, y, 15.0f, h, TextAreaCornerRadius);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);
        
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, 15.0f, h, TextAreaCornerRadius);
        nvgFillColor(vg, nvgRGBA(255, 255, 255, 180));
        nvgFill(vg);
    }


    void DrawTextBlock(NVGcontext *vg, const char *text, float x, float y, float w, float h) {
        /* Save state and scissor. */
        nvgSave(vg);
        nvgScissor(vg, x, y, w, h);

        /* Configure the text. */
        nvgFontSize(vg, 18.0f);
        nvgAddFallbackFont(vg, SwitchStandardFont, SwitchExtFont);
        nvgTextLineHeight(vg, 1.3f);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFillColor(vg, nvgRGB(0, 0, 0));

        /* Determine the bounds of the text box. */
        float bounds[4];
        nvgTextBoxBounds(vg, 0, 0, w, text, nullptr, bounds);

        /* Adjust the y to only show the last part of the text that fits. */
        float y_adjustment = 0.0f;
        if (bounds[3] > h) {
            y_adjustment = bounds[3] - h;
        }

        /* Draw the text box and restore state. */
        nvgTextBox(vg, x, y - y_adjustment, w, text, nullptr);
        nvgRestore(vg);
    }

}
