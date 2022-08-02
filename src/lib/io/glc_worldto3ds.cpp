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
//! \file glc_worldto3ds.cpp implementation of the GLC_WorldTo3ds class.

#include <QFile>
#include <QtDebug>

#include "../geometry/glc_3drep.h"
#include "../geometry/glc_geometry.h"
#include "../geometry/glc_mesh.h"

#include "../shading/glc_material.h"

// Lib3ds Header
#include "3rdparty/lib3ds/file.h"
#include "3rdparty/lib3ds/mesh.h"
#include "3rdparty/lib3ds/node.h"
#include "3rdparty/lib3ds/matrix.h"
#include "3rdparty/lib3ds/material.h"

#include "glc_worldto3ds.h"

static const int maxNameLength= 12;

GLC_WorldTo3ds::GLC_WorldTo3ds(const GLC_World& world, bool exportAll)
    : QObject()
    , m_World(world)
    , m_pLib3dsFile(nullptr)
    , m_FileName()
    , m_ReferenceToMesh()
    , m_WorldMaterialIdTo3dsMaterial()
    , m_pRootLib3dsNode(nullptr)
    , m_CurrentNodeId(0)
    , m_OccIdToNodeId()
    , m_CurrentMeshIndex(0)
    , m_CurrentMaterialIndex(0)
    , m_CurrentTextureIndex(0)
    , m_UseAbsolutePosition(false)
    , m_TextureToFileName()
    , m_ExportAll(exportAll)
{
}

GLC_WorldTo3ds::~GLC_WorldTo3ds()
{

}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
bool GLC_WorldTo3ds::exportToFile(const QString& fileName, bool useAbsolutePosition)
{
    m_ReferenceToMesh.clear();
    m_WorldMaterialIdTo3dsMaterial.clear();
    m_pRootLib3dsNode= nullptr;
	m_CurrentNodeId= 0;
	m_OccIdToNodeId.clear();
	m_CurrentMeshIndex= 0;
	m_UseAbsolutePosition= useAbsolutePosition;
	m_TextureToFileName.clear();

	m_FileName= fileName;
	bool subject= false;
	{
		QFile exportFile(m_FileName);
		subject= exportFile.open(QIODevice::WriteOnly);
		exportFile.close();
	}
	if (subject)
	{
		m_pLib3dsFile= lib3ds_file_new();
		saveWorld();
        subject= lib3ds_file_save(m_pLib3dsFile, fileName.toLatin1().data());
	}

	return subject;
}


//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////

void GLC_WorldTo3ds::saveWorld()
{
	// Save node structure
	GLC_StructOccurrence* pRoot= m_World.rootOccurrence();
	const int childCount= pRoot->childCount();
	for (int i= 0; i < childCount; ++i)
	{
		saveBranch(pRoot->child(i));
	}
}

QList<Lib3dsMesh*> GLC_WorldTo3ds::saveMeshes(GLC_StructReference* pRef)
{
    QList<Lib3dsMesh*> subject;
    if (pRef->hasRepresentation())
    {
        GLC_3DRep* pRep= dynamic_cast<GLC_3DRep*>(pRef->representationHandle());
        if (nullptr != pRep)
        {
            // This reference has a mesh
            subject= createMeshsFrom3DRep(pRep, "MESH");
            for (Lib3dsMesh* pMesh : qAsConst(subject))
            {
                lib3ds_file_insert_mesh(m_pLib3dsFile, pMesh);
                m_ReferenceToMesh.insert(pRef, pMesh);
            }
        }
    }

    return subject;
}

void GLC_WorldTo3ds::saveBranch(GLC_StructOccurrence* pOcc)
{
    if (pOcc->isVisible() || m_ExportAll)
    {
        if (pOcc->structReference()->hasRepresentation() || !m_UseAbsolutePosition)
        {
            createNodeFromOccurrence(pOcc);
        }

        const int childCount= pOcc->childCount();
        for (int i= 0; i < childCount; ++i)
        {
            saveBranch(pOcc->child(i));
        }
    }
}

void GLC_WorldTo3ds::createNodeFromOccurrence(GLC_StructOccurrence* pOcc)
{
	Lib3dsNode* p3dsNode = lib3ds_node_new_object();
	p3dsNode->node_id= m_CurrentNodeId;
	m_OccIdToNodeId.insert(pOcc->id(), m_CurrentNodeId++);

    if (m_UseAbsolutePosition || (pOcc->parent() == m_World.rootOccurrence()))
	{
		p3dsNode->parent_id= LIB3DS_NO_PARENT;
	}
	else
	{
        Q_ASSERT(m_OccIdToNodeId.contains(pOcc->parent()->id()));
        p3dsNode->parent_id= static_cast<unsigned short>(m_OccIdToNodeId.value(pOcc->parent()->id()));
	}

	GLC_StructReference* pRef= pOcc->structReference();
	if (m_UseAbsolutePosition)
    {
        if (pOcc->structReference()->hasRepresentation())
		{
			GLC_3DRep* pRep= dynamic_cast<GLC_3DRep*>(pOcc->structReference()->representationHandle());
            if (nullptr != pRep)
			{
				// This reference has a mesh
				const GLC_Matrix4x4 matrix= pOcc->absoluteMatrix();
                QList<Lib3dsMesh*> meshes= createMeshsFrom3DRep(pRep, "MESH", matrix);

				const int meshCount= meshes.count();
				for (int i= 0; i < meshCount; ++i)
				{
					lib3ds_file_insert_mesh(m_pLib3dsFile, meshes.at(i));
				}

				if (meshCount > 1)
				{
					for (int i= 0; i < meshCount; ++i)
					{

						Lib3dsNode* pCurrent3dsNode = lib3ds_node_new_object();
						pCurrent3dsNode->node_id= m_CurrentNodeId++;
						pCurrent3dsNode->parent_id= p3dsNode->node_id;

                        strncpy(pCurrent3dsNode->name, meshes.at(i)->name, sizeof (pCurrent3dsNode->name));
						lib3ds_file_insert_node(m_pLib3dsFile, pCurrent3dsNode);
					}
				}
				else if (!meshes.isEmpty())
				{
                    strncpy(p3dsNode->name, meshes.constFirst()->name, sizeof (p3dsNode->name));
                    lib3ds_file_insert_node(m_pLib3dsFile, p3dsNode);
				}
                else
                {
                    delete p3dsNode;
                }
			}
		}
	}
	else
	{
		// Node matrix
		const GLC_Matrix4x4 matrix= pOcc->structInstance()->relativeMatrix();
		setNodePosition(p3dsNode, matrix);
        lib3ds_file_insert_node(m_pLib3dsFile, p3dsNode);

		// Set mesh name if necessary
        QList<Lib3dsMesh*> meshes;
        if (m_ReferenceToMesh.contains(pRef))
		{

            meshes= m_ReferenceToMesh.values(pRef);
		}
        else
        {
            meshes= saveMeshes(pRef);
        }
        const int meshCount= meshes.count();
        if (meshCount > 1)
        {
            for (int i= 0; i < meshCount; ++i)
            {

                Lib3dsNode* pCurrent3dsNode = lib3ds_node_new_object();
                pCurrent3dsNode->node_id= m_CurrentNodeId++;
                pCurrent3dsNode->parent_id= p3dsNode->node_id;

                strncpy(pCurrent3dsNode->name, meshes.at(i)->name, sizeof (pCurrent3dsNode->name));
                lib3ds_file_insert_node(m_pLib3dsFile, pCurrent3dsNode);
            }
        }
        else
        {
            strcpy(p3dsNode->name, m_ReferenceToMesh.value(pRef)->name);
        }

	}
}

QList<Lib3dsMesh*> GLC_WorldTo3ds::createMeshsFrom3DRep(GLC_3DRep* pRep, const QString& name, const GLC_Matrix4x4& matrix)
{
	QList<Lib3dsMesh*> subject;
	const int bodyCount= pRep->numberOfBody();
	for (int i= 0; i < bodyCount; ++i)
	{
		GLC_Mesh* pCurrentMesh= dynamic_cast<GLC_Mesh*>(pRep->geomAt(i));
        if ((nullptr != pCurrentMesh) && !pCurrentMesh->isEmpty())
		{
			bool deleteCurrentMesh= false;
			if (pCurrentMesh->lodCount() > 1)
			{
				// Keep only the first level of detail
				pCurrentMesh= pCurrentMesh->createMeshOfGivenLod(0);
				deleteCurrentMesh= true;
			}
            const QString bodyMeshName= to3dsName(name, ++m_CurrentMeshIndex);
			if (matrix.type() != GLC_Matrix4x4::Identity)
			{
				if (!deleteCurrentMesh)
				{
					pCurrentMesh= new GLC_Mesh(*pCurrentMesh);
					deleteCurrentMesh= true;
				}
				pCurrentMesh->transformVertice(matrix);
				Q_ASSERT(!pCurrentMesh->isEmpty());
			}
            Lib3dsMesh* p3dsMesh= create3dsMeshFromGLC_Mesh(pCurrentMesh, bodyMeshName);

			if (deleteCurrentMesh) delete pCurrentMesh;
			subject.append(p3dsMesh);
		}
	}

	return subject;
}

Lib3dsMesh* GLC_WorldTo3ds::create3dsMeshFromGLC_Mesh(GLC_Mesh* pMesh, const QString& meshName)
{
	// Create empty 3ds mesh with the given name
    Lib3dsMesh* p3dsMesh= lib3ds_mesh_new(meshName.toLatin1().data());

	const int stride= 3;

	GLfloatVector vertice= pMesh->positionVector();
    const int pointsCount= vertice.count() / stride;

    // Add points to the 3DS mesh
    lib3ds_mesh_new_point_list(p3dsMesh, static_cast<uint>(pointsCount));

    for (int i= 0; i < pointsCount; ++i)
	{
		Lib3dsPoint point;
		point.pos[0]= vertice[i * 3];
		point.pos[1]= vertice[i * 3 + 1];
		point.pos[2]= vertice[i * 3 + 2];

		p3dsMesh->pointL[i]= point;
	}

	// Add texel to the 3DS mesh
	GLfloatVector texelVector= pMesh->texelVector();
	if(!texelVector.isEmpty())
    {
        lib3ds_mesh_new_texel_list(p3dsMesh, static_cast<uint>(pointsCount));
        for (int i= 0; i < pointsCount; ++i)
        {
            p3dsMesh->texelL[i][0]= texelVector[i * 2];
            p3dsMesh->texelL[i][1]= texelVector[i * 2 + 1];
		}
	}

	// Add faces to the 3ds mesh
	const uint totalFaceCount= pMesh->faceCount(0);
	lib3ds_mesh_new_face_list(p3dsMesh, totalFaceCount);

	QSet<GLC_Material*> materialSet= pMesh->materialSet();
	QSet<GLC_Material*>::iterator iMat= materialSet.begin();
	uint currentFaceIndex= 0;
	while(iMat != materialSet.end())
	{
		GLC_Material* pCurrentGLCMat= *iMat;
		Lib3dsMaterial* pMaterial= get3dsMaterialFromGLC_Material(pCurrentGLCMat);
		IndexList currentTriangleIndex= pMesh->getEquivalentTrianglesStripsFansIndex(0, pCurrentGLCMat->id());
		const int faceCount= currentTriangleIndex.count() / 3;
		for (int i= 0; i < faceCount; ++i)
		{
			Lib3dsFace face;
            strncpy(face.material, pMaterial->name, sizeof (face.material));

            face.points[0]= static_cast<unsigned short>(currentTriangleIndex.at(i * 3));
            face.points[1]= static_cast<unsigned short>(currentTriangleIndex.at(i * 3 + 1));
            face.points[2]= static_cast<unsigned short>(currentTriangleIndex.at(i * 3 + 2));

			p3dsMesh->faceL[currentFaceIndex++]= face;
			Q_ASSERT(currentFaceIndex <= totalFaceCount);
		}
		++iMat;
	}

	return p3dsMesh;
}

Lib3dsMaterial* GLC_WorldTo3ds::get3dsMaterialFromGLC_Material(GLC_Material* pMat)
{
    Lib3dsMaterial* pSubject= nullptr;

    if (m_WorldMaterialIdTo3dsMaterial.contains(pMat->id()))
	{
        pSubject= m_WorldMaterialIdTo3dsMaterial.value(pMat->id());
	}
	else
	{
        pSubject= create3dsMaterialFromGLC_Material(pMat);
	}

	return pSubject;
}

Lib3dsMaterial* GLC_WorldTo3ds::create3dsMaterialFromGLC_Material(GLC_Material* pMat)
{
    const QString matName= to3dsName("MAT", ++m_CurrentMaterialIndex);
	Lib3dsMaterial* pSubject= lib3ds_material_new();
    strncpy(pSubject->name, matName.toLatin1().data(), sizeof (pSubject->name));


	// Ambient Color
	QColor ambient= pMat->ambientColor();
	pSubject->ambient[0]= static_cast<float>(ambient.redF());
	pSubject->ambient[1]= static_cast<float>(ambient.greenF());
	pSubject->ambient[2]= static_cast<float>(ambient.blueF());
	pSubject->ambient[3]= static_cast<float>(ambient.alphaF());

	// Diffuse Color
	QColor diffuse= pMat->diffuseColor();
	pSubject->diffuse[0]= static_cast<float>(diffuse.redF());
	pSubject->diffuse[1]= static_cast<float>(diffuse.greenF());
	pSubject->diffuse[2]= static_cast<float>(diffuse.blueF());
	pSubject->diffuse[3]= static_cast<float>(diffuse.alphaF());

	// Specular Color
	QColor specular= pMat->specularColor();
	pSubject->specular[0]= static_cast<float>(specular.redF());
	pSubject->specular[1]= static_cast<float>(specular.greenF());
	pSubject->specular[2]= static_cast<float>(specular.blueF());
	pSubject->specular[3]= static_cast<float>(specular.alphaF());


	// Shininess
	pSubject->shininess= pMat->shininess();

	// Transparency

	pSubject->transparency= 1.0f - static_cast<float>(pMat->opacity());

	// Texture
    if (pMat->hasTexture())
	{
        GLC_Texture* pTexture= pMat->textureHandle();
        QString sourceTextureKey= pTexture->fileName();
        QString targetTextureFileName= "TEX" + QString::number(++m_CurrentTextureIndex);

        if (sourceTextureKey.isEmpty())
        {
            sourceTextureKey= targetTextureFileName;
        }

        if (!m_TextureToFileName.contains(sourceTextureKey))
		{
			QString filePath= QFileInfo(m_FileName).absolutePath();
            QImage textureImage;
            if (QFileInfo(sourceTextureKey).exists())
            {
                textureImage.load(pTexture->fileName());
            }
            else
            {
                textureImage= pTexture->imageOfTexture();
            }

            targetTextureFileName= targetTextureFileName + ".jpg";
            targetTextureFileName= targetTextureFileName.right(maxNameLength);

            if (!textureImage.isNull())
			{
                const QString type(QFileInfo(targetTextureFileName).suffix());
                QString newTextureFile= filePath + '/' + targetTextureFileName;
				textureImage.save(newTextureFile, type.toUpper().toLatin1().data());
                strcpy(pSubject->texture1_map.name, targetTextureFileName.toLatin1().data());
                m_TextureToFileName.insert(sourceTextureKey, targetTextureFileName);
			}
		}
		else
		{
            --m_CurrentTextureIndex;
            QString textureName= m_TextureToFileName.value(sourceTextureKey);
            strcpy(pSubject->texture1_map.name, textureName.toLatin1().data());
		}

	}

	lib3ds_file_insert_material(m_pLib3dsFile, pSubject);
    m_WorldMaterialIdTo3dsMaterial.insert(pMat->id(), pSubject);

	return pSubject;
}

QString GLC_WorldTo3ds::to3dsName(const QString &name, int id) const
{
    QString subject;
    const QString idString(QString::number(id));
    if ((name.length() + idString.length()) > maxNameLength)
    {
        const int newSize= maxNameLength - idString.length();
        const QString baseName(name.left(newSize));
        subject= baseName + idString;
        Q_ASSERT(subject.length() == maxNameLength);
    }
    else
    {
        subject= name + idString;
    }

    subject.replace(" ", "_");

    return subject;
}

void GLC_WorldTo3ds::setNodePosition(Lib3dsNode* pNode, const GLC_Matrix4x4& matrix)
{
	Lib3dsObjectData *pObjectData= &pNode->data.object;

	GLC_Matrix4x4 isoMatrix(matrix.isometricMatrix());
	// Translation
    Lib3dsLin3Key* pLin3Key= lib3ds_lin3_key_new();
    pLin3Key->value[0]= static_cast<float>(isoMatrix.getData()[12]);
    pLin3Key->value[1]= static_cast<float>(isoMatrix.getData()[13]);
    pLin3Key->value[2]= static_cast<float>(isoMatrix.getData()[14]);

	pLin3Key->tcb.frame= 1;
	pObjectData->pos_track.keyL= pLin3Key;


	// Scaling
	Lib3dsLin3Key* pScale3Key= lib3ds_lin3_key_new();
	pScale3Key->value[0]= static_cast<float>(matrix.scalingX());
	pScale3Key->value[1]= static_cast<float>(matrix.scalingY());
	pScale3Key->value[2]= static_cast<float>(matrix.scalingZ());

	pScale3Key->tcb.frame= 1;
	pObjectData->scl_track.keyL= pScale3Key;

	// Rotation

	Lib3dsQuatKey* pQuatKey= lib3ds_quat_key_new();

	QPair<GLC_Vector3d, double> pair= matrix.rotationVectorAndAngle();

	pQuatKey->angle= static_cast<float>(pair.second);
	pQuatKey->axis[0]= static_cast<float>(pair.first.x());
	pQuatKey->axis[1]= static_cast<float>(pair.first.y());
	pQuatKey->axis[2]= static_cast<float>(pair.first.z());

	pQuatKey->tcb.frame= 1;

	pObjectData->rot_track.keyL= pQuatKey;

}
