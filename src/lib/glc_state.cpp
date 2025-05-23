/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_state.cpp implementation of the GLC_State class.

#include <QtDebug>

#include "glc_state.h"
#include "glc_ext.h"
#include "sceneGraph/glc_octree.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>

bool GLC_State::m_UseVbo= true;
bool GLC_State::m_PointSpriteSupported= true;
bool GLC_State::m_UseShader= true;
bool GLC_State::m_UseSelectionShader= false;
bool GLC_State::m_IsInSelectionMode= false;
bool GLC_State::m_IsPixelCullingActivated= true;
bool GLC_State::m_IsFrameBufferSupported= false;
bool GLC_State::m_IsFrameBufferBlitSupported= false;

QString GLC_State::m_Version;
QString GLC_State::m_Vendor;
QString GLC_State::m_Renderer;

bool GLC_State::m_UseCache= false;

GLC_CacheManager GLC_State::m_CacheManager;

bool GLC_State::m_IsSpacePartitionningActivated= false;
bool GLC_State::m_IsFrustumCullingActivated= false;
bool GLC_State::m_IsValid= false;

GLC_State::~GLC_State()
{
}

bool GLC_State::vboUsed()
{
    return m_UseVbo;
}

bool GLC_State::frameBufferSupported()
{
    Q_ASSERT(m_IsValid);
    return m_IsFrameBufferSupported;
}

bool GLC_State::frameBufferBlitSupported()
{
    Q_ASSERT(m_IsValid);
    return m_IsFrameBufferBlitSupported;
}

bool GLC_State::glslUsed()
{
    Q_ASSERT(m_IsValid);
    return m_UseShader;
}

bool GLC_State::pointSpriteSupported()
{
    return m_PointSpriteSupported;
}

bool GLC_State::selectionShaderUsed()
{
    Q_ASSERT(m_IsValid);
    return m_UseSelectionShader;
}

bool GLC_State::isInSelectionMode()
{
    Q_ASSERT(m_IsValid);
    return m_IsInSelectionMode;
}

QString GLC_State::version()
{
    Q_ASSERT(m_IsValid);
    return m_Version;
}

QString GLC_State::vendor()
{
    Q_ASSERT(m_IsValid);
    return m_Vendor;
}

QString GLC_State::renderer()
{
    Q_ASSERT(m_IsValid);
    return m_Renderer;
}

bool GLC_State::isPixelCullingActivated()
{
    Q_ASSERT(m_IsValid);
    return m_IsPixelCullingActivated;
}

bool GLC_State::cacheIsUsed()
{
    return m_UseCache;
}

GLC_CacheManager& GLC_State::currentCacheManager()
{
    return m_CacheManager;
}

bool GLC_State::isSpacePartitionningActivated()
{
    return m_IsSpacePartitionningActivated;
}

int GLC_State::defaultOctreeDepth()
{
    return GLC_Octree::defaultDepth();
}

bool GLC_State::isFrustumCullingActivated()
{
    return m_IsFrustumCullingActivated;
}

void GLC_State::init()
{
    if (!m_IsValid)
    {
        Q_ASSERT((NULL != QOpenGLContext::currentContext()) &&  QOpenGLContext::currentContext()->isValid());
        setPointSpriteSupport();
        setFrameBufferSupport();
        setFrameBufferBlitSupport();
        m_Version= (char *) glGetString(GL_VERSION);
        m_Vendor= (char *) glGetString(GL_VENDOR);
        m_Renderer= (char *) glGetString(GL_RENDERER);

        m_IsValid= true;
    }
}

bool GLC_State::isValid()
{
    return m_IsValid;
}

void GLC_State::setVboUsage(const bool vboUsed)
{
    m_UseVbo= vboUsed;
}

void GLC_State::setPointSpriteSupport()
{
    m_PointSpriteSupported= glc::extensionIsSupported("GL_ARB_point_parameters") && glc::loadPointSpriteExtension();
    Q_ASSERT(m_PointSpriteSupported);
}

void GLC_State::setFrameBufferSupport()
{
    m_IsFrameBufferSupported= QOpenGLFramebufferObject::hasOpenGLFramebufferObjects();
}

void GLC_State::setFrameBufferBlitSupport()
{
    m_IsFrameBufferBlitSupported= QOpenGLFramebufferObject::hasOpenGLFramebufferBlit();
}

void GLC_State::setGlslUsage(const bool glslUsage)
{
    m_UseShader= glslUsage;
}

void GLC_State::setSelectionShaderUsage(const bool shaderUsed)
{
    m_UseSelectionShader= shaderUsed;
}

void GLC_State::setSelectionMode(const bool mode)
{
    m_IsInSelectionMode= mode;
}

void GLC_State::setPixelCullingUsage(const bool activation)
{
    m_IsPixelCullingActivated= activation;
}

void GLC_State::setCacheUsage(const bool cacheUsage)
{
    m_UseCache= cacheUsage;
}

void GLC_State::setCurrentCacheManager(const GLC_CacheManager& cacheManager)
{
    m_CacheManager= cacheManager;
}

void GLC_State::setSpacePartionningUsage(const bool usage)
{
    m_IsSpacePartitionningActivated= usage;
}

void GLC_State::setDefaultOctreeDepth(int depth)
{
    GLC_Octree::setDefaultDepth(depth);
}

void GLC_State::setFrustumCullingUsage(bool usage)
{
    m_IsFrustumCullingActivated= usage;
}
