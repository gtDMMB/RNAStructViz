#include "GLWindow.h"
#include <FL/glu.h>
#include <FL/Fl.H>

#include <iostream>
#include <assert.h>

GLWindow::GLWindow(int x, int y, int width, int height, const char* label)
    : Fl_Gl_Window(x, y, width, height, label)
    , m_panning(false)
    , m_zooming(false)
{
    mode(FL_RGB|FL_DOUBLE|FL_ALPHA);

    m_needTextureUpdate = false;
    m_textureIndices[0] = m_textureIndices[1] = (GLuint)-1;

    m_origin[0] = 0.0f;
    m_origin[1] = 0.0f;
    m_scale = 1.0f;
    k_scaleAmt = 1.0f;

}

GLWindow::~GLWindow() {
}

void GLWindow::SetTextureData(uchar* data, const int textureSize)
{
    if (textureSize == 2048)
    {
    m_textureData[0] = data;
    m_textureSizes[0] = textureSize;
    }
    else
    {
    m_textureData[1] = data;
    m_textureSizes[1] = textureSize;
    }
}

void GLWindow::draw()
{
    if (!valid())
    {
        // Reset the viewport
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        Fl_Gl_Window::ortho();
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        m_needTextureUpdate = true;

        if (m_textureIndices[0] != (GLuint)-1)
            glDeleteTextures(2, m_textureIndices);
        glGenTextures(2, m_textureIndices);
    }

    if (m_needTextureUpdate)
    {
        for (int i = 0; i < 2; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, m_textureIndices[i]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            GLfloat texBorderColor[4];
            texBorderColor[0] = texBorderColor[1] = texBorderColor[2] = 1.0f;
            texBorderColor[3] = 1.0f;
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, texBorderColor);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                3,
                m_textureSizes[i],
                m_textureSizes[i],
                1,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                m_textureData[i]);
        }

        m_needTextureUpdate = false;
    }

    // Figure out the largest square we can draw
    float texParamDelta[2];
    float polyDelta[2];
    GetTexturingParameters(m_scale, polyDelta, texParamDelta);

    glBindTexture(GL_TEXTURE_2D, m_textureIndices[0]);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(m_origin[0], m_origin[1] + texParamDelta[1]);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(m_origin[0] + texParamDelta[0], m_origin[1] + texParamDelta[1]);
    glVertex2f(polyDelta[0], 0.0f);
    glTexCoord2f(m_origin[0] + texParamDelta[0], m_origin[1]);
    glVertex2f(polyDelta[0], polyDelta[1]);
    glTexCoord2f(m_origin[0], m_origin[1] + texParamDelta[1]);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(m_origin[0] + texParamDelta[0], m_origin[1]);
    glVertex2f(polyDelta[0], polyDelta[1]);
    glTexCoord2f(m_origin[0], m_origin[1]);
    glVertex2f(0.0f, polyDelta[1]);
    glEnd();
}

void GLWindow::GetTexturingParameters(const float scale, float* polyDelta, float* texParamDelta)
{
    polyDelta[0] = (float)w();
    polyDelta[1] = (float)h();
    if (polyDelta[0] < polyDelta[1])
    {
    texParamDelta[0] = scale;
    texParamDelta[1] = scale * polyDelta[1] / polyDelta[0];
    }
    else
    {
    texParamDelta[0] = scale * polyDelta[0] / polyDelta[1];
    texParamDelta[1] = scale;
    }
}

int GLWindow::handle(int event)
{
    int eventState = Fl::event_state();

    switch (event)
    {
        case FL_PUSH:
            m_mouseDown[0] = Fl::event_x();
            m_mouseDown[1] = Fl::event_y();
            m_originDown[0] = m_origin[0];
            m_originDown[1] = m_origin[1];
            m_scaleDown = m_scale;
            if(k_scaleAmt < 1.0)
            {
                m_panning = true;
            }
            return 1;
            break;

        case FL_DRAG:
            if (m_panning)
            {
                ProcessPan(Fl::event_x(), Fl::event_y());
                return 1;
            }
            break;

        case FL_RELEASE:
            if (m_panning)
            {
                ProcessPan(Fl::event_x(), Fl::event_y());
                m_panning = false;
                return 1;
            }
            break;

        case FL_SHORTCUT:
            m_originDown[0] = m_origin[0];
            m_originDown[1] = m_origin[1];
            m_scaleDown = m_scale;
            if(Fl::get_key('+') && (eventState & FL_SHIFT))
            {
                k_scaleAmt = k_scaleAmt - 0.05;
                k_zoomIn = true;
                KeyProcessScale();
            }
            if(Fl::get_key('-') && (eventState & FL_SHIFT))
            {
                k_scaleAmt = k_scaleAmt + 0.05;
                k_zoomIn = false;
                KeyProcessScale();
            }
            return 1;
            break;

        default:
            break;
    }

    return Fl_Gl_Window::handle(event);
}

void GLWindow::ProcessPan(int xCurrent, int yCurrent)
{
    float texParamDelta[2];
    float polyDelta[2];
    GetTexturingParameters(m_scaleDown, polyDelta, texParamDelta);

    float xDiff = ((float)m_mouseDown[0] - (float)xCurrent) / polyDelta[0] * texParamDelta[0];
    float yDiff = ((float)m_mouseDown[1] - (float)yCurrent) / polyDelta[1] * texParamDelta[1];

    m_origin[0] = m_originDown[0] + xDiff;
    m_origin[1] = m_originDown[1] + yDiff;

    redraw();
}

/*
 * OBSOLETE
 * Zoom using the mouse
 */
void GLWindow::ProcessScale(int xCurrent, int yCurrent)
{
    if (xCurrent < 0)
        xCurrent = 0;
    if (xCurrent > w())
        xCurrent = w();
    if (yCurrent < 0)
        yCurrent = 0;
    if (yCurrent > h())
        yCurrent = h();

    float yDiff = (float)yCurrent - (float)m_mouseDown[1];

    if (yDiff < 0)
    {
        // Make it bigger means reduce scale. Min scale 0.1.
        float distToTop = (float)yCurrent / (float)m_mouseDown[1];
        m_scale = 0.1f + distToTop * (m_scaleDown - 0.1f);
    }
    else
    {
        // Make it smaller means reduce scale. Max scale 1.0.
        float distToBottom = (float)h() - (float)m_mouseDown[1];
        m_scale = m_scaleDown + (yDiff / distToBottom) * (1.0f - m_scaleDown);
    }

    float downTexParamDelta[2];
    float downPolyDelta[2];
    float newTexParamDelta[2];
    GetTexturingParameters(m_scaleDown, downPolyDelta, downTexParamDelta);
    GetTexturingParameters(m_scale, downPolyDelta, newTexParamDelta);
    float windowCoords[2];
    windowCoords[0] = (float)m_mouseDown[0] / downPolyDelta[0];
    windowCoords[1] = (float)m_mouseDown[1] / downPolyDelta[1];
    m_origin[0] = m_originDown[0] + windowCoords[0] * (downTexParamDelta[0] - newTexParamDelta[0]);
    m_origin[1] = m_originDown[1] + windowCoords[1] * (downTexParamDelta[1] - newTexParamDelta[1]);
    redraw();
}

/*
 * Zoom using the (+/-) keys
 */
void GLWindow::KeyProcessScale()
{
    if (k_scaleAmt > 1.0f)
        k_scaleAmt = 1.0f;
    if (k_scaleAmt < 0.1f)
        k_scaleAmt = 0.1f;

    if (k_zoomIn)
    {
        // Make it bigger means reduce scale. Min scale 0.1.
        m_scale = 0.1f + k_scaleAmt *(m_scaleDown - 0.1f);
    }
    else
    {
        // Make it smaller means reduce scale. Max scale 1.0.
        m_scale = m_scaleDown + (k_scaleAmt/1.5) * (1.0f - m_scaleDown);
    }
    float downTexParamDelta[2];
    float downPolyDelta[2];
    float newTexParamDelta[2];
    GetTexturingParameters(m_scaleDown, downPolyDelta, downTexParamDelta);
    GetTexturingParameters(m_scale, downPolyDelta, newTexParamDelta);
    m_origin[0] = m_originDown[0] + 0.5f * (downTexParamDelta[0] - newTexParamDelta[0]);
    m_origin[1] = m_originDown[1] + 0.5f * (downTexParamDelta[1] - newTexParamDelta[1]);

    redraw();

}
