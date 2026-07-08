#ifndef VIEW_EFFECT_TOON_EFFECT_H
#define VIEW_EFFECT_TOON_EFFECT_H

#include "raylib.h"

#include "data/render/shadow_params.h"

namespace view
{
class ModelRegistry;

class ToonEffect
{
public:
    void Init(Shader shader);
    void Apply(ModelRegistry& models) const;
    void Upload(const data::ShadowParams& params, Matrix lightViewProj);
    void BindShadowMap(unsigned int depthTextureId, int slot);

private:
    Shader shader_{};
    int sunDirLoc_ = 0;
    int ambientLoc_ = 0;
    int bandsLoc_ = 0;
    int shadowStrengthLoc_ = 0;
    int softnessLoc_ = 0;
    int biasSlopeLoc_ = 0;
    int biasConstantLoc_ = 0;
    int shadowMapLoc_ = 0;
    int lightViewProjLoc_ = 0;
};
}

#endif
