// vas_gl_native.cpp

#include "vas_gl.h"

#include <cmath>

void vasglBeginClipRegion(const QSize &size)
{
    glPushMatrix();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(0xFFFF);
    glColor4f(1.0, 0.0, 0.0, 0.0);

    // draw overal clipping rect to init depth buffer
    glLoadIdentity();
    glTranslated(0, 0, -0.5);
    glBegin(GL_QUADS);
    glVertex2d(0, 0);
    glVertex2d(size.width(), 0);
    glVertex2d(size.width(), size.height());
    glVertex2d(0, size.height());
    glEnd();
}

void vasglEndClipRegion()
{
    glDepthMask(0x0);
    glDepthFunc(GL_LESS);

    glPopMatrix();
}

void vasglDisableClipping()
{
    glDisable(GL_DEPTH_TEST);
}

void vasglCircle(double cx, double cy, double radius, double start_angle,
    double stop_angle, double angle_inc)
{
    double angle;

    glBegin(GL_LINE_STRIP);
    for(angle=start_angle; angle<=stop_angle; angle+=angle_inc)
        glVertex2d(cx+radius*sin(angle), cy-radius*cos(angle));
    glVertex2d(cx+radius*sin(stop_angle), cy-radius*cos(stop_angle));
    glEnd();
}

void vasglFilledCircle(double cx, double cy, double radius,
    double start_angle, double stop_angle, double angle_inc)
{
    double angle;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(cx, cy);
    for(angle=start_angle; angle<=stop_angle; angle+=angle_inc)
        glVertex2d(cx+radius*sin(angle), cy-radius*cos(angle));
    glVertex2d(cx+radius*sin(stop_angle), cy-radius*cos(stop_angle));
    glEnd();
}
