#ifndef __SOLID_VOLUME_VIEW_H__
#define __SOLID_VOLUME_VIEW_H__

/**
 * @file SolidVolumeView.h
 *
 * Header file for the SolidVolumeView class
 *
 * @author Silvain Beriault
 */

// Header files to include
#include <vtkSmartPointer.h>
#include "BasicTypes.h"
#include "VolumeTypes.h"
#include <vtkRenderer.h>
#include <QVTKWidget.h>
#include <map>
#include <string>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <itkImageToVTKImageFilter.h>



namespace seeg {

    /**
     * Base class for volume rendering of one or many 3D datasets
     */

    class SolidVolumeView {


    public:
        enum MapperType {
            MapperTypeRayCast,
            MapperTypeTexture2D
        };

    private:


        typedef itk::ImageToVTKImageFilter<ByteVolume> ConnectorType;

        /** private struct for keeping track of vtk data structures for each dataset */
        struct SolidVolumeSceneElem {
            ByteVolume::Pointer origVolume;
            ConnectorType::Pointer connector;
            vtkSmartPointer<vtkVolume> volume;
            vtkSmartPointer<vtkColorTransferFunction> colorTransfer;
            vtkSmartPointer<vtkPiecewiseFunction> opacityTransfer;
        };

        /** map for all volumes displayed in the scene */
        typedef std::map<std::string, SolidVolumeSceneElem> SolidVolumeSceneMap;

        /** map for other actors displayed in the scene */
        typedef std::map<std::string, vtkSmartPointer<vtkActor> > ActorMap;


    public:

        /** Smart pointer for the SolidVolumeView class */
        typedef mrilSmartPtr<SolidVolumeView> Pointer;

        /**
         * Returns a new instance of a SolidVolumeView
         *
         * @return Pointer A smart pointer to the newly created SolidVolumeView instance
         */
        static Pointer New(MapperType mapperType = MapperTypeRayCast) {
            return Pointer(new SolidVolumeView(mapperType));
        }

    private:
        /**
         * Constructor
         */
        SolidVolumeView(MapperType mapperType );


    public:
        /**
         * Destructor
         */
        virtual ~SolidVolumeView();

        /**
         * Add a volume object to the scene
         *
         * @param name The name of the volume
         * @param volume The 3D dataset
         * @param volumeProperty the volume properties
         */
        void AddVolume( const std::string& name,
                        ByteVolume::Pointer volume,
                        vtkSmartPointer<vtkVolumeProperty> volumeProperty);



        /**
         * Remove a volume object from the scene
         *
         * @param name The name of the volume to remove
         */
        void RemoveVolume(const std::string& name);



        /**
         * Add an actor in the scene
         *
         * @param name The name of the actor
         * @param actor A smart pointer to the actor to display
         */
        void AddActor ( const std::string& name,
                        vtkSmartPointer<vtkActor> actor);

        /**
         * Accessor for a vtkActor instance in the scene
         *
         * @param name The name of the actor
         * @return A smart pointer to a vtkActor or NULL if not found
         */
        vtkSmartPointer<vtkActor> GetActor(const std::string& name);


        /**
         * Remove an actor from the scene
         *
         * @param name The name of the actor to remove
         */
        void RemoveActor(const std::string& name);


        /**
         * Set visibility for a volume
         *
         * @param name The name of the volume
         * @param visibility The volume visibility (1: volume visible, 0:volume invisible)
         */
        void SetVolumeVisibility(const std::string& name, int visibility);


        /**
         * Change the background color (must be called after AddVolume otherwise it crashes)
         *
         * @param rgb array containing the new background color
         */
        void SetBackgroundColor(double *rgb);


        /**
         * Update rendering
         */
        void Render();

        /**
         * Remove all volumes and actors from the scene (cleanup)
         */
        void RemoveAll();


        /**
         * Accessor for a volume being displayed
         *
         * @param name The name of the volume to access
         * @return A ByteVolume::Pointer to the volume to access and modify
         */
        ByteVolume::Pointer GetVolume(const std::string& name);


        /**
         * Accessor for the color transfer function for one volume in the scene
         *
         * @param name The name of the volume
         * @return A smart pointer to the color transfer function
         */
        vtkSmartPointer<vtkColorTransferFunction> GetColorTransferFunction(const std::string& name);

        /**
         * Accessor for the opacity transfer function fo one volume in the scene
         *
         * @param name The name of the volume
         * @return A smart pointer to the opacity transfer function
         */
        vtkSmartPointer<vtkPiecewiseFunction> GetOpacityTransferFunction(const std::string& name);


        /**
         * Accessor for the QVTKWidget (for GUI purpose)
         */
        QVTKWidget * GetWidget();

    protected:


        /** Smart pointer to the vtkRenderer */
        vtkSmartPointer<vtkRenderer> m_Renderer;

        /** Top level QVTKWidget */
        QVTKWidget m_Widget;

        /** Info about all volumes in the scene */
        SolidVolumeSceneMap m_AllVolumes;

        /** A map for all the actors in the scene */
        ActorMap m_AllActors;

        /** Specifies which mapper to use */
        MapperType m_MapperType;

    };

}


#endif
