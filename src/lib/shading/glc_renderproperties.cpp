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
//! \file glc_renderproperties.cpp implementation for the GLC_RenderProperties class.

#include "glc_renderproperties.h"

#include "../maths/glc_geomtools.h"

// Default constructor
GLC_RenderProperties::GLC_RenderProperties()
    : m_Uid(glc::GLC_GenUserID())
    , m_IsSelected(false)
    , m_PolyFace(GL_FRONT_AND_BACK)
    , m_PolyMode(GL_FILL)
    , m_RenderMode(glc::NormalRenderMode)
    , m_SavedRenderMode(m_RenderMode)
    , m_pOverwriteMaterial(nullptr)
    , m_OverwriteOpacity(-1.0f)
    , m_pBodySelectedPrimitvesId(nullptr)
    , m_pOverwritePrimitiveMaterialMaps(nullptr)
    , m_RenderingFlag(glc::ShadingFlag)
    , m_CurrentBody(0)
    , m_MaterialsUsage()
    , m_Selectable(true)
    , m_OverwriteRenderingFlag(glc::NoneRenderFlag)
{

}

// Copy constructor
GLC_RenderProperties::GLC_RenderProperties(const GLC_RenderProperties& other)
    : m_Uid(glc::GLC_GenUserID())
    , m_IsSelected(other.m_IsSelected)
    , m_PolyFace(other.m_PolyFace)
    , m_PolyMode(other.m_PolyMode)
    , m_RenderMode(other.m_RenderMode)
    , m_SavedRenderMode(other.m_SavedRenderMode)
    , m_pOverwriteMaterial(other.m_pOverwriteMaterial)
    , m_OverwriteOpacity(other.m_OverwriteOpacity)
    , m_pBodySelectedPrimitvesId(nullptr)
    , m_pOverwritePrimitiveMaterialMaps(nullptr)
    , m_RenderingFlag(other.m_RenderingFlag)
    , m_CurrentBody(other.m_CurrentBody)
    , m_MaterialsUsage(other.m_MaterialsUsage)
    , m_Selectable(other.m_Selectable)
    , m_OverwriteRenderingFlag(other.m_OverwriteRenderingFlag)
{
	// Update overwrite material usage
    if (nullptr != m_pOverwriteMaterial)
	{
		m_pOverwriteMaterial->addUsage(m_Uid);
	}

	// Copy the Hash of set of id of selected primitives
    if (nullptr != other.m_pBodySelectedPrimitvesId)
	{
		m_pBodySelectedPrimitvesId= new QHash<int, QSet<GLC_uint>* >();
        QHash<int, QSet<GLC_uint>* >::const_iterator iSet= other.m_pBodySelectedPrimitvesId->constBegin();
        while (other.m_pBodySelectedPrimitvesId->constEnd() != iSet)
		{
			// Copy the current body set of id of selected primitive
			m_pBodySelectedPrimitvesId->insert(iSet.key(), new QSet<GLC_uint>(*(iSet.value())));
			++iSet;
		}
	}

	// Copy of the overwrite primitive materials maps
    if (nullptr != other.m_pOverwritePrimitiveMaterialMaps)
	{
		// Copy the hash table of overwrite materials
		m_pOverwritePrimitiveMaterialMaps= new QHash<int, QHash<GLC_uint, GLC_Material*>* >;
        QHash<int, QHash<GLC_uint, GLC_Material*>* >::const_iterator iMatMaps= other.m_pOverwritePrimitiveMaterialMaps->constBegin();
        while (other.m_pOverwritePrimitiveMaterialMaps->constEnd() != iMatMaps)
		{
			QHash<GLC_uint, GLC_Material*>* pBodyMatMap= new QHash<GLC_uint, GLC_Material*>(*(iMatMaps.value()));
			m_pOverwritePrimitiveMaterialMaps->insert(iMatMaps.key(), pBodyMatMap);
			++iMatMaps;
		}
	}

	// Update material usage
	QHash<GLC_Material*, int>::iterator iMatUsage= m_MaterialsUsage.begin();
	while (m_MaterialsUsage.constEnd() != iMatUsage)
	{
		iMatUsage.key()->addUsage(m_Uid);
		++iMatUsage;
	}
}


// Destructor
GLC_RenderProperties::~GLC_RenderProperties()
{
    clear();
}

bool GLC_RenderProperties::operator==(const GLC_RenderProperties &other) const
{
    bool subject= (this == &other);

    if (!subject)
    {
        subject= (m_IsSelected == other.m_IsSelected);
        subject= subject && (m_CurrentBody == other.m_CurrentBody);
        subject= subject && (m_RenderingFlag == other.m_RenderingFlag);
        subject= subject && (m_RenderMode == other.m_RenderMode);
        subject= subject && (m_Selectable == other.m_Selectable);
        subject= subject && fuzzyEquals(other);
    }

    return subject;
}

bool GLC_RenderProperties::fuzzyEquals(const GLC_RenderProperties &other) const
{
    bool subject= (this == &other);

    if (!subject)
    {
        subject= (m_PolyFace == other.m_PolyFace);
        subject= subject && (m_PolyMode == other.m_PolyMode);
        subject= subject && (m_SavedRenderMode == other.m_SavedRenderMode);
        subject= subject && (m_pOverwriteMaterial == other.m_pOverwriteMaterial);
        subject= subject && (glc::compare(m_OverwriteOpacity, other.m_OverwriteOpacity));
        subject= subject && (m_pBodySelectedPrimitvesId == other.m_pBodySelectedPrimitvesId);
        subject= subject && (m_pOverwritePrimitiveMaterialMaps == other.m_pOverwritePrimitiveMaterialMaps);
        subject= subject && (m_Selectable == other.m_Selectable);
        subject= subject && (m_OverwriteRenderingFlag == other.m_OverwriteRenderingFlag);
    }

    return subject;
}



// Return true if rendering properties needs to render with transparency
bool GLC_RenderProperties::needToRenderWithTransparency() const
{
	bool renderWithTransparency= false;
	if (m_RenderMode == glc::OverwriteMaterial)
	{
        Q_ASSERT(nullptr != m_pOverwriteMaterial);
		renderWithTransparency= m_pOverwriteMaterial->isTransparent();
	}
	else if ((m_RenderMode == glc::OverwriteTransparency) || (m_RenderMode == glc::OverwriteTransparencyAndMaterial))
    {
        Q_ASSERT(!qFuzzyCompare(-1.0f, m_OverwriteOpacity));
		renderWithTransparency= (m_OverwriteOpacity < 1.0f);
	}
	else if ((m_RenderMode == glc::OverwritePrimitiveMaterial)
            || ((m_RenderMode == glc::PrimitiveSelected) && (nullptr != m_pOverwritePrimitiveMaterialMaps) && (!m_pOverwritePrimitiveMaterialMaps->isEmpty())))
	{
        Q_ASSERT(nullptr != m_pOverwritePrimitiveMaterialMaps);
		Q_ASSERT(!m_pOverwritePrimitiveMaterialMaps->isEmpty());

		QList<QHash<GLC_uint, GLC_Material* >* > hashList= m_pOverwritePrimitiveMaterialMaps->values();
		QSet<GLC_Material*> materialSet;
		const int size= hashList.size();
		for (int i= 0; i < size; ++i)
		{
            const QList<GLC_Material*> materialList(hashList.at(i)->values());
            materialSet.unite(QSet<GLC_Material*>(materialList.begin(), materialList.end()));
		}

		QSet<GLC_Material*>::const_iterator iMat= materialSet.constBegin();
		while ((materialSet.constEnd() != iMat) && !renderWithTransparency)
		{
			renderWithTransparency= (*iMat)->isTransparent();
			++iMat;
		}
	}

	return renderWithTransparency;
}

bool GLC_RenderProperties::isDefault() const
{
    bool isDefault= (nullptr == m_pOverwriteMaterial);
	isDefault= isDefault && (m_OverwriteOpacity == -1.0f);
	return isDefault;
}

GLC_RenderProperties& GLC_RenderProperties::operator=(const GLC_RenderProperties& renderProperties)
{
    if (this != &renderProperties)
    {
        fuzzyAssignement(renderProperties);
        m_IsSelected= renderProperties.m_IsSelected;
        m_CurrentBody= renderProperties.m_CurrentBody;
    }

    return *this;
}

GLC_RenderProperties &GLC_RenderProperties::fuzzyAssignement(const GLC_RenderProperties &other)
{
    if (this != &other)
    {
        clear();
        m_PolyFace= other.m_PolyFace;
        m_PolyMode= other.m_PolyMode;
        m_RenderMode= other.m_RenderMode;
        m_SavedRenderMode= other.m_SavedRenderMode;
        m_pOverwriteMaterial= other.m_pOverwriteMaterial;
        m_OverwriteOpacity= other.m_OverwriteOpacity;
        m_pBodySelectedPrimitvesId= nullptr;
        m_pOverwritePrimitiveMaterialMaps= nullptr;
        m_RenderingFlag= other.m_RenderingFlag;

        // Update overwrite material usage
        if (nullptr != m_pOverwriteMaterial)
        {
            m_pOverwriteMaterial->addUsage(m_Uid);
        }

        // Copy the Hash of set of id of selected primitives
        if (nullptr != other.m_pBodySelectedPrimitvesId)
        {
            m_pBodySelectedPrimitvesId= new QHash<int, QSet<GLC_uint>* >();
            QHash<int, QSet<GLC_uint>* >::const_iterator iSet= other.m_pBodySelectedPrimitvesId->constBegin();
            while (other.m_pBodySelectedPrimitvesId->constEnd() != iSet)
            {
                // Copy the current body set of id of selected primitive
                m_pBodySelectedPrimitvesId->insert(iSet.key(), new QSet<GLC_uint>(*(iSet.value())));
                ++iSet;
            }
        }

        // Update primitive overwrite material usage
        if (nullptr != other.m_pOverwritePrimitiveMaterialMaps)
        {
            // Copy the hash table of overwrite materials
            m_pOverwritePrimitiveMaterialMaps= new QHash<int, QHash<GLC_uint, GLC_Material*>* >;
            QHash<int, QHash<GLC_uint, GLC_Material*>* >::const_iterator iMatMaps= other.m_pOverwritePrimitiveMaterialMaps->constBegin();
            while (other.m_pOverwritePrimitiveMaterialMaps->constEnd() != iMatMaps)
            {
                QHash<GLC_uint, GLC_Material*>* pBodyMatMap= new QHash<GLC_uint, GLC_Material*>(*(iMatMaps.value()));
                m_pOverwritePrimitiveMaterialMaps->insert(iMatMaps.key(), pBodyMatMap);

                QHash<GLC_uint, GLC_Material*>::iterator iMatMap= pBodyMatMap->begin();
                while (pBodyMatMap->constEnd() != iMatMap)
                {
                    iMatMap.value()->addUsage(m_Uid);
                    ++iMatMap;
                }

                ++iMatMaps;
            }
        }
    }

    m_Selectable= other.m_Selectable;

    return *this;

}

void GLC_RenderProperties::clear()
{
    if (nullptr != m_pOverwriteMaterial)
	{
		// Delete the material if it is unused
		m_pOverwriteMaterial->delUsage(m_Uid);
		if (m_pOverwriteMaterial->isUnused()) delete m_pOverwriteMaterial;
        m_pOverwriteMaterial= nullptr;
	}

	clearSelectedPrimitives();

	clearOverwritePrimitiveMaterials();
}

// Set the overwrite material
void GLC_RenderProperties::setOverwriteMaterial(GLC_Material* pMaterial)
{
    Q_ASSERT(nullptr != pMaterial);
    if (nullptr != m_pOverwriteMaterial)
	{
		m_pOverwriteMaterial->delUsage(m_Uid);
		if (m_pOverwriteMaterial->isUnused()) delete m_pOverwriteMaterial;
	}
	m_pOverwriteMaterial= pMaterial;

	m_pOverwriteMaterial->addUsage(m_Uid);
}

// Return true if the specified primitive id of the specified body index is selected
bool GLC_RenderProperties::primitiveIsSelected(int index, GLC_uint id) const
{
	bool result= false;
    if ((nullptr != m_pBodySelectedPrimitvesId) && m_pBodySelectedPrimitvesId->contains(m_CurrentBody))
	{
		result= m_pBodySelectedPrimitvesId->value(index)->contains(id);
	}
	return result;
}

// Set the list of selected primitives id
void GLC_RenderProperties::addSetOfSelectedPrimitivesId(const QSet<GLC_uint>& set, int body)
{
    if (nullptr == m_pBodySelectedPrimitvesId)
	{
		m_pBodySelectedPrimitvesId= new QHash<int, QSet<GLC_uint>* >();
		m_pBodySelectedPrimitvesId->insert(body, new QSet<GLC_uint>(set));
	}
	else if (!m_pBodySelectedPrimitvesId->contains(body))
	{
		m_pBodySelectedPrimitvesId->insert(body, new QSet<GLC_uint>(set));
	}
	else
	{
		m_pBodySelectedPrimitvesId->value(body)->unite(set);
	}
}

// Add a selected primitive
void GLC_RenderProperties::addSelectedPrimitive(GLC_uint id, int body)
{
    if (nullptr == m_pBodySelectedPrimitvesId)
	{
		m_pBodySelectedPrimitvesId= new QHash<int, QSet<GLC_uint>* >();
		m_pBodySelectedPrimitvesId->insert(body, new QSet<GLC_uint>());

	}
	else if (!m_pBodySelectedPrimitvesId->contains(body))
	{
		m_pBodySelectedPrimitvesId->insert(body, new QSet<GLC_uint>());
	}
	m_pBodySelectedPrimitvesId->value(body)->insert(id);
}

// Clear selectedPrimitive Set
void GLC_RenderProperties::clearSelectedPrimitives()
{
    if (nullptr != m_pBodySelectedPrimitvesId)
	{
		QHash<int, QSet<GLC_uint>* >::const_iterator iSet= m_pBodySelectedPrimitvesId->constBegin();
		while (m_pBodySelectedPrimitvesId->constEnd() != iSet)
		{
			delete iSet.value();
			++iSet;
		}
	}
	delete m_pBodySelectedPrimitvesId;
    m_pBodySelectedPrimitvesId= nullptr;
}

// Add an overwrite primitive material
void GLC_RenderProperties::addOverwritePrimitiveMaterial(GLC_uint id, GLC_Material* pMaterial, int bodyIndex)
{
    Q_ASSERT(nullptr != pMaterial);
    if (nullptr != m_pOverwritePrimitiveMaterialMaps)
	{
		if (m_pOverwritePrimitiveMaterialMaps->contains(bodyIndex))
		{
			QHash<GLC_uint, GLC_Material*>* pHash= m_pOverwritePrimitiveMaterialMaps->value(bodyIndex);
			if (pHash->contains(id))
			{
				if (pHash->value(id) != pMaterial)
				{
					GLC_Material* pOldMaterial= pHash->value(id);
					unUseMaterial(pOldMaterial);
					pHash->remove(id);

					pHash->insert(id, pMaterial);
					useMaterial(pMaterial);
				}
				// Else, noting to do
			}
			else
			{
				pHash->insert(id, pMaterial);
				useMaterial(pMaterial);
			}
		}
		else
		{
			QHash<GLC_uint, GLC_Material*>* pHash= new QHash<GLC_uint, GLC_Material*>();
			pHash->insert(id, pMaterial);
			useMaterial(pMaterial);
			m_pOverwritePrimitiveMaterialMaps->insert(bodyIndex, pHash);
		}
	}
	else
	{
		m_pOverwritePrimitiveMaterialMaps= new QHash<int, QHash<GLC_uint, GLC_Material*>* >();
		QHash<GLC_uint, GLC_Material*>* pHash= new QHash<GLC_uint, GLC_Material*>();
		pHash->insert(id, pMaterial);
		m_pOverwritePrimitiveMaterialMaps->insert(bodyIndex, pHash);
		useMaterial(pMaterial);
	}
}

// Clear overwrite primitive materials
void GLC_RenderProperties::clearOverwritePrimitiveMaterials()
{
    if (nullptr != m_pOverwritePrimitiveMaterialMaps)
	{
		Q_ASSERT(!m_MaterialsUsage.isEmpty());
		QHash<int, QHash<GLC_uint, GLC_Material* >* >::iterator iHash= m_pOverwritePrimitiveMaterialMaps->begin();
		while (m_pOverwritePrimitiveMaterialMaps->constEnd() != iHash)
		{
			delete iHash.value();
			++iHash;
		}

		QHash<GLC_Material*, int>::iterator iMat= m_MaterialsUsage.begin();
		while (m_MaterialsUsage.constEnd() != iMat)
		{
			GLC_Material* pMat= iMat.key();
			pMat->delUsage(m_Uid);
			if (pMat->isUnused()) delete pMat;
			++iMat;
		}
		m_MaterialsUsage.clear();
	}
	else
	{
		Q_ASSERT(m_MaterialsUsage.isEmpty());
	}

	delete m_pOverwritePrimitiveMaterialMaps;
    m_pOverwritePrimitiveMaterialMaps= nullptr;

    if (m_SavedRenderMode == glc::OverwritePrimitiveMaterial)
	{
		m_SavedRenderMode= glc::NormalRenderMode;
	}
}

