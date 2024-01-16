#include "widget_manager.hpp"
#include "ecs.hpp"
#include "texture_manager.hpp"
#include "user_input_manager.hpp"
#include "render_manager.hpp"
#include "font_manager.hpp"

extern realware::core::mUserInput* userInputManager;
extern realware::font::mFont* fontManager;

namespace realware
{
    namespace ui
    {
        core::entity mWidget::CreatePopupMenu(mWidget::sCreatePopupDescriptor* desc)
        {
            core::entity popupMenu = desc->Scene->CreateEntity(desc->Name);

            core::sCTransform* popupTransform = desc->Scene->Add<core::sCTransform>(popupMenu);
            core::sCMaterial* popupMaterial = desc->Scene->Add<core::sCMaterial>(popupMenu);
            core::sCPopupMenu* popupWidget = desc->Scene->Add<core::sCPopupMenu>(popupMenu);
            core::sCCaption* popupCaption = desc->Scene->Add<core::sCCaption>(popupMenu);
            core::sCAnimation* popupAnimation = desc->Scene->Add<core::sCAnimation>(popupMenu);

            popupTransform->Init(core::K_TRUE, desc->Position, glm::vec3(0.0f), glm::vec3(desc->Scale, 1.0f));

            popupMaterial->Init(nullptr, glm::vec4(1.0f));

            popupWidget->Init(core::sCWidget::eType::POPUP_MENU);
            popupWidget->OnUpdate = desc->PopupOnUpdate;
            popupWidget->OnClick = desc->PopupOnClick;
            popupWidget->OnHover = desc->PopupOnHover;

            popupCaption->Init(core::sCWidget::eType::CAPTION);
            popupCaption->Font = desc->Font;
            popupCaption->Scale = desc->TextScale;
            popupCaption->Text = "Popup";

            popupAnimation->Frames[0] = desc->PopupAnimation;
            popupAnimation->Fade = 0.0f;

            core::usize index = 0;
            for (auto optionTitle : desc->PopupOptionTitles)
            {
                std::string optionName = desc->Name + std::to_string(index);
                core::entity option = desc->Scene->CreateEntity(optionName);
                index += 1;

                core::sCTransform* buttonTransform = desc->Scene->Add<core::sCTransform>(option);
                core::sCMaterial* buttonMaterial = desc->Scene->Add<core::sCMaterial>(option);
                core::sCButton* buttonWidget = desc->Scene->Add<core::sCButton>(option);
                core::sCCaption* buttonCaption = desc->Scene->Add<core::sCCaption>(option);
                core::sCAnimation* buttonAnimation = desc->Scene->Add<core::sCAnimation>(option);

                buttonTransform->Init(core::K_TRUE, desc->Position, glm::vec3(0.0f), glm::vec3(desc->Scale, 1.0f));

                buttonMaterial->Init(nullptr, glm::vec4(1.0f));

                buttonWidget->Init(core::sCWidget::eType::BUTTON);
                buttonWidget->AlignTextToCenter = core::K_FALSE;
                buttonWidget->OnUpdate = desc->OptionOnUpdate;
                buttonWidget->OnClick = desc->OptionOnClick;
                buttonWidget->OnHover = desc->OptionOnHover;

                buttonCaption->Init(core::sCWidget::eType::CAPTION);
                buttonCaption->Font = desc->Font;
                buttonCaption->Scale = desc->TextScale;
                buttonCaption->Text = optionTitle;

                buttonAnimation->Frames[0] = desc->OptionAnimation;
                buttonAnimation->Fade = 0.0f;

                popupWidget->Elements[popupWidget->ElementCount++] = option;
            }

            return popupMenu;
        }

        core::entity mWidget::CreateCheckbox(mWidget::sCreateCheckboxDescriptor* desc)
        {
            core::entity checkbox = desc->Scene->CreateEntity(desc->Name);

            core::sCTransform* checkboxTransform = desc->Scene->Add<core::sCTransform>(checkbox);
            core::sCMaterial* checkboxMaterial = desc->Scene->Add<core::sCMaterial>(checkbox);
            core::sCCheckbox* checkboxWidget = desc->Scene->Add<core::sCCheckbox>(checkbox);
            core::sCCaption* checkboxCaption = desc->Scene->Add<core::sCCaption>(checkbox);
            core::sCAnimation* checkboxAnimation = desc->Scene->Add<core::sCAnimation>(checkbox);

            checkboxTransform->Init(core::K_TRUE, desc->Position, glm::vec3(0.0f), glm::vec3(desc->Scale, 1.0f));

            checkboxMaterial->Init(nullptr, glm::vec4(1.0f));

            checkboxWidget->Init(core::sCWidget::eType::CHECKBOX);
            checkboxWidget->OnUpdate = desc->OnUpdate;
            checkboxWidget->OnClick = desc->OnClick;
            checkboxWidget->OnHover = desc->OnHover;

            checkboxCaption->Init(core::sCWidget::eType::CAPTION);
            checkboxCaption->Font = desc->Font;
            checkboxCaption->Scale = desc->TextScale;
            checkboxCaption->Text = desc->Text;

            checkboxAnimation->Frames[0] = desc->Animation;

            return checkbox;
        }

        void mWidget::Update(core::cApplication* app, core::cScene* scene)
        {
            UpdateButtons(app, scene);
            UpdatePopupMenus(app, scene);
            UpdateCheckboxes(app, scene);
        }

        void mWidget::UpdateCaptions(core::cApplication* app, core::cScene* scene)
        {
//            scene->ForEachComponent<core::sCCaption>(
//                app,
//                [](core::cApplication* app_, core::cScene* scene_, core::sCCaption* widget_)
//                {
//                    if (widget_->IsVisible == core::K_TRUE && widget_->Text != nullptr)
//                    {
//                        const glm::vec3 widgetPosition = scene_->GetComponent<core::sCTransform>(widget_->Owner)->Position;
//                        const glm::vec3 widgetSize = scene_->GetComponent<core::sCTransform>(widget_->Owner)->Scale;
//                        const glm::vec3 textSize = glm::vec3(
//                            font::mFont::GetTextWidth(widget_->Font, widget_->Text),
//                            font::mFont::GetTextHeight(widget_->Font, widget_->Text),
//                            0.0f
//                        );
//                        float textScale = 0.0f;
//                        if (widget_->Type == core::sWidgetComponent::eType::SLIDER)
//                        {
//                            core::sCTransform* sliderButton = scene_->GetComponent<core::sCTransform>(widget_->Elements[0]);
//                            textScale = (sliderButton->Scale.y / ((float)widget_->Font->GlyphSize / userInputManager->GetWindowSize().y)) * widget_->TextScale;
//                        }
//                        else
//                        {
//                            textScale = (widgetSize.y / ((float)text->Font->GlyphSize / userInputManager->GetWindowSize().y)) * widget_->TextScale;
//                        }
//
//                        glm::vec2 textPosition;
//                        if (widget_->Type == core::sWidgetComponent::eType::BUTTON)
//                        {
//                            textPosition = (widgetPosition + (0.25f * widgetSize)) - (textScale * textSize * 0.25f);
//                        }
//                        else if (widget_->Type == core::sWidgetComponent::eType::COMBOBOX)
//                        {
//                            textPosition = glm::vec3(widgetPosition.x + (0.05f * widgetSize.x), (widgetPosition.y + (0.25f * widgetSize.y)) - (textScale * textSize.y * 0.25f), 0.0f);
//                        }
//                        else if (widget_->Type == core::sWidgetComponent::eType::CHECKBOX)
//                        {
//                            textPosition = glm::vec3(widgetPosition.x - (textScale * textSize.x * 0.5f) - 0.01f, (widgetPosition.y + (0.25f * widgetSize.y)) - (textScale * textSize.y * 0.25f), 0.0f);
//                        }
//                        else if (widget_->Type == core::sWidgetComponent::eType::SLIDER)
//                        {
//                            core::sCTransform* sliderButton = scene_->GetComponent<core::sCTransform>(widget_->Elements[0]);
//                            textPosition = glm::vec3(widgetPosition.x - (textScale * textSize.x * 0.5f) - 0.01f, (widgetPosition.y + (0.25f * widgetSize.y)) - (textScale * textSize.y * 0.25f), 0.0f);
//                        }
//
//                        text->Scale = textScale;
//                        text->Position = textPosition;
//                    }
//                }
//            );
        }

        void mWidget::UpdateButtons(core::cApplication* app, core::cScene* scene)
        {
            scene->ForEach<core::sCButton>(
                app,
                [](core::cApplication* app_, core::cScene* scene_, core::sCButton* widget_)
                {
                    if (widget_->IsVisible == core::K_TRUE)
                    {
                        widget_->OnUpdate(app_, scene_, widget_);

                        core::sCTransform* transform = scene_->Get<core::sCTransform>(widget_->Owner);
                        core::sCCaption* caption = scene_->Get<core::sCCaption>(widget_->Owner);

                        glm::vec2 mousePositionClipSpace = userInputManager->GetCursorPosition() / userInputManager->GetWindowSize();
                        mousePositionClipSpace.y = 1.0f - mousePositionClipSpace.y;

                        if (mousePositionClipSpace.x > transform->Position.x &&
                            mousePositionClipSpace.y > transform->Position.y &&
                            mousePositionClipSpace.x < transform->Position.x + (transform->Scale.x * 0.5f) &&
                            mousePositionClipSpace.y < transform->Position.y + (transform->Scale.y * 0.5f))
                        {
                            widget_->IsHovered = core::K_TRUE;
                            widget_->OnHover(app_, scene_, widget_);

                            if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT) == core::mUserInput::eButtonState::PRESSED)
                            {
                                if ((widget_->StateBits & 1) == 1 && (widget_->StateBits & 2) == 2)
                                {
                                    widget_->StateBits = 0;
                                }
                                else if (widget_->StateBits == 0)
                                {
                                    widget_->StateBits = 1;
                                    widget_->OnClick(app_, scene_, widget_);
                                }
                            }
                            else if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT) == userInputManager->eButtonState::RELEASED)
                            {
                                if (widget_->StateBits == 1) {
                                    widget_->StateBits |= 2;
                                }
                            }
                        }
                        else
                        {
                            widget_->IsHovered = core::K_FALSE;
                        }

                        if (caption != nullptr)
                        {
                            const glm::vec3 textSize = glm::vec3(
                                fontManager->GetTextWidth(caption->Font, caption->Text),
                                fontManager->GetTextHeight(caption->Font, caption->Text),
                                0.0f
                            );

                            if (widget_->AlignTextToCenter == core::K_TRUE) {
                                caption->Position = (transform->Position + (0.25f * transform->Scale)) -
                                                    (caption->Scale * textSize * 0.25f);
                            } else if (widget_->AlignTextToCenter == core::K_FALSE) {
                                caption->Position = glm::vec3(
                                    transform->Position.x + (0.025f * transform->Scale.x),
                                    (transform->Position.y + (0.25f * transform->Scale.y)) -
                                    (caption->Scale * textSize.y * 0.25f),
                                    0.0f
                                );
                            }
                        }
                    }
                }
            );
        }

        void mWidget::UpdatePopupMenus(core::cApplication* app, core::cScene* scene)
        {
            scene->ForEach<core::sCPopupMenu>(
                app,
                [](core::cApplication* app_, core::cScene* scene_, core::sCPopupMenu* widget_)
                {
                    if (widget_->IsVisible == core::K_TRUE)
                    {
                        core::sCTransform* transform = scene_->Get<core::sCTransform>(widget_->Owner);
                        core::sCCaption* caption = scene_->Get<core::sCCaption>(widget_->Owner);

                        glm::vec2 mousePositionClipSpace = userInputManager->GetCursorPosition() / userInputManager->GetWindowSize();
                        mousePositionClipSpace.y = 1.0f - mousePositionClipSpace.y;

                        if (mousePositionClipSpace.x > transform->Position.x &&
                            mousePositionClipSpace.y > transform->Position.y &&
                            mousePositionClipSpace.x < transform->Position.x + (transform->Scale.x * 0.5f) &&
                            mousePositionClipSpace.y < transform->Position.y + (transform->Scale.y * 0.5f))
                        {
                            widget_->IsHovered = core::K_TRUE;
                            widget_->OnHover(app_, scene_, widget_);

                            if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT) == userInputManager->eButtonState::PRESSED)
                            {
                                if ((widget_->StateBits & 1) == 1 && (widget_->StateBits & 2) == 2)
                                {
                                    widget_->StateBits = 0;
                                }
                                else if (widget_->StateBits == 0)
                                {
                                    widget_->StateBits = 1;
                                    if (widget_->HideElements == core::K_TRUE) {
                                        widget_->HideElements = core::K_FALSE;
                                    } else if (widget_->HideElements == core::K_FALSE) {
                                        widget_->HideElements = core::K_TRUE;
                                    }
                                    widget_->OnClick(app_, scene_, widget_);
                                }
                            }
                            else if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT) == userInputManager->eButtonState::RELEASED)
                            {
                                if (widget_->StateBits == 1) {
                                    widget_->StateBits |= 2;
                                }
                            }
                        }
                        else
                        {
                            widget_->IsHovered = core::K_FALSE;
                        }

                        if (widget_->SelectedElementIndex == -1) {
                            widget_->SelectedElementIndex = 0;
                        }

                        const core::s32 elementCount = widget_->ElementCount;
                        const core::s32 startElement = 0;
                        float height = 0.0f;

                        for (core::s32 i = startElement; i < elementCount; i++)
                        {
                            core::entity element = widget_->Elements[i];
                            core::sCButton* elementWidget = scene_->Get<core::sCButton>(element);
                            core::sCCaption* elementCaption = scene_->Get<core::sCCaption>(element);
                            core::sCTransform* elementTransform = scene_->Get<core::sCTransform>(element);
                            const float elementHeight = elementTransform->Scale.y;

                            if (widget_->HideElements == core::K_FALSE)
                            {
                                height += elementHeight * 0.5f;
                                const glm::vec2 position = glm::vec2(transform->Position.x, transform->Position.y - height);

                                if (elementWidget->StateBits == 1) {
                                    widget_->SelectedElementIndex = i;
                                }

                                if (elementCaption != nullptr) {
                                    elementCaption->IsVisible = core::K_TRUE;
                                }
                                elementWidget->IsVisible = core::K_TRUE;
                                elementTransform->Position.x = position.x;
                                elementTransform->Position.y = position.y;
                            }
                            else if (widget_->HideElements == core::K_TRUE)
                            {
                                if (elementCaption != nullptr) {
                                    elementCaption->IsVisible = core::K_FALSE;
                                }
                                elementWidget->IsVisible = core::K_FALSE;
                            }
                        }

                        const glm::vec3 textSize = glm::vec3(
                            fontManager->GetTextWidth(caption->Font, caption->Text),
                            fontManager->GetTextHeight(caption->Font, caption->Text),
                            0.0f
                        );

                        caption->Text = scene_->Get<core::sCCaption>(widget_->Elements[widget_->SelectedElementIndex])->Text;
                        caption->Position = glm::vec3(transform->Position.x + (0.05f * transform->Scale.x), (transform->Position.y + (0.25f * transform->Scale.y)) - (caption->Scale * textSize.y * 0.25f), 0.0f);
//                        caption->Position = glm::vec3(transform->Position.x + (0.05f * transform->Scale.x), (transform->Position.y + (0.25f * transform->Scale.y)) - (caption->Scale * textSize.y * 0.25f), 0.0f);
                    }
                }
            );
        }

        void mWidget::UpdateCheckboxes(core::cApplication* app, core::cScene* scene)
        {
            scene->ForEach<core::sCCheckbox>(
                app,
                [](core::cApplication* app_, core::cScene* scene_, core::sCCheckbox* widget_)
                {
                    if (widget_->IsVisible == core::K_TRUE)
                    {
                        core::sCTransform* transform = scene_->Get<core::sCTransform>(widget_->Owner);
                        core::sCCaption* caption = scene_->Get<core::sCCaption>(widget_->Owner);

                        glm::vec2 mousePositionClipSpace = userInputManager->GetCursorPosition() / userInputManager->GetWindowSize();
                        mousePositionClipSpace.y = 1.0f - mousePositionClipSpace.y;

                        if (mousePositionClipSpace.x > transform->Position.x &&
                            mousePositionClipSpace.y > transform->Position.y &&
                            mousePositionClipSpace.x < transform->Position.x + (transform->Scale.x * 0.5f) &&
                            mousePositionClipSpace.y < transform->Position.y + (transform->Scale.y * 0.5f))
                        {
                            widget_->IsHovered = core::K_TRUE;
                            widget_->OnHover(app_, scene_, widget_);

                            if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT) == userInputManager->eButtonState::PRESSED)
                            {
                                if ((widget_->StateBits & 1) == 1 && (widget_->StateBits & 2) == 2)
                                {
                                    widget_->StateBits = 0;
                                    widget_->Value = core::K_TRUE;
                                }
                                else if (widget_->StateBits == 0)
                                {
                                    widget_->StateBits = 1;
                                    widget_->Value = core::K_FALSE;
                                    widget_->OnClick(app_, scene_, widget_);
                                }
                            }
                            else if (userInputManager->GetMouseKey(GLFW_MOUSE_BUTTON_LEFT) == userInputManager->eButtonState::RELEASED)
                            {
                                if (widget_->StateBits == 1) {
                                    widget_->StateBits |= 2;
                                }
                            }
                        }
                        else
                        {
                            widget_->IsHovered = core::K_FALSE;
                        }

                        const glm::vec3 textSize = glm::vec3(
                            fontManager->GetTextWidth(caption->Font, caption->Text),
                            fontManager->GetTextHeight(caption->Font, caption->Text),
                            0.0f
                        );

                        caption->Position = glm::vec3(transform->Position.x - (caption->Scale * textSize.x * 0.5f) - 0.01f, (transform->Position.y + (0.25f * transform->Scale.y)) - (caption->Scale * textSize.y * 0.25f), 0.0f);
                    }
                }
            );
        }
    }
}