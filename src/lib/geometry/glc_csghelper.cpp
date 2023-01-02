/*
 *  glc_csghelper.cpp
 *
 *  Created on: 15/07/2016
 *      Author: Laurent Ribon
 */
#include "glc_csghelper.h"

#include "../3rdparty/csgjs/csgjs.h"

#include "../maths/glc_matrix4x4.h"
#include "../maths/glc_vector3d.h"

#include "glc_mesh.h"

GLC_CsgHelper::GLC_CsgHelper()
{

}

GLC_Mesh*GLC_CsgHelper::intersection(const GLC_Mesh* pMesh1, const GLC_Matrix4x4& m1, const GLC_Mesh* pMesh2, const GLC_Matrix4x4& m2)
{
    csgjs_model* pCsgModel1= csgModelFromMesh(pMesh1, m1);
    csgjs_model* pCsgModel2= csgModelFromMesh(pMesh2, m2);

    csgjs_model result= csgjs_intersection(*pCsgModel1, *pCsgModel2);

    GLC_Mesh* pSubject= meshFromCsgModel(result, materialHash(pMesh1, pMesh2));

    delete pCsgModel1;
    delete pCsgModel2;

    return pSubject;
}

void GLC_CsgHelper::intersection(GLC_Mesh* pResultMesh, const GLC_Mesh* pMesh1, const GLC_Matrix4x4& m1, const GLC_Mesh* pMesh2, const GLC_Matrix4x4& m2)
{
    csgjs_model* pCsgModel1= csgModelFromMesh(pMesh1, m1);
    csgjs_model* pCsgModel2= csgModelFromMesh(pMesh2, m2);

    csgjs_model result= csgjs_intersection(*pCsgModel1, *pCsgModel2);

    meshFromCsgModel(result, materialHash(pMesh1, pMesh2), pResultMesh);

    pResultMesh->addVerticeGroups(*pMesh1, m1);
    pResultMesh->addVerticeGroups(*pMesh2, m2);

    delete pCsgModel1;
    delete pCsgModel2;
}

GLC_Mesh*GLC_CsgHelper::add(const GLC_Mesh* pMesh1, const GLC_Matrix4x4& m1, const GLC_Mesh* pMesh2, const GLC_Matrix4x4& m2)
{
    csgjs_model* pCsgModel1= csgModelFromMesh(pMesh1, m1);
    csgjs_model* pCsgModel2= csgModelFromMesh(pMesh2, m2);

    csgjs_model result= csgjs_union(*pCsgModel1, *pCsgModel2);

    GLC_Mesh* pSubject= meshFromCsgModel(result, materialHash(pMesh1, pMesh2));

    delete pCsgModel1;
    delete pCsgModel2;

    return pSubject;
}

void GLC_CsgHelper::add(GLC_Mesh* pResultMesh, const GLC_Mesh* pMesh1, const GLC_Matrix4x4& m1, const GLC_Mesh* pMesh2, const GLC_Matrix4x4& m2)
{
    csgjs_model* pCsgModel1= csgModelFromMesh(pMesh1, m1);
    csgjs_model* pCsgModel2= csgModelFromMesh(pMesh2, m2);

    csgjs_model result= csgjs_union(*pCsgModel1, *pCsgModel2);

    meshFromCsgModel(result, materialHash(pMesh1, pMesh2), pResultMesh);

    pResultMesh->addVerticeGroups(*pMesh1, m1);
    pResultMesh->addVerticeGroups(*pMesh2, m2);

    delete pCsgModel1;
    delete pCsgModel2;
}

GLC_Mesh *GLC_CsgHelper::soustract(const GLC_Mesh *pMesh1, const GLC_Matrix4x4& m1, const GLC_Mesh *pMesh2, const GLC_Matrix4x4& m2)
{
    csgjs_model* pCsgModel1= csgModelFromMesh(pMesh1, m1);
    csgjs_model* pCsgModel2= csgModelFromMesh(pMesh2, m2);

    csgjs_model result= csgjs_difference(*pCsgModel1, *pCsgModel2);

    GLC_Mesh* pSubject= meshFromCsgModel(result, materialHash(pMesh1, pMesh2));

    delete pCsgModel1;
    delete pCsgModel2;

    return pSubject;
}

void GLC_CsgHelper::soustract(GLC_Mesh* pResultMesh, const GLC_Mesh* pMesh1, const GLC_Matrix4x4& m1, const GLC_Mesh* pMesh2, const GLC_Matrix4x4& m2)
{
    csgjs_model* pCsgModel1= csgModelFromMesh(pMesh1, m1);
    csgjs_model* pCsgModel2= csgModelFromMesh(pMesh2, m2);

    csgjs_model result= csgjs_difference(*pCsgModel1, *pCsgModel2);

    meshFromCsgModel(result, materialHash(pMesh1, pMesh2), pResultMesh);

    pResultMesh->addVerticeGroups(*pMesh1, m1);
    pResultMesh->addVerticeGroups(*pMesh2, m2);

    delete pCsgModel1;
    delete pCsgModel2;
}

csgjs_model *GLC_CsgHelper::csgModelFromMesh(const GLC_Mesh *pMesh, const GLC_Matrix4x4& matrix)
{
    csgjs_model* pSubject= new csgjs_model;

    const GLfloatVector& positionVector= pMesh->positionVector();
    const GLfloatVector& normalVector= pMesh->normalVector();
    const GLfloatVector& texelVector= pMesh->texelVector();

    QList<GLC_uint> materialIdList= pMesh->materialIds();
    const int materialCount= materialIdList.count();
    int newIndex= 0;
    for (int iMat= 0; iMat < materialCount; ++iMat)
    {
        const GLC_uint materialId= materialIdList.at(iMat);
        const IndexList indexList= pMesh->getEquivalentTrianglesStripsFansIndex(0, materialId);
        const int count= indexList.count();
        for (int i= 0; i < count; ++i)
        {
            const int index= indexList.at(i);
            GLC_Vector3d vectPos(positionVector.at(index * 3), positionVector.at((index * 3) + 1), positionVector.at((index * 3) + 2));
            GLC_Vector3d vectNormal(normalVector.at(index * 3), normalVector.at((index * 3) + 1), normalVector.at((index * 3) + 2));
            if (matrix.type() != GLC_Matrix4x4::Identity)
            {
                vectPos= matrix * vectPos;
                const GLC_Matrix4x4 rotationMatrix= matrix.rotationMatrix().optimise();
                vectNormal= rotationMatrix * vectNormal;
            }

            csgjs_vertex vertex;
            vertex.pos.x= static_cast<float>(vectPos.x());
            vertex.pos.y= static_cast<float>(vectPos.y());
            vertex.pos.z= static_cast<float>(vectPos.z());

            vertex.normal.x= static_cast<float>(vectNormal.x());
            vertex.normal.y= static_cast<float>(vectNormal.y());
            vertex.normal.z= static_cast<float>(vectNormal.z());

            vertex.matId= materialId;

            if (!texelVector.isEmpty())
            {
                vertex.uv.x= texelVector.at(index * 2);
                vertex.uv.y= texelVector.at((index * 2) + 1);
            }

            pSubject->vertices.push_back(vertex);
            pSubject->indices.push_back(newIndex++);
        }
    }

    return pSubject;
}

GLC_Mesh *GLC_CsgHelper::meshFromCsgModel(const csgjs_model& model, const QHash<GLC_uint, GLC_Material*>& materialHash)
{
    GLC_Mesh* pSubject= new GLC_Mesh;
    meshFromCsgModel(model, materialHash, pSubject);

    return pSubject;
}

void GLC_CsgHelper::meshFromCsgModel(const csgjs_model& model, const QHash<GLC_uint, GLC_Material*>& materialHash, GLC_Mesh* pMesh)
{
    Q_ASSERT(NULL != pMesh);
    pMesh->clear();
    QHash<GLC_uint, GLC_Material*>::ConstIterator iMat= materialHash.constBegin();
    QList<int> indices= model.indices;

    while (iMat != materialHash.constEnd())
    {
        IndexList indexList;
        const GLC_uint& matId= iMat.key();
        QList<int>::iterator iIndice= indices.begin();
        while (iIndice != indices.end())
        {
            if (model.vertices.at(*iIndice).matId == matId)
            {
                indexList.append(*iIndice);
                iIndice= indices.erase(iIndice);
            }
            else
            {
                ++iIndice;
            }
        }
        if (!indexList.isEmpty())
        {
            pMesh->addTriangles(iMat.value(), indexList);
        }

        ++iMat;
    }

    Q_ASSERT(indices.isEmpty());

    GLfloatVector positionVector;
    GLfloatVector normalVector;
    GLfloatVector texelVector;

    const size_t verticeCount= model.vertices.size();
    for (size_t i= 0; i < verticeCount; ++i)
    {
        csgjs_vertex vertex= model.vertices.at(i);
        positionVector << vertex.pos.x << vertex.pos.y << vertex.pos.z;
        normalVector << vertex.normal.x << vertex.normal.y << vertex.normal.z;
        texelVector << vertex.uv.x << vertex.uv.y;
    }
    pMesh->addVertice(positionVector);
    pMesh->addNormals(normalVector);
    pMesh->addTexels(texelVector);

    pMesh->finish();
}

QHash<GLC_uint, GLC_Material*> GLC_CsgHelper::materialHash(const GLC_Mesh* pMesh1, const GLC_Mesh* pMesh2)
{
    QHash<GLC_uint, GLC_Material*> mesh1Hash(pMesh1->materialHash());
    QHash<GLC_uint, GLC_Material*> subject(pMesh2->materialHash());

    QHash<GLC_uint, GLC_Material*>::ConstIterator iMatMesh1= mesh1Hash.constBegin();
    while(iMatMesh1 != mesh1Hash.constEnd())
    {
        if (!subject.contains(iMatMesh1.key()))
        {
            subject.insert(iMatMesh1.key(), iMatMesh1.value());
        }
        ++iMatMesh1;
    }

    return subject;
}
