project( ${PluginName} )

find_package(VTK REQUIRED NO_MODULE COMPONENTS RenderingImage )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/planning
    ${CMAKE_CURRENT_SOURCE_DIR}/seegplanning
    ${CMAKE_CURRENT_SOURCE_DIR}/visualization
    ${CMAKE_CURRENT_SOURCE_DIR}/itkext
    )

# define sources
#RIZ20140815: Incorporate everything that corresponds to IBIS plugin (to get rid of static library)
set( PluginSrc seegatlasplugininterface.cpp
	       SEEGAtlasWidget.cpp
	       Point3DInputDialog.cpp
	       SEEGTrajVisWidget.cpp
               ContactsListTableWidget.cpp
	       NameInputDialog.cpp
    seegplanning/SEEGFileHelper.cpp
    seegplanning/SEEGElectrodeModel.cpp
    seegplanning/SEEGTrajectoryROIPipeline.cpp
    seegplanning/SEEGContactsROIPipeline.cpp
    seegplanning/SEEGElectrodesCohort.cpp
    seegplanning/ElectrodeInfo.cpp
    seegplanning/ContactInfo.cpp
    seegplanning/ChannelInfo.cpp
    seegplanning/BipolarChannelModel.cpp
    visualization/ProbeEyeView.cpp
	       visualization/BasicVolumeVisualizer2D.cpp
	       visualization/SolidVolumeView.cpp
	       visualization/TrajectoryView2D.cpp
	       core/GeneralTransform.cpp # RIZ: uses volume_io from minc library instead of VTK
	       core/FileUtils.cpp
	       core/MathUtils.cpp
	       core/VolumeTypes.cpp
	       core/ItkUtils.cpp
	       )

set( PluginHdr
    seegplanning/SEEGFileHelper.h
    seegplanning/SEEGElectrodeModel.h
    seegplanning/SEEGTrajectoryROIPipeline.h
    seegplanning/ElectrodeInfo.h
    seegplanning/SEEGContactsROIPipeline.h
    seegplanning/SEEGElectrodesCohort.h
    seegplanning/ElectrodeInfo.h
    seegplanning/ContactInfo.h
    seegplanning/ChannelInfo.h
    seegplanning/BipolarChannelModel.h
    visualization/ProbeEyeView.h
	       visualization/BasicVolumeVisualizer2D.h
	       visualization/SolidVolumeView.h
	       visualization/TrajectoryView2D.h
	       core/GeneralTransform.h # RIZ: uses volume_io from minc library instead of VTK
	       core/FileUtils.h
	       core/MathUtils.h
	       core/VolumeTypes.h
	       core/ItkUtils.h
)

set( PluginHdrMoc SEEGAtlasWidget.h
		  seegatlasplugininterface.h
		  Point3DInputDialog.h
		  SEEGTrajVisWidget.h
                  ContactsListTableWidget.h
		  NameInputDialog.h
		   )

set( PluginUi   SEEGAtlasWidget.ui
		Point3DInputDialog.ui
		NameInputDialog.ui
	      )


IF (NOT IBIS_PLUGINS_BUILD_ContourSurface)
   message( SEND_ERROR "Cannot find the ContourSurface plugin." ${PluginName} " Plugin depends on ContourSurface.\n" )
ENDIF()

# Create plugin
DefinePlugin( "${PluginSrc}" "${PluginHdr}" "${PluginHdrMoc}" "${PluginUi}" )
target_link_libraries( ${PluginName} ${VTK_LIBRARIES} ContourSurface )
