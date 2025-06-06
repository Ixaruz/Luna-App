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

#include <array>
#include <memory>
#include <vector>
#include <optional>
#include <nanovg.h>
#include <switch.h>
#include "ui_util.hpp"
#include "util.h"
#include "version.h"

namespace dbk {

    struct Button {
        static constexpr u32 InvalidButtonId = -1;

        u32 id;
        bool selected;
        bool enabled;
        char text[256];
        float x;
        float y;
        float w;
        float h;

        inline bool IsPositionInBounds(float x, float y) {
            return x >= this->x && y >= this->y && x < (this->x + this->w) && y < (this->y + this->h);
        }
    };

    class LogEntry {
    public:
        LogEntry(std::string text, TextStyle style) : m_text(text), m_style(style)
        {}

        virtual void Draw(NVGcontext* vg) {
            if (this->m_properties_set) {
                DrawTextAlign(vg, this->m_x, this->m_y, this->m_w, this->m_text.c_str(), this->m_style);
            }
        }

        inline bool IsEntryInBounds(float low_y, float high_y) { return this->m_y >= low_y && this->m_y <= high_y; }


        void setProperties(float x, float y, float w) {
            this->m_x = x;
            this->m_y = y;
            this->m_w = w;
            this->m_properties_set = true;
        }

        inline std::string getText() { return this->m_text; }
        inline void setText(std::string text) { this->m_text = text; }
    
    protected:
        std::string m_text = "";
        float m_x;
        float m_y;
        float m_w;
        bool m_properties_set = false;
        TextStyle m_style;
    };

    enum class Direction {
        Up,
        Down,
        Left,
        Right,
        Invalid,
    };

    class Menu {
        protected:
            static constexpr size_t MaxButtons = 32;
            static constexpr size_t LogBufferSize = 0x1000;
        protected:
            std::array<std::optional<Button>, MaxButtons> m_buttons;
            const std::shared_ptr<Menu> m_prev_menu;
            char m_log_buffer[LogBufferSize];
        protected:
            void AddButton(u32 id, const char *text, float x, float y, float w, float h);
            void SetButtonSelected(u32 id, bool selected);
            void DeselectAllButtons();
            void SetButtonEnabled(u32 id, bool enabled);

            Button *GetButton(u32 id);
            Button *GetSelectedButton();
            Button *GetClosestButtonToSelection(Direction direction);
            Button *GetTouchedButton();
            Button *GetActivatedButton();

            void UpdateButtons();
            void DrawButtons(NVGcontext *vg, u64 ns);

        public:
            Menu(std::shared_ptr<Menu> prev_menu) : m_buttons({}), m_prev_menu(prev_menu), m_log_buffer{} { /* ... */ }
            void LogText(const char* format, ...);

            std::shared_ptr<Menu> GetPrevMenu();
            virtual void Update(u64 ns) = 0;
            virtual void Draw(NVGcontext *vg, u64 ns) = 0;
    };

    class AlertMenu : public Menu {
        protected:
            static constexpr float WindowWidth           = 600.0f;
            static constexpr float WindowHeight          = 240.0f;
            static constexpr float TitleGap              = 100.0f;
            static constexpr float SubTextHeight         = 40.0f;
        protected:
            char m_text[0x100];
            char m_subtext[0x100];
            char m_result_text[0x30];
            char m_format_text[0x20];
            u64 m_rc;
        public:
            AlertMenu(std::shared_ptr<Menu> prev_menu, const char *text, const char *subtext, const char *format = "Result: 0x%08X", u64 rc = 0);

            virtual void Draw(NVGcontext *vg, u64 ns) override;
    };

    class ErrorMenu : public AlertMenu {
        private:
            static constexpr u32 ExitButtonId = 0;
        public:
            ErrorMenu(const char *text, const char *subtext, const char *format = "Result: 0x%08X", u64 rc = 0);

            virtual void Update(u64 ns) override;
    };

    class WarningMenu : public AlertMenu {
        private:
            static constexpr u32 BackButtonId     = 0;
            static constexpr u32 ContinueButtonId = 1;
        private:
            const std::shared_ptr<Menu> m_next_menu;
        public:
            WarningMenu(std::shared_ptr<Menu> prev_menu, std::shared_ptr<Menu> next_menu, const char *text, const char *subtext, const char *format = "Result: 0x%08X", u64 rc = 0);

            virtual void Update(u64 ns) override;
    };

    class MainMenu : public Menu {
        private:
            static constexpr u32 DumpButtonId       = 0;
            static constexpr u32 ExitButtonId       = 1;

            static constexpr float WindowWidth             = 400.0f;
            static constexpr float WindowHeight            = 240.0f;
            static constexpr float TitleGap                = 90.0f;
        protected:
            void gotoNextMenu(u64 ns);
        public:
            MainMenu(const char* islandname = "");

            virtual void Update(u64 ns) override;
            virtual void Draw(NVGcontext *vg, u64 ns) override;
    };


    class DumpingMenu : public Menu {
    private:
        static constexpr u32 ExitButtonId = 0;
        static constexpr size_t MaxFileRows = 11;

        static constexpr float WindowWidth = 600.0f;
        static constexpr float WindowHeight = 600.0f;
        static constexpr float TitleGap = 90.0f;
        static constexpr float ProgressTextHeight = 20.0f;
        static constexpr float ProgressBarHeight = 30.0f;
        static constexpr float LogPadding = 10.0f;
        static constexpr float TextAreaHeight = 348.0f;
        static constexpr float LogRowHeight = 20.0f;
        static constexpr float LogRowGap = 6.0f;
        static constexpr float LogRowHorizontalInset = 10.0f;
        static constexpr float LogListHeight = MaxFileRows * (LogRowHeight + LogRowGap);

        static constexpr size_t UpdateTaskBufferSize = 0x100000;
    private:
        void InterpretInput();
        Result TransitionDumpState(u64 ns);
        float getTopBound();
        float getBottomBound();
        float getLeftBound();
        float getRightBound();
    public:
        enum class DumpState {
            NeedsDraw,
            NeedsSetup,
            NeedsWait,
            NeedsDecrypt,
            NeedsRW,
            NeedsFix,
            NeedsSave,
            End,
        };

        std::vector<LogEntry*> m_log_entries;
        //offset of entire log
        float m_offset = 0, m_nextOffset = 0;
        float m_logHeight = 0;
        float m_speed = 0;
        float m_speedmax = 0.4f;
        float m_speedmin = 0.1f;
        bool m_scrolling = false;

        void EnableButtons();
        int LogAddLine(std::string text, int index = -1, bool scroll = true, TextStyle style = TextStyle::LeftAlign);
        void LogEditLastElement(std::string text);
        
        void ScrollTo(float offset) {
            this->m_nextOffset = offset;
        }

        void ScrollDown() {
            this->m_nextOffset = this->m_logHeight - this->TextAreaHeight + this->LogRowHorizontalInset;
        }

        void ScrollUp() {
            this->m_nextOffset = 0;
        }


        DumpState m_dumping_state;
        AsyncResult m_prepare_result;
        Debugger* m_debugger = nullptr;
        float m_progress_percent = 0.0f;
        float m_progress_nextPercent = 0.0f;
        bool m_enable_buttons = false;

        DumpingMenu(std::shared_ptr<Menu> prev_menu);

        virtual void Update(u64 ns) override;
        virtual void Draw(NVGcontext *vg, u64 ns) override;
    };

    void showErrorMenu(Error error, std::string additional_info = "");
    void StarRandomizer();
    bool InitializeMenu(u32 screen_width, u32 screen_height);
    void UpdateMenu(u64 ns);
    void RenderMenu(NVGcontext *vg, u64 ns);
    bool IsExitRequested();

}
