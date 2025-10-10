#include "ExtraUI.h"

#include "UI/UIContext.h"

void ExtraUI::draw()
{
    if (*(bool*)0xDEAF2A == 0xEB && *PLAYER_CONTEXT)
    {
        static bool visible = true;
        float sizeX = *BACKBUFFER_WIDTH * 0.5f;
        float sizeY = *BACKBUFFER_HEIGHT * 170.0f / 720.0f;
        float const shadowPosX = *BACKBUFFER_WIDTH * 0.002f;
        ImGui::Begin("Cheat", &visible, UIContext::m_hudFlags);
        {
            static char const* damage = "Disable Damage Cheat Enabled";
            ImGui::SetWindowFocus();
            ImGui::SetWindowSize(ImVec2(sizeX, sizeY));
            ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 0.9f), damage);
            ImGui::SetCursorPos(ImVec2(shadowPosX, *BACKBUFFER_HEIGHT * 0.0036f));
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), damage);
            ImGui::SetWindowPos(ImVec2(0.0f, *BACKBUFFER_HEIGHT * 0.95f));
        }
        ImGui::End();
    }
}
