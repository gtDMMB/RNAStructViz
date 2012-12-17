/*
    A window that uses OpenGL to draw orthographic texturesd quad, basically to project an image.
*/

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

class GLWindow : public Fl_Gl_Window
{
public:
    GLWindow(int x, int y, int w, int h, const char *label = 0);

    inline void UpdateTexture()
    {
	m_needTextureUpdate = true;
	redraw();
    }

    void SetTextureData(uchar* data, const int textureSize);

protected:
    void draw();

    int handle(int);

private:
    void GetTexturingParameters(const float scale, float* polyDelta, float* texParamDelta);
    void ProcessPan(int xCurrent, int yCurrent);
    void ProcessScale(int xCurrent, int yCurrent);
    void KeyProcessScale();

    // Texture data for many levels: 2048, 1024
    int m_textureSizes[2];
    uchar* m_textureData[2];
    GLuint m_textureIndices[2];
    bool m_needTextureUpdate;

    // Pan/Zoom params
    float m_origin[2]; // Origin in texture space
    float m_scale; // scale to apply to texture coordinates

    // Mouse motion processing
    int m_mouseDown[2];
    float m_originDown[2];
    float m_scaleDown;
    bool m_panning;
    bool m_zooming;
    
    //Keybaord
    float k_scaleAmt;
    float k_scaleDown;
    bool k_zoomIn;
    
};


#endif // GLWINDOW_H
