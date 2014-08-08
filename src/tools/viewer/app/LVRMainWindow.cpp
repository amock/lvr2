/* Copyright (C) 2011 Uni Osnabrück
 * This file is part of the LAS VEGAS Reconstruction Toolkit,
 *
 * LAS VEGAS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LAS VEGAS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/**
 * MainWindow.cpp
 *
 *  @date Jan 31, 2014
 *  @author Thomas Wiemann
 */

#include <QFileInfo>
#include <QAbstractItemView>
#include <QtGui>

#include "LVRMainWindow.hpp"
#include "../vtkBridge/LVRModelBridge.hpp"
#include "../widgets/LVRModelItem.hpp"
#include "../widgets/LVRItemTypes.hpp"
#include "../widgets/LVRTransformationDialog.hpp"
#include "../widgets/LVRPointCloudItem.hpp"
#include "../widgets/LVRMeshItem.hpp"

#include "io/ModelFactory.hpp"
#include "io/DataStruct.hpp"

#include "registration/ICPPointAlign.hpp"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPointPicker.h>
#include <vtkCamera.h>

namespace lvr
{

LVRMainWindow::LVRMainWindow()
{
    setupUi(this);
    setupQVTK();

    // Init members
    m_correspondanceDialog = new LVRCorrespondanceDialog(treeWidget);

    // Setup specific properties
    QHeaderView* v = this->treeWidget->header();
    v->resizeSection(0, 175);

    treeWidget->setSelectionMode( QAbstractItemView::SingleSelection);
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_treeWidgetHelper = new LVRTreeWidgetHelper(treeWidget);

    m_treeContextMenu = new QMenu;
    m_actionShowColorDialog = new QAction("Select base color...", this);
    m_actionDeleteModelItem = new QAction("Delete model", this);
    m_actionExportModelTransformed = new QAction("Export model with transformation", this);

    m_treeContextMenu->addAction(m_actionShowColorDialog);
    m_treeContextMenu->addAction(m_actionDeleteModelItem);
    m_treeContextMenu->addAction(m_actionExportModelTransformed);

    // Toolbar item "File"
    m_actionOpen = this->actionOpen;
    m_actionExport = this->actionExport;
    m_actionQuit = this->actionQuit;
    // Toolbar item "Views"
    m_actionReset_Camera = this->actionReset_Camera;
    m_actionStore_Current_View = this->actionStore_Current_View;
    m_actionRecall_Stored_View = this->actionRecall_Stored_View;
    // QToolbar below toolbar
    m_actionShow_Points = this->actionShow_Points;
    m_actionShow_Mesh = this->actionShow_Mesh;
    // Slider below tree widget
    m_horizontalSliderPointSize = this->horizontalSliderPointSize;
    m_horizontalSliderTransparency = this->horizontalSliderTransparency;
    // Combo boxes
    m_comboBoxGradient = this->comboBoxGradient;
    m_comboBoxShading = this->comboBoxShading;
    // Buttons below combo boxes
    m_buttonRecordPath = this->buttonRecordPath;
    m_buttonCreateMesh = this->buttonCreateMesh;
    m_buttonExportData = this->buttonExportData;
    m_buttonTransformModel = this->buttonTransformModel;

    m_pickingInteractor = new LVRPickingInteractor(m_renderer);
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle( m_pickingInteractor );
    vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
    qvtkWidget->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
    connectSignalsAndSlots();
}

LVRMainWindow::~LVRMainWindow()
{
    if(m_correspondanceDialog)
    {
        delete m_correspondanceDialog;
    }
}

void LVRMainWindow::connectSignalsAndSlots()
{
    QObject::connect(m_actionOpen, SIGNAL(activated()), this, SLOT(loadModel()));
    QObject::connect(m_buttonTransformModel, SIGNAL(pressed()), this, SLOT(showTransformationDialog()));
    QObject::connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeContextMenu(const QPoint&)));
    QObject::connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(restoreSliders(QTreeWidgetItem*, int)));
    QObject::connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(setModelVisibility(QTreeWidgetItem*, int)));

    QObject::connect(m_actionQuit, SIGNAL(activated()), qApp, SLOT(quit()));

    QObject::connect(m_actionShowColorDialog, SIGNAL(activated()), this, SLOT(showColorDialog()));
    QObject::connect(m_actionDeleteModelItem, SIGNAL(activated()), this, SLOT(deleteModelItem()));
    QObject::connect(m_actionExportModelTransformed, SIGNAL(activated()), this, SLOT(exportSelectedModel()));

    QObject::connect(m_actionReset_Camera, SIGNAL(activated()), this, SLOT(updateView()));
    QObject::connect(m_actionStore_Current_View, SIGNAL(activated()), this, SLOT(saveCamera()));
    QObject::connect(m_actionRecall_Stored_View, SIGNAL(activated()), this, SLOT(loadCamera()));

    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(accepted()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(accepted()), this, SLOT(alignPointClouds()));
    QObject::connect(m_correspondanceDialog->m_dialog, SIGNAL(rejected()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog, SIGNAL(addArrow(LVRVtkArrow*)), this, SLOT(addArrow(LVRVtkArrow*)));
    QObject::connect(m_correspondanceDialog, SIGNAL(removeArrow(LVRVtkArrow*)), this, SLOT(removeArrow(LVRVtkArrow*)));
    QObject::connect(m_correspondanceDialog, SIGNAL(disableCorrespondenceSearch()), m_pickingInteractor, SLOT(correspondenceSearchOff()));
    QObject::connect(m_correspondanceDialog, SIGNAL(enableCorrespondenceSearch()), m_pickingInteractor, SLOT(correspondenceSearchOn()));

    QObject::connect(m_actionShow_Points, SIGNAL(toggled(bool)), this, SLOT(togglePoints(bool)));
    QObject::connect(m_actionShow_Mesh, SIGNAL(toggled(bool)), this, SLOT(toggleMeshes(bool)));

    QObject::connect(m_horizontalSliderPointSize, SIGNAL(valueChanged(int)), this, SLOT(changePointSize(int)));
    QObject::connect(m_horizontalSliderTransparency, SIGNAL(valueChanged(int)), this, SLOT(changeTransparency(int)));

    QObject::connect(m_comboBoxShading, SIGNAL(currentIndexChanged(int)), this, SLOT(changeShading(int)));

    QObject::connect(m_pickingInteractor, SIGNAL(firstPointPicked(double*)),m_correspondanceDialog, SLOT(firstPointPicked(double*)));
    QObject::connect(m_pickingInteractor, SIGNAL(secondPointPicked(double*)),m_correspondanceDialog, SLOT(secondPointPicked(double*)));

    QObject::connect(this, SIGNAL(correspondenceDialogOpened()), m_pickingInteractor, SLOT(correspondenceSearchOn()));

    QObject::connect(this->actionICP_using_manual_correspondance, SIGNAL(activated()), this, SLOT(manualICP()));
}

void LVRMainWindow::setupQVTK()
{
    // Add new renderer to the render window of the QVTKWidget
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_camera = vtkSmartPointer<vtkCamera>::New();
    this->qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);
}

void LVRMainWindow::updateView()
{
    m_renderer->ResetCamera();
    m_renderer->ResetCameraClippingRange();
	this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::refreshView()
{
	this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::saveCamera()
{
	m_camera->DeepCopy(m_renderer->GetActiveCamera());
}

void LVRMainWindow::loadCamera()
{
	m_renderer->GetActiveCamera()->DeepCopy(m_camera);
	refreshView();
}

void LVRMainWindow::addArrow(LVRVtkArrow* a)
{
    if(a)
    {
        m_renderer->AddActor(a->getArrowActor());
        m_renderer->AddActor(a->getStartActor());
        m_renderer->AddActor(a->getEndActor());
    }
    this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::removeArrow(LVRVtkArrow* a)
{
    if(a)
    {
        m_renderer->RemoveActor(a->getArrowActor());
        m_renderer->RemoveActor(a->getStartActor());
        m_renderer->RemoveActor(a->getEndActor());
    }
    this->qvtkWidget->GetRenderWindow()->Render();
}

void LVRMainWindow::restoreSliders(QTreeWidgetItem* treeWidgetItem, int column)
{

    if(treeWidgetItem->type() == LVRModelItemType)
    {
        /*
        QTreeWidgetItemIterator it(treeWidgetItem);

        while(*it)
        {
            QTreeWidgetItem* child_item = *it;
            cout << "CHILD TYPE: " << child_item->type() << endl;
            if(child_item->type() == LVRPointCloudItemType)
            {
                LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(child_item);
                m_horizontalSliderPointSize->setEnabled(true);
                m_horizontalSliderPointSize->setValue(model_item->getPointSize());
                int transparency = ((float)1 - model_item->getOpacity()) * 100;
                m_horizontalSliderTransparency->setEnabled(true);
                m_horizontalSliderTransparency->setValue(transparency);
            }
            else if(child_item->type() == LVRMeshItemType)
            {
                LVRMeshItem* model_item = static_cast<LVRMeshItem*>(child_item);
                m_horizontalSliderPointSize->setEnabled(false);
                m_horizontalSliderPointSize->setValue(1);
                int transparency = ((float)1 - model_item->getOpacity()) * 100;
                m_horizontalSliderTransparency->setEnabled(true);
                m_horizontalSliderTransparency->setValue(transparency);
            }
            ++it;
        }
        */
    }
    else if(treeWidgetItem->type() == LVRPointCloudItemType)
    {
        LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(treeWidgetItem);
        m_horizontalSliderPointSize->setEnabled(true);
        m_horizontalSliderPointSize->setValue(model_item->getPointSize());
        int transparency = ((float)1 - model_item->getOpacity()) * 100;
        m_horizontalSliderTransparency->setEnabled(true);
        m_horizontalSliderTransparency->setValue(transparency);
    }
    else if(treeWidgetItem->type() == LVRMeshItemType)
    {
        LVRMeshItem* model_item = static_cast<LVRMeshItem*>(treeWidgetItem);
        m_horizontalSliderPointSize->setEnabled(false);
        m_horizontalSliderPointSize->setValue(1);
        int transparency = ((float)1 - model_item->getOpacity()) * 100;
        m_horizontalSliderTransparency->setEnabled(true);
        m_horizontalSliderTransparency->setValue(transparency);
    }
    else
    {
        m_horizontalSliderPointSize->setEnabled(false);
        m_horizontalSliderPointSize->setValue(1);
        m_horizontalSliderTransparency->setEnabled(false);
        m_horizontalSliderTransparency->setValue(0);
    }
}

void LVRMainWindow::exportSelectedModel()
{
    // Get selected point cloud
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        if(item->type() == LVRPointCloudItemType)
        {
            if(item->parent() && item->parent()->type() == LVRModelItemType)
            {
                QString qFileName = QFileDialog::getSaveFileName(this, tr("Export Point Cloud As..."), "", tr("Point cloud Files(*.ply *.3d)"));

                LVRModelItem* model_item = static_cast<LVRModelItem*>(item->parent());
                LVRPointCloudItem* pc_item = static_cast<LVRPointCloudItem*>(item);
                PointBufferPtr points = pc_item->getPointBuffer();

                // Get transformation matrix
                Pose p = model_item->getPose();
                Matrix4f mat(Vertexf(p.x, p.y, p.z), Vertexf(p.r, p.t, p.p));

                // Allocate target buffer and insert transformed points
                size_t n;
                floatArr transformedPoints(new float[3 * points->getNumPoints()]);
                floatArr pointArray = points->getPointArray(n);
                for(size_t i = 0; i < points->getNumPoints(); i++)
                {
                    Vertexf v(pointArray[3 * i], pointArray[3 * i + 1], pointArray[3 * i + 2]);
                    Vertexf vt = mat * v;

                    transformedPoints[3 * i    ] = vt[0];
                    transformedPoints[3 * i + 1] = vt[1];
                    transformedPoints[3 * i + 2] = vt[2];
                }

                // Save transformed points
                PointBufferPtr trans(new PointBuffer);
                trans->setPointArray(transformedPoints, n);
                ModelPtr model(new Model(trans));
                ModelFactory::saveModel(model, qFileName.toStdString());

            }
        }
    }
}

void LVRMainWindow::alignPointClouds()
{
    Matrix4f mat = m_correspondanceDialog->getTransformation();
    QString name = m_correspondanceDialog->getDataName();
    QString modelName = m_correspondanceDialog->getModelName();

    PointBufferPtr modelBuffer = m_treeWidgetHelper->getPointBuffer(modelName);
    PointBufferPtr dataBuffer  = m_treeWidgetHelper->getPointBuffer(name);

    float pose[6];
    LVRModelItem* item = m_treeWidgetHelper->getModelItem(name);

    if(item)
    {
        mat.toPostionAngle(pose);

        // Pose ist in radians, so we need to convert p to degrees
        // to achieve consistency
        Pose p;
        p.x = pose[0];
        p.y = pose[1];
        p.z = pose[2];
        p.r = pose[3]  * 57.295779513;
        p.t = pose[4]  * 57.295779513;
        p.p = pose[5]  * 57.295779513;
        item->setPose(p);
    }

    updateView();
    // Refine pose via ICP
    if(m_correspondanceDialog->doICP() && modelBuffer && dataBuffer)
    {
        ICPPointAlign icp(modelBuffer, dataBuffer, mat);
        icp.setEpsilon(m_correspondanceDialog->getEpsilon());
        icp.setMaxIterations(m_correspondanceDialog->getMaxIterations());
        icp.setMaxMatchDistance(m_correspondanceDialog->getMaxDistance());
        Matrix4f refinedTransform = icp.match();

        cout << "Initial: " << mat << endl;

        // Apply correction to initial estimation
        //refinedTransform = mat * refinedTransform;
        refinedTransform.toPostionAngle(pose);

        cout << "Refined: " << refinedTransform << endl;

        Pose p;
        p.x = pose[0];
        p.y = pose[1];
        p.z = pose[2];
        p.r = pose[3]  * 57.295779513;
        p.t = pose[4]  * 57.295779513;
        p.p = pose[5]  * 57.295779513;
        item->setPose(p);
    }
    m_correspondanceDialog->clearAllItems();
    updateView();

}

void LVRMainWindow::showTreeContextMenu(const QPoint& p)
{
	// Only display context menu for point clounds and meshes
	QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
	if(items.size() > 0)
	{
		QTreeWidgetItem* item = items.first();
		if(item->type() == LVRPointCloudItemType || item->type() == LVRMeshItemType)
		{
			QPoint globalPos = treeWidget->mapToGlobal(p);
			m_treeContextMenu->exec(globalPos);
		}
	}
}

void LVRMainWindow::loadModel()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Open Model"), "", tr("Model Files (*.ply *.obj *.pts *.3d *.txt)"));

    if(filenames.size() > 0)
    {
        QStringList::Iterator it = filenames.begin();
        while(it != filenames.end())
        {
            // Load model and generate vtk representation
            ModelPtr model = ModelFactory::readModel((*it).toStdString());
            ModelBridgePtr bridge( new LVRModelBridge(model));
            bridge->addActors(m_renderer);

            // Add item for this model to tree widget
            QFileInfo info((*it));
            QString base = info.fileName();
            LVRModelItem* item = new LVRModelItem(bridge, base);
            this->treeWidget->addTopLevelItem(item);
            item->setExpanded(true);
            ++it;
        }

        updateView();
    }
}

void LVRMainWindow::setModelVisibility(QTreeWidgetItem* treeWidgetItem, int column)
{
    if(treeWidgetItem->type() == LVRModelItemType)
    {
        LVRModelItem* item = static_cast<LVRModelItem*>(treeWidgetItem);
        item->checkboxWrapper(column);
        refreshView();
    }
}

void LVRMainWindow::deleteModelItem()
{
	QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
	if(items.size() > 0)
	{
		QTreeWidgetItem* item = items.first();

		// Remove model from view
		if(item->type() == LVRPointCloudItemType)
		{
			LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(item);
			m_renderer->RemoveActor(model_item->getActor());
		}
		else if(item->type() == LVRMeshItemType)
		{
			LVRMeshItem* model_item = static_cast<LVRMeshItem*>(item);
			m_renderer->RemoveActor(model_item->getActor());
		}

		// Remove list item (safe according to http://stackoverflow.com/a/9399167)
		delete item;

		refreshView();
	}
}

void LVRMainWindow::changePointSize(int pointSize)
{
	QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
	if(items.size() > 0)
	{
		QTreeWidgetItem* item = items.first();

        if(item->type() == LVRModelItemType)
        {
            QTreeWidgetItemIterator it(item);

            while(*it)
            {
                QTreeWidgetItem* child_item = *it;
                if(child_item->type() == LVRPointCloudItemType)
                {
                    LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(child_item);
                    model_item->setPointSize(pointSize);
                }
                ++it;
            }
        }
        else if(item->type() == LVRPointCloudItemType)
        {
            LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(item);
            model_item->setPointSize(pointSize);
        }

		refreshView();
	}
}

void LVRMainWindow::changeTransparency(int transparencyValue)
{
	QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
	if(items.size() > 0)
	{
		QTreeWidgetItem* item = items.first();
		float opacityValue = 1 - ((float)transparencyValue / (float)100);

		if(item->type() == LVRModelItemType)
		{
		    QTreeWidgetItemIterator it(item);

            while(*it)
            {
                QTreeWidgetItem* child_item = *it;
                if(child_item->type() == LVRPointCloudItemType)
                {
                    LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(child_item);
                    model_item->setOpacity(opacityValue);
                }
                else if(child_item->type() == LVRMeshItemType)
                {
                    LVRMeshItem* model_item = static_cast<LVRMeshItem*>(child_item);
                    model_item->setOpacity(opacityValue);
                }
                ++it;
            }
		}
		else if(item->type() == LVRPointCloudItemType)
		{
		    LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(item);
		    model_item->setOpacity(opacityValue);
		}
		else if(item->type() == LVRMeshItemType)
		{
		    LVRMeshItem* model_item = static_cast<LVRMeshItem*>(item);
		    model_item->setOpacity(opacityValue);
		}

		refreshView();
	}
}

void LVRMainWindow::changeShading(int shader)
{
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();

        if(item->type() == LVRMeshItemType)
        {
            LVRMeshItem* model_item = static_cast<LVRMeshItem*>(item);
            model_item->setShading(shader);
            refreshView();
        }
    }
}

void LVRMainWindow::togglePoints(bool checkboxState)
{
	QTreeWidgetItemIterator it(treeWidget);

	while(*it)
	{
		QTreeWidgetItem* item = *it;
		if(item->type() == LVRPointCloudItemType)
		{
			LVRPointCloudItem* model_item = static_cast<LVRPointCloudItem*>(item);
			model_item->setVisibility(checkboxState);
		}
		++it;
	}

	refreshView();
}

void LVRMainWindow::toggleMeshes(bool checkboxState)
{
	QTreeWidgetItemIterator it(treeWidget);

	while(*it)
	{
		QTreeWidgetItem* item = *it;
		if(item->type() == LVRMeshItemType)
		{
			LVRMeshItem* model_item = static_cast<LVRMeshItem*>(item);
			model_item->setVisibility(checkboxState);
		}
		++it;
	}

	refreshView();
}

void LVRMainWindow::manualICP()
{
    m_correspondanceDialog->fillComboBoxes();
    m_correspondanceDialog->m_dialog->show();
    m_correspondanceDialog->m_dialog->raise();
    m_correspondanceDialog->m_dialog->activateWindow();
    Q_EMIT(correspondenceDialogOpened());
}

void LVRMainWindow::showColorDialog()
{
	QColor c = QColorDialog::getColor();
	QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
	if(items.size() > 0)
	{
		QTreeWidgetItem* item = items.first();
		if(item->type() == LVRPointCloudItemType)
		{
			LVRPointCloudItem* pc_item = static_cast<LVRPointCloudItem*>(item);
			pc_item->setColor(c);
		}
		else if(item->type() == LVRMeshItemType)
		{
		    LVRMeshItem* mesh_item = static_cast<LVRMeshItem*>(item);
		    mesh_item->setColor(c);
		}
		else {
			return;
		}

		refreshView();
	}
}

void LVRMainWindow::showTransformationDialog()
{
    // Setup a message box for unsupported items
    QMessageBox box;
    box.setText("Unsupported Item for Transformation.");
    box.setInformativeText("Only whole models, point clouds or meshes can be transformed.");
    box.setStandardButtons(QMessageBox::Ok);

    // Get selected item from tree and check type
    QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
    if(items.size() > 0)
    {
        QTreeWidgetItem* item = items.first();
        if(item->type() == LVRModelItemType)
        {
            LVRModelItem* item = static_cast<LVRModelItem*>(items.first());
            LVRTransformationDialog* dialog = new LVRTransformationDialog(item, qvtkWidget->GetRenderWindow());
        }
        else if(item->type() == LVRPointCloudItemType || item->type() == LVRMeshItemType)
        {
            if(item->parent()->type() == LVRModelItemType)
            {
                LVRModelItem* l_item = static_cast<LVRModelItem*>(item->parent());
                LVRTransformationDialog* dialog = new LVRTransformationDialog(l_item, qvtkWidget->GetRenderWindow());
            }
            else
            {
                box.exec();
            }
        }
        else
        {
            box.exec();
        }
    }
}


} /* namespace lvr */
