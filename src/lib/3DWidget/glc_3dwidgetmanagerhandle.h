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
//! \file glc_3dwidgetmanagerhandle.h Interface for the GLC_3DWidgetManagerHandle class.

#ifndef GLC_3DWIDGETMANAGERHANDLE_H_
#define GLC_3DWIDGETMANAGERHANDLE_H_

#include "../glc_config.h"
#include "../sceneGraph/glc_3dviewcollection.h"
#include "../viewport/glc_viewport.h"

class GLC_3DViewInstance;
class GLC_3DWidget;

class GLC_LIB_EXPORT GLC_3DWidgetManagerHandle
{

//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Construct a 3D widget manager attached to the given viewport
	GLC_3DWidgetManagerHandle(GLC_Viewport* pViewport);

	//! Destructor
	~GLC_3DWidgetManagerHandle();
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Return true if there is only one widget manager associated to this handle
	inline bool isOrphan() const
	{return  0 == m_Count;}

	//! Return the 3DView instance of the given id
	inline GLC_3DViewInstance* instanceHandle(GLC_uint id)
	{return m_Collection.instanceHandle(id);}

	//! Return true if this 3DWidget manager has active widget
	inline bool hasAnActiveWidget() const
	{return 0 != m_Active3DWidgetId;}

    GLC_uint active3DWidgetId() const
    {return m_Active3DWidgetId;}

    //! Return true if this 3DWidget manager has visible widget
    inline bool hasVisibleWidget() const
    {return m_Collection.hasVisibleInstance();}

	//! Return an handle to the camera of the viewport of this manager
	inline const GLC_Camera* cameraHandle() const
	{return m_pViewport->cameraHandle();}

	//! Return the boundingBox of 3dwidget
	inline GLC_BoundingBox boundingBox()
	{return m_Collection.boundingBox();}

	//! Return true if the viewport use orthographic projection
	inline bool useOrtho() const
	{return m_pViewport->useOrtho();}

	//! Return the tangent value of the viewport
	double viewportTangent() const
	{return m_pViewport->viewTangent();}

	//! Return the viewport of this manager
	inline GLC_Viewport* viewport() const
	{return m_pViewport;}

	//! Return true if this manager contains the given widget id
	inline bool contains3DWidget(GLC_uint id) const
	{return m_3DWidgetHash.contains(id);}

	//! Return the 3DWidget of the given widget id
	inline GLC_3DWidget* widget(GLC_uint id) const
	{return m_3DWidgetHash.value(id);}

	//! Return true if this 3DWidget manager is empty
	inline bool isEmpty() const
	{return m_3DWidgetHash.isEmpty();}

    bool useOrderRendering() const
    {return m_Collection.useOrderRendering();}

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Increment the number of world
	inline void increment()
	{++m_Count;}

	//! Decrement the number of world
	inline void decrement()
	{--m_Count;}

	//! Add the given 3D widget into this manager
	void add3DWidget(GLC_3DWidget* p3DWidget);

	//! Remove the 3D widget with the given id from this manager
	/*! Associated 3D view instance are removed*/
	void remove3DWidget(GLC_uint id);

	//! Take the 3D widget with the given id from this manager
	/*! Associated 3D view instance are NOT removed*/
	GLC_3DWidget* take(GLC_uint id);

	//! Add the given 3D view instance link to the given 3D widget into this manager
	void add3DViewInstance(const GLC_3DViewInstance& instance, GLC_uint widgetId);

	//! Remove the 3D view instance with the given id from this manager collection
	void remove3DViewInstance(GLC_uint id);

	//! Remove all 3D view instance from this manager
	void clear();

    //! Remove all 3D view instance of the given type from this manager
    void clear(int type);

	//! Set the visibility of the given 3D widget id
	void setWidgetVisible(GLC_uint id, bool visible);

    //! Update all 3dwidget
    void update();

    void setOrderRenderingUsage(bool use)
    {m_Collection.setOrderRenderingUsage(use);}

//@}
//////////////////////////////////////////////////////////////////////
/*! \name Interaction Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Recieve Mouse move event with the given instance id Return true if the event is catch
    glc::WidgetEventFlag moveEvent(GLC_uint selectedId, const GLC_Point3d &pos, QInputEvent* pInputEvent);

	//! Recieve Mouse press event with the given instance id Return true if the event is catch
    glc::WidgetEventFlag pressEvent(GLC_uint selectedId, const GLC_Point3d &pos, QInputEvent* pInputEvent);

	//! Recieve Mouse release event with the given instance id Return true if the event is catch
    glc::WidgetEventFlag releaseEvent(QInputEvent* pInputEvent);

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Render the 3DWidget of this manager
	void render();

//@}

//////////////////////////////////////////////////////////////////////
// Private services function
//////////////////////////////////////////////////////////////////////
private:

//////////////////////////////////////////////////////////////////////
// Private Member
//////////////////////////////////////////////////////////////////////
private:
	//! The Collection
	GLC_3DViewCollection m_Collection;

	//! Widget manager count
	int m_Count;

	//! The 3D widget hash table
	QHash<GLC_uint, GLC_3DWidget*> m_3DWidgetHash;

	//! The papping between 3D view instance and 3DWidget
	QHash<GLC_uint, GLC_uint> m_MapBetweenInstanceWidget;

	//! The viewport of this 3d widget manager handle
	GLC_Viewport* m_pViewport;

	//! The active 3Dwidget id
	GLC_uint m_Active3DWidgetId;

	//! The preselected 3DWidget
	GLC_uint m_Preselected3DWidgetId;
};

#endif /* GLC_3DWIDGETMANAGERHANDLE_H_ */
