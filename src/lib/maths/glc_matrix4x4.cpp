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
//! \file glc_matrix4x4.cpp implementation of the GLC_Matrix4x4 class.

#include "glc_matrix4x4.h"

#include <cmath>
#include <QVector4D>

#include <QtDebug>

using namespace std;

GLC_Matrix4x4 GLC_Matrix4x4::frustumMatrix(double left, double right, double bottom, double top, double nearVal, double farVal)
{
    const double a= (right + left) / (right - left);
    const double b= (top + bottom) / (top - bottom);
    const double c= - (farVal + nearVal) / (farVal - nearVal);
    const double d= - (2.0 * farVal * nearVal) / (farVal - nearVal);

    GLC_Matrix4x4 subject;

    subject.m_Matrix[0]= (2.0 * nearVal) / (right - left);
    subject.m_Matrix[5]= (2.0 * nearVal) / (top - bottom);
    subject.m_Matrix[8]= a;
    subject.m_Matrix[9]= b;
    subject.m_Matrix[10]= c;
    subject.m_Matrix[11]= -1.0;
    subject.m_Matrix[14]= d;
    subject.m_Matrix[15]= 0.0;

    subject.m_Type= General;
    return subject;
}

GLC_Matrix4x4 GLC_Matrix4x4::orthonormalMatrix(double left, double right, double bottom, double top, double nearVal, double farVal)
{
    const double tx= - (right + left) / (right - left);
    const double ty= - (top + bottom) / (top - bottom);
    const double tz= - (farVal + nearVal) / (farVal - nearVal);

    GLC_Matrix4x4 subject;
    subject.m_Matrix[0]= 2.0 / (right - left);
    subject.m_Matrix[5]= 2.0 / (top - bottom);
    subject.m_Matrix[10]= -2.0 / (farVal - nearVal);
    subject.m_Matrix[12]= tx;
    subject.m_Matrix[13]= ty;
    subject.m_Matrix[14]= tz;

    subject.m_Type= General;
    return subject;
}

GLC_Plane GLC_Matrix4x4::operator *(const GLC_Plane& plane) const
{

    QVector4D planeFactors(plane.coefA(), plane.coefB(), plane.coefC(), plane.coefD());
    QMatrix4x4 matrix(qMatrix());

    QVector4D newPlaneFactor= matrix.inverted().transposed() * planeFactors;
    GLC_Plane subject(newPlaneFactor.x(), newPlaneFactor.y(), newPlaneFactor.z(), newPlaneFactor.w());

    return subject;
}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////

GLC_Matrix4x4& GLC_Matrix4x4::fromEuler(const double angle_x, const double angle_y, const double angle_z)
{
    const double A= cos(angle_x);
    const double B= sin(angle_x);
    const double C= cos(angle_y);
    const double D= sin(angle_y);
    const double E= cos(angle_z);
    const double F= sin(angle_z);

    const double AD= A * D;
    const double BD= B * D;

    m_Matrix[0]  = C * E;
    m_Matrix[4]  = -C * F;
    m_Matrix[8]  = -D;
    m_Matrix[1]  = -BD * E + A * F;
    m_Matrix[5]  = BD * F + A * E;
    m_Matrix[9]  = -B * C;
    m_Matrix[2]  = AD * E + B * F;
    m_Matrix[6]  = -AD * F + B * E;
    m_Matrix[10] = A * C;

    m_Matrix[12]=  0.0; m_Matrix[13]= 0.0; m_Matrix[14]= 0.0; m_Matrix[3]= 0.0; m_Matrix[7]= 0.0; m_Matrix[11] = 0.0;
    m_Matrix[15] =  1.0;

    m_Type= Direct;

	return *this;
}

GLC_Matrix4x4& GLC_Matrix4x4::setColumn(int index, const GLC_Vector3d& vector)
{
	Q_ASSERT(index < 4);
	index= index * 4;
	m_Matrix[index]= vector.x();
	m_Matrix[index + 1]= vector.y();
	m_Matrix[index + 2]= vector.z();

	m_Type= General;

	return *this;
}
//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////

QVector<double> GLC_Matrix4x4::toEuler(void) const
{
	double angle_x;
	double angle_y;
	double angle_z;
	double tracex, tracey;
	angle_y= -asin(m_Matrix[8]);
	double C= cos(angle_y);

	if (!(qAbs(C - 0.0) <= glc::EPSILON)) // Gimball lock?
	{
		tracex= m_Matrix[10] / C;
		tracey= - m_Matrix[9] / C;
		angle_x= atan2( tracey, tracex);

		tracex= m_Matrix[0] / C;
		tracey= - m_Matrix[4] / C;
		angle_z= atan2( tracey, tracex);
	}
	else // Gimball lock?
	{
		angle_x= 0.0;
		tracex= m_Matrix[5] / C;
		tracey= m_Matrix[1] / C;
		angle_z= atan2( tracey, tracex);
	}
	QVector<double> result;
	result.append(fmod(angle_x, 2.0 * glc::PI));
	result.append(fmod(angle_y, 2.0 * glc::PI));
	result.append(fmod(angle_z, 2.0 * glc::PI));

	return result;
}

QString GLC_Matrix4x4::toString() const
{
	QString result;
	for (int i= 0; i < DIMMAT4X4; ++i)
	{
		result+= (QString::number(m_Matrix[0 + i])) + QString(" ");
		result+= (QString::number(m_Matrix[4 + i])) + QString(" ");
		result+= (QString::number(m_Matrix[8 + i])) + QString(" ");
		result+= (QString::number(m_Matrix[12 + i])) + QString("\n");
	}
	result.remove(result.size() - 1, 1);
	return result;
}

QQuaternion GLC_Matrix4x4::quaternion() const
{
	QQuaternion subject;
    GLC_Matrix4x4 rotMat= rotationMatrix();
	if ((this->type() != GLC_Matrix4x4::Identity) && (rotMat != GLC_Matrix4x4()))
	{
		const double matrixTrace= rotMat.trace();
		double s, w, x, y, z;

		if (matrixTrace > 0.0)
		{
			s= 0.5 / sqrt(matrixTrace);
			w= 0.25 / s;
			x= (rotMat.m_Matrix[9] - rotMat.m_Matrix[6]) * s;
			y= (rotMat.m_Matrix[2] - rotMat.m_Matrix[8]) * s;
			z= (rotMat.m_Matrix[4] - rotMat.m_Matrix[1]) * s;
		}
		else
		{
            if ((rotMat.m_Matrix[0] > rotMat.m_Matrix[5]) &&  (rotMat.m_Matrix[0] > rotMat.m_Matrix[10]))
			{	// column 0 greater
                s= 2.0 * sqrt( 1.0 + rotMat.m_Matrix[0] - rotMat.m_Matrix[5] - rotMat.m_Matrix[10]);
                w= (rotMat.m_Matrix[9] - rotMat.m_Matrix[6] ) / s;
                x= 0.25 * s;
                y= (rotMat.m_Matrix[1] + rotMat.m_Matrix[4] ) / s;
                z= (rotMat.m_Matrix[2] + rotMat.m_Matrix[8] ) / s;

			}
            else if (rotMat.m_Matrix[5] > rotMat.m_Matrix[10])
			{	// column 1 greater
                s= 2.0 * sqrt( 1.0 + rotMat.m_Matrix[5] - rotMat.m_Matrix[0] - rotMat.m_Matrix[10]);
                w= (rotMat.m_Matrix[2] - rotMat.m_Matrix[8] ) / s;
                x= (rotMat.m_Matrix[1] + rotMat.m_Matrix[4] ) / s;
                y= 0.25 * s;
                z= (rotMat.m_Matrix[6] + rotMat.m_Matrix[9] ) / s;

			}
			else
            {	// column 2 greater
                s= 2.0 * sqrt( 1.0 + rotMat.m_Matrix[10] - rotMat.m_Matrix[0] - rotMat.m_Matrix[5] );
                w= (rotMat.m_Matrix[4] - rotMat.m_Matrix[1] ) / s;
                x= (rotMat.m_Matrix[2] + rotMat.m_Matrix[8] ) / s;
                y= (rotMat.m_Matrix[6] + rotMat.m_Matrix[9] ) / s;
                z= 0.25 * s;
			}
		}
		subject= QQuaternion(w, x, y, z);
	}

	return subject;
}

QPair<GLC_Vector3d, double> GLC_Matrix4x4::rotationVectorAndAngle() const
{
    QPair<GLC_Vector3d, double> subject(GLC_Vector3d(), 0.0);
    if (GLC_Matrix4x4(*this).optimise().type() != GLC_Matrix4x4::Identity)
    {
		QQuaternion quaternion= this->quaternion();
		quaternion.normalize();

	    const double cos_angle= quaternion.scalar();
	    const double angle= acos(cos_angle);
	    double sin_angle= sqrt(1.0 - cos_angle * cos_angle);


	    if (fabs(sin_angle) < 0.0005) sin_angle= 1.0;

	    subject.first.setX(quaternion.x() / sin_angle);
	    subject.first.setY(quaternion.y() / sin_angle);
	    subject.first.setZ(quaternion.z() / sin_angle);

	    subject.second= angle * 2.0;
	}

    return subject;
}

GLC_Vector3d GLC_Matrix4x4::getXvector() const
{
    const int index= 0 * 4;
    GLC_Vector3d subject(m_Matrix[index], m_Matrix[index + 1], m_Matrix[index + 2]);

    return subject;
}

GLC_Vector3d GLC_Matrix4x4::getYvector() const
{
    const int index= 1 * 4;
    GLC_Vector3d subject(m_Matrix[index], m_Matrix[index + 1], m_Matrix[index + 2]);

    return subject;
}

GLC_Vector3d GLC_Matrix4x4::getZvector() const
{
    const int index= 2 * 4;
    GLC_Vector3d subject(m_Matrix[index], m_Matrix[index + 1], m_Matrix[index + 2]);

    return subject;
}

GLC_Vector3d GLC_Matrix4x4::getWvector() const
{
    const int index= 3 * 4;
    GLC_Vector3d subject(m_Matrix[index], m_Matrix[index + 1], m_Matrix[index + 2]);

    return subject;
}



