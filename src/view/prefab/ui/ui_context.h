#ifndef VIEW_PREFAB_UI_UI_CONTEXT_H
#define VIEW_PREFAB_UI_UI_CONTEXT_H

#include "view/prefab/registries/shader_registry.h"
#include "view/prefab/registries/texture_registry.h"
#include "view/prefab/ui/ui_input.h"
#include "view/prefab/ui/ui_text.h"
#include "view/prefab/ui/ui_theme.h"

namespace view::ui
{
class UiContext
{
public:
    void Load(const ShaderRegistry& shaders, const TextureRegistry& textures)
    {
        text_.Load(shaders.Sdf());
        theme_.Load(shaders.Crystal(), textures.Cards(), textures.White());
    }
    void Unload() { theme_.Unload(); text_.Unload(); }
    void BeginFrame() { input_.BeginFrame(); }

    TextRenderer& Text() { return text_; }
    UiTheme& Theme() { return theme_; }
    UiInput& Input() { return input_; }

private:
    TextRenderer text_;
    UiTheme theme_;
    UiInput input_;
};
}

#endif
