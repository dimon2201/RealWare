#pragma once

#include <string>
#include <vector>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "ecs.hpp"

namespace realware
{
    namespace core
    {
        class cApplication;
    }

    namespace render
    {
        struct sRenderPass;
    }

    namespace font
    {
        class sFont;
        class cFontManager;
    }

    using namespace core;

    namespace ui
    {
        class mWidget
        {

        public:
            mWidget(cApplication* app);
            ~mWidget();

            struct sCreatePopupDescriptor
            {
                core::cScene* Scene;
                std::string Name;
                glm::vec3 Position;
                glm::vec2 Scale;
                font::sFont* Font;
                float TextScale;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> PopupOnUpdate;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> PopupOnClick;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> PopupOnHover;
                std::vector<core::sArea*>* PopupAnimation;
                std::vector<const char*> PopupOptionTitles;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> OptionOnUpdate;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> OptionOnClick;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> OptionOnHover;
                std::vector<core::sArea*>* OptionAnimation;
            };

            struct sCreateCheckboxDescriptor
            {
                core::cScene* Scene;
                std::string Name;
                glm::vec3 Position;
                glm::vec2 Scale;
                font::sFont* Font;
                float TextScale;
                const char* Text;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> OnUpdate;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> OnClick;
                std::function<void(core::cApplication*, core::cScene*, core::sCWidget*)> OnHover;
                std::vector<core::sArea*>* Animation;
            };

            core::entity CreatePopupMenu(sCreatePopupDescriptor* desc);
            core::entity CreateCheckbox(sCreateCheckboxDescriptor* desc);
            void Update(core::cApplication* app, core::cScene* scene);
            void UpdateCaptions(core::cApplication* app, core::cScene* scene);
            void UpdateButtons(core::cApplication* app, core::cScene* scene);
            void UpdatePopupMenus(core::cApplication* app, core::cScene* scene);
            void UpdateCheckboxes(core::cApplication* app, core::cScene* scene);

            inline void SetFont(const font::sFont* font) { m_font = (font::sFont*)font; }

        private:
            cApplication* m_app = nullptr;
            font::sFont* m_font = nullptr;
            core::sCWidget* m_activeWidgetComponent = nullptr;

        };
    }
}