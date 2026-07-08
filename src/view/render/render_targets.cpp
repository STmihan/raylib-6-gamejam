#include "view/render/render_targets.h"

#include "rlgl.h"

namespace view
{
namespace
{
    constexpr int DepthTextureFormat = 19;
}

RenderTexture2D LoadShadowmap(int width, int height)
{
    RenderTexture2D target = {0};
    target.id = rlLoadFramebuffer();
    target.texture.width = width;
    target.texture.height = height;
    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = DepthTextureFormat;
        target.depth.mipmaps = 1;
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferComplete(target.id);
        rlDisableFramebuffer();
    }
    return target;
}
}
