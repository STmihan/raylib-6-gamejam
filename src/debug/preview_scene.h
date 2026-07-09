#ifndef DEBUG_PREVIEW_SCENE_H
#define DEBUG_PREVIEW_SCENE_H

#include "raylib.h"

namespace view { class ModelRegistry; }

namespace debug
{
class PreviewScene
{
public:
    bool Active() const { return active_; }
    bool& ActiveRef() { return active_; }

    int& SelectedRef() { return selected_; }
    float& HpFractionRef() { return hpFraction_; }
    float HpFraction() const { return hpFraction_; }
    int ItemCount() const;
    const char* ItemName(int index) const;

    void UpdateCamera();
    Camera3D Camera() const;
    void Draw(const view::ModelRegistry& models) const;
    void DrawGizmo() const;

    void Hit();
    void UpdateFlash(float dt);

private:
    bool active_ = false;
    int selected_ = 0;
    float yaw_ = 45.0f;
    float pitch_ = 25.0f;
    float distance_ = 6.0f;
    float hpFraction_ = 0.7f;
    float flashTimer_ = 0.0f;
};
}

#endif
