/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

#include <QtDebug>
#include <QGLContext>

#include "glwidget.h"

#include <GLC_UserInput>
#include <GLC_Context>
#include <GLC_CsgHelper>
#include <GLC_LatheMesh>

// GLC_Lib Exception
#include <GLC_Exception>

GLWidget::GLWidget(QWidget *p_parent)
: QGLWidget(new QGLContext(QGLFormat(QGL::SampleBuffers)),p_parent)
, m_Cylinder(GLC_Factory::instance()->createCylinder(1.0, 2.0))	// Cylinder definition.
, m_Collection()
, m_light()
, m_GlView()
, m_MoverController()
{
	connect(&m_GlView, SIGNAL(updateOpenGL()), this, SLOT(updateGL()));
	// Set cylinder material
	QColor matBlue;
	matBlue.setRgbF(0.5, 0.8, 1.0, 1.0);
	m_Cylinder.geomAt(0)->addMaterial(new GLC_Material(matBlue));

    //m_Collection.add(m_Cylinder);

//    GLC_Mesh* pMesh1= new GLC_Cylinder(2.0, 2.0);
//    pMesh1->update();
//    GLC_Mesh* pMesh2= new GLC_Cylinder(1.0, 3.0);
//    pMesh2->update();
//    GLC_Mesh* pMesh3= GLC_CsgHelper::soustract(pMesh1, GLC_Matrix4x4(), pMesh2, GLC_Matrix4x4());

//    GLC_3DViewInstance instance(pMesh3);
//    m_Collection.add(instance);

    QList<GLC_Point3d> profile;
    GLC_Point3d p1(0.0, 0.0, -3.5);
    GLC_Point3d p2(47.5, 0.0, -3.5);
    GLC_Point3d p3(47.5, 0.0, -5.0);
    GLC_Point3d p4(48.46, 0.0, -5.0);
    GLC_Point3d p5(50.0, 0.0, -2.8);
    GLC_Point3d p6(50.0, 0.0, 2.8);
    GLC_Point3d p7(48.46, 0.0, 5.0);
    GLC_Point3d p8(47.5, 0.0, 5.0);
    GLC_Point3d p9(47.5, 0.0, 3.5);
    GLC_Point3d p10(0.0, 0.0, 3.5);


    profile << p1 << p2 << p3 << p4 << p5 << p6 << p7 << p8 << p9 << p10;

    GLC_Mesh* pMesh= new GLC_LatheMesh(profile);
    GLC_BoundingBox bBox(pMesh->boundingBox());
    qDebug().noquote() << bBox.lowerCorner().toString();
    qDebug().noquote() << bBox.upperCorner().toString();
    GLC_3DViewInstance instance(pMesh);
    m_Collection.add(instance);

	// Set up mover controller
	QColor repColor;
	repColor.setRgbF(1.0, 0.11372, 0.11372, 1.0);
	m_MoverController= GLC_Factory::instance()->createDefaultMoverController(repColor, &m_GlView);

	// Set iso view
	m_GlView.cameraHandle()->setIsoView();
}

GLWidget::~GLWidget()
{

}

void GLWidget::initializeGL()
{
	// OpenGL initialisation
	m_GlView.initGl();
	// Reframe to the cylinder bounding Box
    m_GlView.reframe(m_Collection.boundingBox());
    m_GlView.clearBackground(Qt::white);
    m_light.setTwoSided(true);
}

void GLWidget::paintGL()
{
	// Clear screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Load identity matrix
    GLC_Context::current()->glcLoadIdentity();
	try
	{
		// Set the opengl clipping plane
        m_GlView.setDistMinAndMax(m_Collection.boundingBox());

		// define the light
		m_light.glExecute();

		// define view matrix
		m_GlView.glExecuteCam();

		// Display the cylinder
        m_Collection.render(0, glc::WireRenderFlag);

        m_GlView.renderText(m_Cylinder.boundingBox().center(), "Hello Qt I'm Laumaya", Qt::black, QFont(), devicePixelRatio());

        // Display UI Info (orbit circle)
        m_MoverController.drawActiveMoverRep();
	}
	catch (GLC_Exception &e)
	{
		qDebug() << e.what();
	}
}

void GLWidget::resizeGL(int width, int height)
{
	m_GlView.setWinGLSize(width, height);	// Compute window aspect ratio

}
void GLWidget::mousePressEvent(QMouseEvent *e)
{
	if (m_MoverController.hasActiveMover()) return;
	switch (e->button())
	{
	case (Qt::RightButton):
		m_MoverController.setActiveMover(GLC_MoverController::TrackBall, GLC_UserInput(e->x(), e->y()));
		updateGL();
		break;
	case (Qt::LeftButton):
		m_MoverController.setActiveMover(GLC_MoverController::Pan, GLC_UserInput(e->x(), e->y()));
		updateGL();
		break;
	case (Qt::MidButton):
		m_MoverController.setActiveMover(GLC_MoverController::Zoom, GLC_UserInput(e->x(), e->y()));
		updateGL();
		break;

	default:
		break;
	}
}

void GLWidget::mouseMoveEvent(QMouseEvent * e)
{
	if (m_MoverController.hasActiveMover())
	{
		m_MoverController.move(GLC_UserInput(e->x(), e->y()));
		// Update opengl clipping plane
		m_GlView.setDistMinAndMax(m_Cylinder.boundingBox());
		updateGL();
	}
}

void GLWidget::mouseReleaseEvent(QMouseEvent*)
{
	if (m_MoverController.hasActiveMover())
	{
		m_MoverController.setNoMover();
		updateGL();
	}
}
