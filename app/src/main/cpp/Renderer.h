#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <android/log.h>
#include <memory>
#include <vector>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

struct android_app;

class Renderer {
public:
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure GL
     */
    android_app *app_;
    Renderer(android_app *pApp)
    {
        app_ = pApp;
        width = ANativeWindow_getWidth(app_->window);
        height = ANativeWindow_getHeight(app_->window);
    }

    virtual ~Renderer() = default;

    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */

    void handleInput();


    virtual void initRenderer() = 0;
    /*!
     * Renders all the models in the renderer
     */
    virtual void render() = 0;

    int width;
    int height;

private:
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H