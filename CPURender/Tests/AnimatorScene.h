#ifndef ANIMATOR_SCENE_H_INCLUDED
#define ANIMATOR_SCENE_H_INCLUDED

#include "../Scene.h"
#include "../Animator.h"

class AnimatorScene : public Scene
{
public:
    void Init() override;
    void Update(float _dt) override;
    void LoadSkybox() override;
    void HandleEvent(float _dt) override;
    
private:
    Model       m_model;
    Mesh        m_mesh;
    Material    m_material;
    Material    m_groundMaterial;
    Texture     m_texture;
    Mesh        m_groundMesh;
    Animation*  m_anim = nullptr;
    Animator*   m_animator = nullptr;
};

#endif