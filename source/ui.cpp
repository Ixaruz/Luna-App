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
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <limits>
#include <random>
//#include <math.h>
#include <dirent.h>
#include "ui.hpp"
#include "dump.hpp"
#include "assert.hpp"
#include "templatecheck.hpp"

namespace dbk {

    namespace {

        static constexpr float ImageInset = 40.0f;
        static constexpr float ImageSize = 255.0f;

        /* Insets of content within windows. */
        static constexpr float HorizontalInset       = 20.0f;
        static constexpr float BottomInset           = 20.0f;

        /* Insets of content within text areas. */
        static constexpr float TextHorizontalInset   = 8.0f;
        static constexpr float TextVerticalInset     = 8.0f;

        static constexpr float ButtonHeight          = 60.0f;
        static constexpr float ButtonHorizontalGap   = 10.0f;

        static constexpr float VerticalGap           = 10.0f;

        u32 g_screen_width;
        u32 g_screen_height;

        static u64 tid = 0, bid = 0, pid = 0;
        static bool isSupportedVersion = false;

        static u64 g_last_ns = 0;
        static u64 g_frame_counter = 0;
        static u64 g_last_ft = 0;
        static u64 g_last_ft2 = 0;
        static u64 g_last_ft3 = 0;
        static u64 g_last_touch = 0;

        const int g_starwidthmax = 7.0f;
        const int g_starwidthmin = 2.0f;
        const int g_starcount = 30;
        float   g_starsx[g_starcount];
        float   g_starsy[g_starcount];
        float   g_starsw[g_starcount];
        float   g_starmarginwidth;
        float   g_starmarginheight;

        std::shared_ptr<Menu> g_current_menu;
        std::shared_ptr<DumpingMenu> g_dumping_menu;

        static size_t g_maxloglength = 50;

        bool g_initialized = false;
        bool g_exit_requested = false;

        bool g_editinglog = false;

        PadState g_pad;

        u32 g_prev_touch_count = -1;
        HidTouchScreenState g_start_touch;
        bool g_started_touching = false;
        bool g_tapping = false;
        bool g_touches_moving = false;
        bool g_finished_touching = false;

        std::thread dump;

        constexpr u32 MaxTapMovement = 5;

        void UpdateInput() {
            /* Scan for input and update touch state. */
            padUpdate(&g_pad);
            HidTouchScreenState current_touch;
            hidGetTouchScreenStates(&current_touch, 1);
            const u32 touch_count = current_touch.count;

            if (g_prev_touch_count == 0 && touch_count > 0) {
                hidGetTouchScreenStates(&g_start_touch, 1);
                g_started_touching = true;
                g_tapping = true;
            } else {
                g_started_touching = false;
            }

            if (g_prev_touch_count > 0 && touch_count == 0) {
                g_finished_touching = true;
                g_tapping = false;
            } else {
                g_finished_touching = false;
            }

            /* Check if currently moving. */
            if (g_prev_touch_count > 0 && touch_count > 0) {
                if ((abs((float)current_touch.touches[0].x - g_start_touch.touches[0].x) > MaxTapMovement || abs((float)current_touch.touches[0].y - g_start_touch.touches[0].y) > MaxTapMovement)) {
                    g_touches_moving = true;
                    g_last_touch = g_last_ns;
                    g_tapping = false;
                } else {
                    g_touches_moving = false;
                }
            } else {
                g_touches_moving = false;
            }

            /* Update the previous touch count. */
            g_prev_touch_count = current_touch.count;
        }

        void ChangeMenu(std::shared_ptr<Menu> menu) {
            g_current_menu = menu;
        }

        void ReturnToPreviousMenu() {
            /* Go to the previous menu if there is one. */
            if (g_current_menu->GetPrevMenu() != nullptr) {
                g_current_menu = g_current_menu->GetPrevMenu();
            }
        }


    }

    void Menu::AddButton(u32 id, const char *text, float x, float y, float w, float h) {
        DBK_ABORT_UNLESS(id < MaxButtons);
        Button button = {
            .id = id,
            .selected = false,
            .enabled = true,
            .x = x,
            .y = y,
            .w = w,
            .h = h,
        };

        strncpy(button.text, text, sizeof(button.text)-1);
        m_buttons[id] = button;
    }

    void Menu::SetButtonSelected(u32 id, bool selected) {
        DBK_ABORT_UNLESS(id < MaxButtons);
        auto &button = m_buttons[id];

        if (button) {
            button->selected = selected;
        }
    }

    void Menu::DeselectAllButtons() {
        for (auto &button : m_buttons) {
            /* Ensure button is present. */
            if (!button) {
                continue;
            }
            button->selected = false;
        }
    }

    void Menu::SetButtonEnabled(u32 id, bool enabled) {
        DBK_ABORT_UNLESS(id < MaxButtons);
        auto &button = m_buttons[id];
        button->enabled = enabled;
    }

    Button *Menu::GetButton(u32 id) {
        DBK_ABORT_UNLESS(id < MaxButtons);
        return !m_buttons[id] ? nullptr : &(*m_buttons[id]);
    }

    Button *Menu::GetSelectedButton() {
        for (auto &button : m_buttons) {
            if (button && button->enabled && button->selected) {
                return &(*button);
            }
        }

        return nullptr;
    }

    Button *Menu::GetClosestButtonToSelection(Direction direction) {
        const Button *selected_button = this->GetSelectedButton();

        if (selected_button == nullptr || direction == Direction::Invalid) {
            return nullptr;
        }

        Button *closest_button = nullptr;
        float closest_distance = 0.0f;

        for (auto &button : m_buttons) {
            /* Skip absent button. */
            if (!button || !button->enabled) {
                continue;
            }

            /* Skip buttons that are in the wrong direction. */
            if ((direction == Direction::Down && button->y <= selected_button->y)  ||
                (direction == Direction::Up && button->y >= selected_button->y)    ||
                (direction == Direction::Right && button->x <= selected_button->x) ||
                (direction == Direction::Left && button->x >= selected_button->x)) {
                continue;
            }

            const float x_dist = button->x - selected_button->x;
            const float y_dist = button->y - selected_button->y;
            const float sq_dist = x_dist * x_dist + y_dist * y_dist;

            /* If we don't already have a closest button, set it. */
            if (closest_button == nullptr) {
                closest_button = &(*button);
                closest_distance = sq_dist;
                continue;
            }

            /* Update the closest button if this one is closer. */
            if (sq_dist < closest_distance) {
                closest_button = &(*button);
                closest_distance = sq_dist;
            }
        }

        return closest_button;
    }

    Button *Menu::GetTouchedButton() {
        HidTouchScreenState current_touch;
        hidGetTouchScreenStates(&current_touch, 1);
        const u32 touch_count = current_touch.count;

        for (u32 i = 0; i < touch_count && g_started_touching; i++) {
            for (auto &button : m_buttons) {
                if (button && button->enabled && button->IsPositionInBounds(current_touch.touches[i].x, current_touch.touches[i].y)) {
                    return &(*button);
                }
            }
        }

        return nullptr;
    }

    Button *Menu::GetActivatedButton() {
        Button *selected_button = this->GetSelectedButton();

        if (selected_button == nullptr) {
            return nullptr;
        }

        const u64 k_down = padGetButtonsDown(&g_pad);

        if (k_down & HidNpadButton_A || this->GetTouchedButton() == selected_button) {
            return selected_button;
        }

        return nullptr;
    }

    void Menu::UpdateButtons() {
        const u64 k_down = padGetButtonsDown(&g_pad);
        Direction direction = Direction::Invalid;

        if (k_down & HidNpadButton_AnyDown) {
            direction = Direction::Down;
        } else if (k_down & HidNpadButton_AnyUp) {
            direction = Direction::Up;
        } else if (k_down & HidNpadButton_AnyLeft) {
            direction = Direction::Left;
        } else if (k_down & HidNpadButton_AnyRight) {
            direction = Direction::Right;
        }

        /* Select the closest button. */
        if (const Button *closest_button = this->GetClosestButtonToSelection(direction); closest_button != nullptr) {
            this->DeselectAllButtons();
            this->SetButtonSelected(closest_button->id, true);
        }

        /* Select the touched button. */
        if (const Button *touched_button = this->GetTouchedButton(); touched_button != nullptr) {
            this->DeselectAllButtons();
            this->SetButtonSelected(touched_button->id, true);
        }
    }

    void Menu::DrawButtons(NVGcontext *vg, u64 ns) {
        for (auto &button : m_buttons) {
            /* Ensure button is present. */
            if (!button) {
                continue;
            }

            /* Set the button style. */
            auto style = ButtonStyle::StandardDisabled;
            if (button->enabled) {
                style = button->selected ? ButtonStyle::StandardSelected : ButtonStyle::Standard;
            }
            float fontsize = 20.0f;
            if ((strlen(button->text) * fontsize/1.25f) > button->w) fontsize = button->w * 1.25f / strlen(button->text);
            DrawButton(vg, button->text, button->x, button->y, button->w, button->h, style, ns, fontsize);
        }
    }

    void Menu::LogText(const char *format, ...) {
        /* Create a temporary string. */
        char tmp[0x100];
        va_list args;
        va_start(args, format);
        vsnprintf(tmp, sizeof(tmp), format, args);
        va_end(args);

        /* Append the text to the log buffer. */
        strncat(m_log_buffer, tmp, sizeof(m_log_buffer)-1);
    }

    std::shared_ptr<Menu> Menu::GetPrevMenu() {
        return m_prev_menu;
    }

    AlertMenu::AlertMenu(std::shared_ptr<Menu> prev_menu, const char* text, const char* subtext, const char* format, u64 rc) : Menu(prev_menu), m_text{}, m_subtext{}, m_result_text{}, m_format_text{}, m_rc(rc) {
        /* Copy the input text. */
        strncpy(m_text, text, sizeof(m_text)-1);
        strncpy(m_subtext, subtext, sizeof(m_subtext)-1);

        /* Copy result text if there is a result. */
        if (R_FAILED(rc)) {
            snprintf(m_result_text, sizeof(m_result_text), format, rc);
        }
    }

    void AlertMenu::Draw(NVGcontext *vg, u64 ns) {
        const float window_height = WindowHeight + (R_FAILED(m_rc) ? SubTextHeight : 0.0f);
        const float x = g_screen_width / 2.0f - WindowWidth / 2.0f;
        const float y = g_screen_height / 2.0f - window_height / 2.0f;

        if (strlen(m_text) > 5) DrawWindow(vg, m_text, x, y, WindowWidth, window_height);
        else DrawAlertWindow(vg, m_text, x, y, WindowWidth, window_height);
        DrawHeadline(vg, x + HorizontalInset, y + TitleGap, WindowWidth - HorizontalInset * 2.0f, m_subtext);

        /* Draw the result if there is one. */
        if (R_FAILED(m_rc)) {
            DrawHeadline(vg, x + HorizontalInset, y + TitleGap + SubTextHeight, WindowWidth - HorizontalInset * 2.0f, m_result_text);
        }

        this->DrawButtons(vg, ns);
    }

    ErrorMenu::ErrorMenu(const char *text, const char *subtext, const char *format, u64 rc) : AlertMenu(nullptr, text, subtext, format, rc)  {
        const float window_height = WindowHeight + (R_FAILED(m_rc) ? SubTextHeight : 0.0f);
        const float x = g_screen_width / 2.0f - WindowWidth / 2.0f;
        const float y = g_screen_height / 2.0f - window_height / 2.0f;
        const float button_y = y + TitleGap + SubTextHeight + VerticalGap * 2.0f + (R_FAILED(m_rc) ? SubTextHeight : 0.0f);
        const float button_width = (WindowWidth - HorizontalInset * 2.0f) / 2.0f - ButtonHorizontalGap;

        /* Add buttons. */
        this->AddButton(ExitButtonId, "Exit", x + WindowWidth / 2 - button_width / 2, button_y, button_width, ButtonHeight);
        this->SetButtonSelected(ExitButtonId, true);
    }

    void ErrorMenu::Update(u64 ns) {
        u64 k_down = padGetButtonsDown(&g_pad);

        /* Go back if B is pressed. */
        if (k_down & HidNpadButton_B) {
            g_exit_requested = true;
            return;
        }

        /* Take action if a button has been activated. */
        if (const Button *activated_button = this->GetActivatedButton(); activated_button != nullptr) {
            switch (activated_button->id) {
                case ExitButtonId:
                    g_exit_requested = true;
                    break;
            }
        }

        this->UpdateButtons();

        /* Fallback on selecting the exfat button. */
        if (const Button *selected_button = this->GetSelectedButton(); k_down && selected_button == nullptr) {
            this->SetButtonSelected(ExitButtonId, true);
        }
    }

    WarningMenu::WarningMenu(std::shared_ptr<Menu> prev_menu, std::shared_ptr<Menu> next_menu, const char *text, const char *subtext, const char *format, u64 rc) : AlertMenu(prev_menu, text, subtext, format, rc), m_next_menu(next_menu) {
        const float window_height = WindowHeight + (R_FAILED(m_rc) ? SubTextHeight : 0.0f);
        const float x = g_screen_width / 2.0f - WindowWidth / 2.0f;
        const float y = g_screen_height / 2.0f - window_height / 2.0f;

        const float button_y = y + TitleGap + SubTextHeight + VerticalGap * 2.0f + (R_FAILED(m_rc) ? SubTextHeight : 0.0f);
        const float button_width = (WindowWidth - HorizontalInset * 2.0f) / 2.0f - ButtonHorizontalGap;
        this->AddButton(BackButtonId, "Back", x + HorizontalInset, button_y, button_width, ButtonHeight);
        this->AddButton(ContinueButtonId, "Continue", x + HorizontalInset + button_width + ButtonHorizontalGap, button_y, button_width, ButtonHeight);
        this->SetButtonSelected(ContinueButtonId, true);
    }

    void WarningMenu::Update(u64 ns) {
        u64 k_down = padGetButtonsDown(&g_pad);

        /* Go back if B is pressed. */
        if (k_down & HidNpadButton_B) {
            ReturnToPreviousMenu();
            return;
        }

        /* Take action if a button has been activated. */
        if (const Button *activated_button = this->GetActivatedButton(); activated_button != nullptr) {
            switch (activated_button->id) {
                case BackButtonId:
                    ReturnToPreviousMenu();
                    return;
                case ContinueButtonId:
                    ChangeMenu(m_next_menu);
                    return;
            }
        }

        this->UpdateButtons();

        /* Fallback on selecting the exfat button. */
        if (const Button *selected_button = this->GetSelectedButton(); k_down && selected_button == nullptr) {
            this->SetButtonSelected(ContinueButtonId, true);
        }
    }

    void ErrorOpenACNH(u64 *tid) {
        const char* ret = "Please open ACNH!";
#if DEBUG
        const char* formatter = "current TID: 0x%016lX";
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", ret, formatter, *tid));
#else
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", ret));
#endif
    }

    void ErrorWrongBID(u64 *bid) {
        const char* ret = "Please update ACNH to the latest version!";
#if DEBUG
        const char* formatter = "current BID: 0x%016lX";
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", ret, formatter, *bid));
#else
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", ret));
#endif
    }

    void ErrorSimpCheck() {
        ChangeMenu(std::make_shared<ErrorMenu>("\uE00A", "No island was found.", "Please try again?", 1));
    }

    void ErrorNoTemplate() {
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", "No valid template was found."));
    }

    void ErrorMissingFiles() {
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", "You seem to be missing template files."));
    }

    void ErrorWrongRevision(std::string info) {
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", "Wrong template-save revision.", info.c_str(), 1));
    }

    void ErrorNotEnoughPlayers(std::string info) {
        ChangeMenu(std::make_shared<ErrorMenu>("\uE150", "Template is lacking:", std::string("Villager" + info).c_str(), 1));
    }

    void MainMenu::gotoNextMenu() {
#if !DEBUG_UI

        u32 dreamstrval;
        u16 IsDreamingBed = 0;

        //[[[[main+4BAEF28]+10]+130]+10]
        u64 mainAddr = util::FollowPointerMain(VersionPointerOffset[versionindex], 0x10, 0x130, 0x10, 0xFFFFFFFFFFFFFFFF);
        
        Check chkres = CheckTemplateFiles(std::string(LUNA_TEMPLATE_DIR), mainAddr);
        if (chkres.check_result != CheckResult::Success) {
            if (chkres.check_result == CheckResult::NoTemplate) ErrorNoTemplate();
            if (chkres.check_result == CheckResult::MissingFiles) ErrorMissingFiles();
            if (chkres.check_result == CheckResult::WrongRevision) ErrorWrongRevision(chkres.additional_info);
            if (chkres.check_result == CheckResult::NotEnoughPlayers) ErrorNotEnoughPlayers(chkres.additional_info);
            return;
        }
       
        if (tid == 0) {
            ErrorOpenACNH(&tid);
            return;
        }


        bool isACNH = (tid == TID);

        if (!isACNH) {
            ErrorOpenACNH(&tid);
            return;
        }

        if (!isSupportedVersion) {
            ErrorWrongBID(&bid);
            return;
        }

        if (mainAddr == 0x00) {
            util::PrintToNXLink("Error: mainAddr\n");
            ErrorSimpCheck();
            return;
        }

        dmntchtReadCheatProcessMemory(mainAddr, &dreamstrval, sizeof(u32));
        dmntchtReadCheatProcessMemory(mainAddr + EventFlagOffset + (346 * 2), &IsDreamingBed, sizeof(u16));

        if (dreamstrval == 0x0 /*|| IsDreamingBed == 0x0*/) {
            util::PrintToNXLink("Error: NoDream");
            ErrorSimpCheck();
            return;
        }

#endif

        g_dumping_menu = std::make_shared<DumpingMenu>(g_current_menu);
        ChangeMenu(g_dumping_menu);
        g_dumpstart_ft = ns;
        return;
    }

    MainMenu::MainMenu(const char *islandname) : Menu(nullptr) {
        const float x = g_screen_width / 2.0f - WindowWidth / 2.0f;
        const float y = g_screen_height / 2.0f - WindowHeight / 2.0f;
        char dump_button_text[0x1C];
        snprintf(dump_button_text, sizeof(dump_button_text), " Dump %s", islandname);

        this->AddButton(DumpButtonId, dump_button_text, x + HorizontalInset, y + TitleGap, WindowWidth - HorizontalInset * 2, ButtonHeight);
        this->AddButton(ExitButtonId, "Exit", x + HorizontalInset, y + TitleGap + ButtonHeight + VerticalGap, WindowWidth - HorizontalInset * 2, ButtonHeight);
        this->SetButtonSelected(DumpButtonId, true);
    }

    void MainMenu::Update(u64 ns) {
        u64 k_down = padGetButtonsDown(&g_pad);

        if (k_down & HidNpadButton_B) {
            g_exit_requested = true;
        }

        /* Take action if a button has been activated. */
        if (const Button *activated_button = this->GetActivatedButton(); activated_button != nullptr) {
            switch (activated_button->id) {
                case DumpButtonId:
                {
                    gotoNextMenu();
                    return;
                }
                case ExitButtonId:
                    g_exit_requested = true;
                    return;
            }
        }

        this->UpdateButtons();

        /* Fallback on selecting the install button. */
        if (const Button *selected_button = this->GetSelectedButton(); k_down && selected_button == nullptr) {
            this->SetButtonSelected(DumpButtonId, true);
        }
    }

    void MainMenu::Draw(NVGcontext *vg, u64 ns) {
        DrawWindow(vg, (std::string("Luna ") + std::string(STRING_VERSION)).c_str(), g_screen_width / 2.0f - WindowWidth / 2.0f, g_screen_height / 2.0f - WindowHeight / 2.0f, WindowWidth, WindowHeight);
        this->DrawButtons(vg, ns);
    }

    DumpingMenu::DumpingMenu(std::shared_ptr<Menu> prev_menu) : Menu(prev_menu), m_dumping_state(DumpState::NeedsDraw), m_progress_percent(0.0f) {
        const float x = g_screen_width / 2.0f - WindowWidth / 2.0f;
        const float y = g_screen_height / 2.0f - WindowHeight / 2.0f;
        const float button_width = (WindowWidth - HorizontalInset * 2.0f) / 2.0f - ButtonHorizontalGap;

        /* Add buttons. */
        this->AddButton(ExitButtonId, "Exit", x + WindowWidth / 2 - button_width / 2, y + WindowHeight - BottomInset - ButtonHeight + VerticalGap / 2, button_width, ButtonHeight);
        this->SetButtonEnabled(ExitButtonId, false);

        /* Add new Debugger. */
        this->m_debugger = new Debugger();

        /* Prevent the home button from being pressed during dump. */
        hiddbgDeactivateHomeButton();
    }

    void DumpingMenu::InterpretInput() {
        
        if (g_touches_moving) {
            HidTouchScreenState current_touch;
            hidGetTouchScreenStates(&current_touch, 1);

            const int dist_y = g_start_touch.touches[0].y - current_touch.touches[0].y;
            g_start_touch.touches[0].y = current_touch.touches[0].y;
            this->m_nextOffset += static_cast<float>(dist_y);
        }
        //this will be 1.0 at 60FPS and 2.0 at 30FPS and so on... should ensure same speed always
        //float multiplier = (g_last_ft * 60) / 1'000'000'000;
        float stickl = 15.0f/* * multiplier*/;
        float stickr = stickl / 3.0f;
        float dpad = this->LogRowHeight + this->LogRowGap;

        u64 k_down = padGetButtons(&g_pad);
        /* Scroll down. */
       if (k_down & HidNpadButton_AnyDown) {
            if      (k_down & HidNpadButton_StickLDown) this->m_nextOffset += stickl;
            else if (k_down & HidNpadButton_StickRDown) this->m_nextOffset += stickr;
        }
       /* Scroll up. */
        else if (k_down & HidNpadButton_AnyUp) {
            if      (k_down & HidNpadButton_StickLUp) this->m_nextOffset -= stickl;
            else if (k_down & HidNpadButton_StickRUp) this->m_nextOffset -= stickr;
        }
       //require multiple pressed on dpad
       k_down = padGetButtonsDown(&g_pad);
       if (k_down & HidNpadButton_Down) {
           //align offset to next logEntry
           this->m_nextOffset = (this->LogRowHeight + this->LogRowGap) * ceil(this->m_nextOffset / (this->LogRowHeight + this->LogRowGap));
           this->m_nextOffset += dpad;
       }
       else if (k_down & HidNpadButton_Up) {
           //align offset to last logEntry
           this->m_nextOffset = (this->LogRowHeight + this->LogRowGap) * floor(this->m_nextOffset / (this->LogRowHeight + this->LogRowGap));
           this->m_nextOffset -= dpad;
       }

        float max_scroll = this->m_logHeight - this->TextAreaHeight + this->LogRowHorizontalInset;
        /* Don't allow scrolling if there is not enough elements. */
        if (max_scroll < 0.0f) {
            max_scroll = 0.0f;
        }

        /* Don't allow scrolling before the first element. */
        if (this->m_nextOffset < 0.0f) {
                this->m_nextOffset = 0.0f;
        }

        /* Don't allow scrolling past the last element. */
        if (this->m_nextOffset > max_scroll) {
            this->m_nextOffset = max_scroll;
        }
    }

    float DumpingMenu::getTopBound() { return g_screen_height / 2.0f - WindowHeight / 2.0f + TitleGap + ProgressTextHeight + ProgressBarHeight + HorizontalInset + LogPadding; }
    float DumpingMenu::getBottomBound() { return getTopBound() + this->TextAreaHeight - this->LogRowHorizontalInset; }
    float DumpingMenu::getLeftBound() { return g_screen_width / 2.0f - this->WindowWidth / 2.0f + HorizontalInset + this->LogRowHorizontalInset; }
    float DumpingMenu::getRightBound() { return g_screen_width / 2.0f + this->WindowWidth / 2.0f - HorizontalInset - this->LogRowHorizontalInset; }

    void DumpingMenu::EnableButtons() {
        this->SetButtonEnabled(ExitButtonId, true);
        this->SetButtonSelected(ExitButtonId, true);
    }

    //return index of inserted object
    //log will crash, if we reach 2147483647 (max int size) log elements or something, but should be fine for now
    //scroll: should we scroll to the latest entry?
    //TextStyle: how do you want to align the text?
    int DumpingMenu::LogAddLine(std::string text, int index, bool scroll, TextStyle style) {
        int retindex = -1;
        g_editinglog = true;
        int log_entries_ctr = 0;
        for (size_t i = 0; i <= text.size(); i += g_maxloglength) {
            std::string substringtext = text.substr(i, g_maxloglength);
            //no index given
            if (index < 0) {
                LogEntry* newLogEntry = new LogEntry(substringtext, style);
                this->m_log_entries.push_back(newLogEntry);
                retindex = (int)this->m_log_entries.size() - 1;
            }
            //we assume when editing a log entry, that is multiple lines long, that we have to edit the next one as well
            else {
                this->m_log_entries[index + log_entries_ctr]->setText(substringtext);
                retindex = index;
            }
            log_entries_ctr++;
        }
        this->m_scrolling = scroll;
        g_editinglog = false;
        return retindex;
    }

    //horrible memory usage, try to use this as little as possible
    /*
    void DumpingMenu::LogEditElement(std::string oldtext, std::string newtext) {
        g_editinglog = true;
        for (auto elm : this->m_log_entries) {
            if (elm->getText() == oldtext) {
                elm->setText(newtext);
            }
        }
        g_editinglog = false;
    }
    */

    void DumpingMenu::LogEditLastElement(std::string text) {
        g_editinglog = true;
        std::string substringtext = text.substr(0, g_maxloglength);
        this->m_log_entries[this->m_log_entries.size() - 1]->setText(substringtext);
        g_editinglog = false;
    }

    Result DumpingMenu::TransitionDumpState(u64 ns) {
        Result rc = 0;
        if (m_dumping_state == DumpState::NeedsSetup) {
   
            //std::thread decrypt = std::thread([this]() {(Dump::Decryptshizfile(g_current_menu, &this->m_progress_percent)); });
            dump = std::thread([this]() { (Dump::Setup(g_dumping_menu, &this->m_progress_nextPercent, &this->m_enable_buttons, &this->m_dumping_state)); });
            m_dumping_state = DumpState::NeedsWait;
        }
        else if (m_dumping_state == DumpState::NeedsDecrypt) {
            dump.join();
            dump = std::thread([]() { (Dump::Decrypt()); });
            m_dumping_state = DumpState::NeedsWait;
        }

        else if (m_dumping_state == DumpState::NeedsRW) {
            dump.join();
            dump = std::thread([]() { (Dump::RWData()); });
            m_dumping_state = DumpState::NeedsWait;
        }

        else if (m_dumping_state == DumpState::NeedsFix) {
            dump.join();
            dump = std::thread([]() { (Dump::Fixes()); });
            m_dumping_state = DumpState::NeedsWait;
        }
        else if (m_dumping_state == DumpState::NeedsSave) {
            dump.join();
            dump = std::thread([]() { (Dump::Save()); });
            m_dumping_state = DumpState::NeedsWait;
        }
        else if (m_dumping_state == DumpState::End) {
            dump.join();
        }

        return rc;
    }

    void DumpingMenu::Update(u64 ns) {

        /* Transition to the next update state. */
        if (m_dumping_state != DumpState::NeedsDraw && m_dumping_state != DumpState::End && m_dumping_state != DumpState::NeedsWait) {
            this->TransitionDumpState();
        }

        //fontsize + line space
        this->m_logHeight = (20.0f + 6.0f) * m_log_entries.size();

        if (this->m_enable_buttons) {
            this->EnableButtons();
            this->m_enable_buttons = false;
        }
        if (this->m_scrolling) {
            this->ScrollDown();
            this->m_scrolling = false;
        }

        this->InterpretInput();

        if (this->m_logHeight > this->TextAreaHeight) {
            //this the smoof animation
            //while touching, make animation faster
            if (g_prev_touch_count > 0) {
                this->m_speed = this->m_speedmax;
            }
            else {
                //if last touch was less than a 0.35 seconds ago
                if (g_last_touch != 0 && ns - g_last_touch < 350'000'000) {
                    //util::PrintToNXLink("change speed\n");
                    this->m_speed = this->m_speedmax - ((float)(ns - g_last_touch) / 350'000'000) * (this->m_speedmax - this->m_speedmin);
                }
                else {
                    this->m_speed = this->m_speedmin;
                }
                //carry some momentum
                if (this->m_speed > this->m_speedmin + 0.02f){
                    this->m_nextOffset += (this->m_nextOffset - this->m_offset) * this->m_speed;
                }
            }
            this->m_offset += (this->m_nextOffset - this->m_offset) * this->m_speed;
        }
        if (!g_editinglog) {
            //maybe 255 is too little oops
            for (size_t i = 0; i < this->m_log_entries.size(); i++) {
                float y = (20.0f + 6.0f) * i;
                this->m_log_entries[i]->setProperties(this->getLeftBound(), this->getTopBound() - this->m_offset + y, this->getRightBound() - this->getLeftBound());
            }
        }

        
        if (m_progress_percent != m_progress_nextPercent) {
            if (m_progress_percent < m_progress_nextPercent) {
                m_progress_percent += ceil((m_progress_nextPercent - m_progress_percent) * 100) / 300;
                if (m_progress_percent >= m_progress_nextPercent) {
                    m_progress_percent = m_progress_nextPercent;
                }
            }
            if (m_progress_percent > m_progress_nextPercent) {
                m_progress_percent += floor((m_progress_nextPercent - m_progress_percent) * 100) / 300;
                if (m_progress_percent <= m_progress_nextPercent) {
                    m_progress_percent = m_progress_nextPercent;
                }
            }
        }

        u64 k_down = padGetButtonsDown(&g_pad);

        //since we only have one button:
        const Button* activated_button = this->GetActivatedButton();
        if ((activated_button != nullptr || k_down & HidNpadButton_B) && this->GetButton(ExitButtonId)->enabled) {
            g_exit_requested = true;
            dump.join();
            delete this->m_debugger;
            return;
        }

        this->UpdateButtons();
    }

    void DumpingMenu::Draw(NVGcontext* vg, u64 ns) {
        const float x = g_screen_width / 2.0f - WindowWidth / 2.0f;
        const float y = g_screen_height / 2.0f - WindowHeight / 2.0f;

        DrawDumpWindow(vg, "Dumping", x, y, WindowWidth, WindowHeight, x + HorizontalInset, y + TitleGap + ProgressTextHeight + ProgressBarHeight + VerticalGap + LogPadding, WindowWidth - HorizontalInset * 2.0f, TextAreaHeight, x + HorizontalInset, y + TitleGap + ProgressTextHeight, WindowWidth - HorizontalInset * 2.0f, ProgressBarHeight, m_progress_percent, ns);
        DrawProgressText(vg, x + HorizontalInset, y + TitleGap, m_progress_percent);
        //DrawProgressBar(vg, x + HorizontalInset, y + TitleGap + ProgressTextHeight, WindowWidth - HorizontalInset * 2.0f, ProgressBarHeight, m_progress_percent);
        //DrawTextBackground(vg, x + HorizontalInset, y + TitleGap + ProgressTextHeight + ProgressBarHeight + VerticalGap + LogPadding, WindowWidth - HorizontalInset * 2.0f, TextAreaHeight);
        //DrawTextBlock(vg, m_log_buffer, x + HorizontalInset + TextHorizontalInset, y + TitleGap + ProgressTextHeight + ProgressBarHeight + VerticalGap + TextVerticalInset, WindowWidth - (HorizontalInset + TextHorizontalInset) * 2.0f, TextAreaHeight - TextVerticalInset * 2.0f);

        nvgSave(vg);
        nvgScissor(vg, x + HorizontalInset, y + TitleGap + ProgressTextHeight + ProgressBarHeight + VerticalGap + LogPadding, WindowWidth - HorizontalInset * 2.0f, TextAreaHeight);


        if (!g_editinglog) {
            for (u32 i = 0; i < m_log_entries.size(); i++) {
                if (this->m_log_entries[i]->IsEntryInBounds(this->getTopBound() - this->LogRowHeight, this->getBottomBound() - this->LogRowHorizontalInset)) {
                    m_log_entries[i]->Draw(vg);
                }
            }
        }

        if (this->m_logHeight > this->TextAreaHeight) {
            //add scrollbar here
            float paddingtopbottom = 10.0f;
            float scrollbarHeight = (this->TextAreaHeight * (this->TextAreaHeight + this->LogRowHorizontalInset) / this->m_logHeight) - paddingtopbottom*2;
            if (scrollbarHeight < 10.0f) scrollbarHeight = 10.0f;
            float TextBackgroundOffset = y + TitleGap + ProgressTextHeight + ProgressBarHeight + VerticalGap + LogPadding;
            //value between 0.0f and 1.0f
            float percentalposition = (this->m_offset) / (this->m_logHeight - this->TextAreaHeight + this->LogRowHorizontalInset);
            float percentualpadding = paddingtopbottom - (percentalposition * paddingtopbottom * 2.0f);
            float scrollbarOffset = TextBackgroundOffset + (percentalposition * (this->TextAreaHeight)) - (((scrollbarHeight)) * percentalposition) + percentualpadding;
            DrawScrollbar(vg, x + WindowWidth - HorizontalInset * 2.0f - this->LogRowHorizontalInset, scrollbarOffset, scrollbarHeight);
        }

        nvgRestore(vg);

        this->DrawButtons(vg, ns);

        if (m_dumping_state == DumpState::NeedsDraw) {
            m_dumping_state = DumpState::NeedsSetup;
        }
    }

    void StarRandomizer() {

        g_starmarginwidth = (((float)g_screen_width - (VerticalGap * 4.0f)) / (g_starcount / 2));
        g_starmarginheight = (((float)g_screen_height - (VerticalGap * 4.0f)) / (g_starcount / 2));

        u64 seed = 0;
        envGetRandomSeed(&seed);
        std::default_random_engine eng(seed);
        std::uniform_real_distribution<float> distrx(-(g_starmarginwidth - VerticalGap) / 2, g_starmarginwidth - VerticalGap / 2);
        std::uniform_real_distribution<float> distry(-(g_starmarginheight - VerticalGap) / 2, g_starmarginheight - VerticalGap / 2);
        std::uniform_real_distribution<float> distrw(g_starwidthmin, g_starwidthmax);

        for (int i = 0; i < g_starcount; i++) {
            if (i / 2 == 0) {
                g_starsx[i] = (i / 2) * g_starmarginwidth + (VerticalGap * 4) + (distrx(eng) / 2) + HorizontalInset;
                g_starsy[i] = (i / 2) * g_starmarginheight + (VerticalGap * 4) + (distry(eng) / 2) + HorizontalInset;
            }
            if (i / 2 == (g_starcount -1) / 2) {
                g_starsx[i] = (i / 2) * g_starmarginwidth + (VerticalGap * 4) + (distrx(eng) / 2) - HorizontalInset;
                g_starsy[i] = (i / 2) * g_starmarginheight + (VerticalGap * 4) + (distry(eng) / 2) - HorizontalInset;
            }
            else {
                g_starsx[i] = (i / 2) * g_starmarginwidth + (VerticalGap * 4) + distrx(eng);
                g_starsy[i] = (i / 2) * g_starmarginheight + (VerticalGap * 4) + distry(eng);
            }
            g_starsw[i] = distrw(eng);
        }

        std::random_shuffle(g_starsx, g_starsx + sizeof(g_starsx) / sizeof(g_starsx[0]));
        std::random_shuffle(g_starsy, g_starsy + sizeof(g_starsy) / sizeof(g_starsy[0]));
    }

    bool InitializeMenu(u32 screen_width, u32 screen_height) {
        /* Configure and initialize the gamepad. */
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&g_pad);

        /* Initialize the touch screen. */
        hidInitializeTouchScreen();

        /* Set the screen width and height. */
        g_screen_width = screen_width;
        g_screen_height = screen_height;

        /* Randomize the Placement of our Stars */
        StarRandomizer();

        /* Mark as initialized. */
        g_initialized = true;

        dmntchtForceOpenCheatProcess();

        pmdmntGetApplicationProcessId(&pid);
        pminfoGetProgramId(&tid, pid);


        DmntCheatProcessMetadata metadata;
        dmntchtGetCheatProcessMetadata(&metadata);
        memcpy(&bid, metadata.main_nso_build_id, 0x8);
        //fix endianess of hash
        bid = __builtin_bswap64(bid);

        isSupportedVersion = util::findVersionIndex(bid);

        //[[[[main+4BAEF28]+10]+140]+08]
        u64 playerAddr = util::FollowPointerMain(VersionPointerOffset[versionindex], 0x10, 0x140, 0x08, 0xFFFFFFFFFFFFFFFF);

        /* Change the current menu to the main menu and if there is an island name representable, put it on the dump button. */
        g_current_menu = std::make_shared<MainMenu>(util::getIslandNameASCII(playerAddr).c_str());
        //ChangeMenu(std::make_shared<MainMenu>(util::getIslandNameASCII(playerAddr).c_str()));
        
        return true;
    }

    void UpdateMenu(u64 ns) {
        DBK_ABORT_UNLESS(g_initialized);
        DBK_ABORT_UNLESS(g_current_menu != nullptr);
        UpdateInput();
        g_current_menu->Update(ns);
    }

    void RenderMenu(NVGcontext* vg, u64 ns) {
        DBK_ABORT_UNLESS(g_initialized);
        DBK_ABORT_UNLESS(g_current_menu != nullptr);

        /* Draw background. */
        DrawBackground(vg, g_screen_width, g_screen_height, ns);

        /* Draw stars. */
        for (int j = 0; j < g_starcount; j++) {
            const double t = static_cast<double>(ns) / 1'000'000'000.0d + (3 * asin(g_starsw[j] / g_starwidthmax));
            DrawStar(vg, g_screen_width, g_screen_height, g_starsx[j], g_starsy[j], (0.8f * sin(3.0f /*frequency*/ * t) + 4.0f), (1.2f * sin(3.0f /*frequency*/ * t) + 1.0f));
        }

#if DEBUG_OV
        //Draw frametime
        if (g_last_ft3 != 0) {
            char meta[100];
            float nextoffset = 0;
            float logoffset = 0;
            float speed = 0;

            if (g_dumping_menu != nullptr) {
                nextoffset = g_dumping_menu->m_nextOffset;
                logoffset = g_dumping_menu->m_offset;
                speed = g_dumping_menu->m_speed;
            }
            snprintf(meta, sizeof(meta), "FPS: %.1f Framecount: %ld", (1'000'000'000 / ((g_last_ft3 + g_last_ft2 + g_last_ft) / 3.0f)), g_frame_counter);
            DrawMeta(vg, TextHorizontalInset, TextVerticalInset, meta, TextStyle::LeftAlign);
            snprintf(meta, sizeof(meta), "logoffset: %.1F nextlogoffset: %.1F speed: %.2F", logoffset, nextoffset, speed - 0.1f);
            DrawMeta(vg, g_screen_width - TextHorizontalInset, TextVerticalInset, meta, TextStyle::RightAlign);
            snprintf(meta, sizeof(meta), "last touch: %.1fs", (float)(ns - g_last_touch) / 1'000'000'000);
            DrawMeta(vg, TextHorizontalInset, TextVerticalInset + 24.0f + 8.0f + 4.0f, meta, TextStyle::LeftAlign);
        }

        if (g_frame_counter % 60 == 60 / 3) {
            g_last_ft = ns - g_last_ns;
        }
        if (g_frame_counter % 60 == 60 / 6) {
            g_last_ft2 = ns - g_last_ns;
        }
        if (g_frame_counter % 60 == 60 / 9) {
            g_last_ft3 = ns - g_last_ns;
        }
        g_frame_counter++;
        g_last_ns = ns;

#endif




        /*
        DrawStar(vg, g_screen_width, g_screen_height, 40.0f, 64.0f, 3.0f);
        DrawStar(vg, g_screen_width, g_screen_height, 110.0f, 300.0f, 3.0f);
        DrawStar(vg, g_screen_width, g_screen_height, 200.0f, 150.0f, 4.0f);
        DrawStar(vg, g_screen_width, g_screen_height, 370.0f, 280.0f, 3.0f);
        DrawStar(vg, g_screen_width, g_screen_height, 450.0f, 40.0f, 3.5f);
        DrawStar(vg, g_screen_width, g_screen_height, 710.0f, 90.0f, 3.0f);
        DrawStar(vg, g_screen_width, g_screen_height, 900.0f, 240.0f, 3.0f);
        DrawStar(vg, g_screen_width, g_screen_height, 970.0f, 64.0f, 4.0f);
        DrawStar(vg, g_screen_width, g_screen_height, 1160.0f, 160.0f, 3.5f);
        DrawStar(vg, g_screen_width, g_screen_height, 1210.0f, 350.0f, 3.0f);

        */


        g_current_menu->Draw(vg, ns);
    }

    bool IsExitRequested() {
        return g_exit_requested;
    }

}
