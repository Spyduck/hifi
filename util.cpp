//
//  util.cpp
//  interface
//
//  Created by Philip Rosedale on 8/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include "world.h"

void render_world_box()
{
    //  Show edge of world 
    glDisable(GL_LIGHTING);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    glVertex3f(0,0,0);
    glVertex3f(WORLD_SIZE,0,0);
    glVertex3f(WORLD_SIZE,WORLD_SIZE,0);
    glVertex3f(0,WORLD_SIZE,0);
    glVertex3f(0,0,0);
    glVertex3f(0,0,WORLD_SIZE);
    glVertex3f(WORLD_SIZE,0,WORLD_SIZE);
    glVertex3f(WORLD_SIZE,WORLD_SIZE,WORLD_SIZE);
    glVertex3f(0,WORLD_SIZE,WORLD_SIZE);
    glVertex3f(0,0,WORLD_SIZE);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(0,WORLD_SIZE,0);
    glVertex3f(0,WORLD_SIZE,WORLD_SIZE);

    glVertex3f(WORLD_SIZE,WORLD_SIZE,0);
    glVertex3f(WORLD_SIZE,WORLD_SIZE,WORLD_SIZE);

    glVertex3f(WORLD_SIZE,0,0);
    glVertex3f(WORLD_SIZE,0,WORLD_SIZE);
    glEnd();
}


double diffclock(timeval clock1,timeval clock2)
{
	double diffms = (clock2.tv_sec - clock1.tv_sec) * 1000.0;
    diffms += (clock2.tv_usec - clock1.tv_usec) / 1000.0;   // us to ms
    
	return diffms;
}

void drawtext(int x, int y, float scale, float rotate, float thick, int mono, char *string, 
              float r=1.0, float g=1.0, float b=1.0)
{
    //
    //  Draws text on screen as stroked so it can be resized
    //
    int len, i;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glColor3f(r,g,b);
    glRotated(180+rotate,0,0,1);
    glRotated(180,0,1,0);
    glLineWidth(thick);
    glScalef(scale, scale, 1.0);
    len = (int) strlen(string);
	for (i = 0; i < len; i++)
	{
        if (!mono) glutStrokeCharacter(GLUT_STROKE_ROMAN, int(string[i]));
        else glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, int(string[i]));
	}
    glPopMatrix();

}
