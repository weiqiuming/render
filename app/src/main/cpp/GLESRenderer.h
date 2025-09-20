#ifndef ANDROIDGLINVESTIGATIONS_GLESRENDERER_H
#define ANDROIDGLINVESTIGATIONS_GLESRENDERER_H

#include <EGL/egl.h>
#include <memory>

#include "Model.h"
#include "Shader.h"
#include "Renderer.h"

class GLESRenderer : public Renderer {
public:
    //GLESRenderer(android_app *pApp);
    using Renderer::Renderer;
    ~GLESRenderer();
    void initRenderer() override ;
    void render() override;

private:
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;
};

#endif //ANDROIDGLINVESTIGATIONS_GLESRENDERER_H