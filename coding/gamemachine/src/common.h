﻿#define BEGIN_NS namespace gm {
#define END_NS }
#define DEFINE_PRIVATE(className) \
	private: \
	className##Private m_data; \
	protected: \
	className##Private& dataRef() { return m_data; }

#define OUT
#define USE_OPENGL
#define RESTART_INDEX (GMuint)(-1)

#ifndef _WINDOWS
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned char BYTE;
#endif

#define FREEGLUT_STATIC
#define GLEW_STATIC
#include "GL/glew.h"

// Options
typedef unsigned char FByte;
typedef GLfloat GMfloat;
typedef GLint GMint;
typedef GLuint GMuint;
#define LINE_MAX 256

// Callbacks settings
#define NEW_GameWorldRenderCallback new GLGameWorldRenderCallback

#ifdef USE_OPENGL
#define DEFAULT_DRAWER new GMGLObjectDrawer()
#endif