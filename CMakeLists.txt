# --------------------------------------------------
# Project name
SET( ProjectName NQVTK )
PROJECT( ${ProjectName} )

# --------------------------------------------------
# Dependencies

cmake_minimum_required( VERSION 2.4.0 )

IF( COMMAND cmake_policy )
	cmake_policy( SET CMP0003 NEW )
ENDIF( COMMAND cmake_policy )

# We need VTK for its datatypes
FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )
# Required VTK modules
SET( VTK_lib
	vtkFiltering
	vtkImaging
	vtkGraphics
)

# OpenGL and GLU
FIND_PACKAGE( OpenGL REQUIRED )

# GLEW
IF (WIN32)
	FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
		$ENV{PROGRAMFILES}/GLEW/include
		DOC "The directory where GL/glew.h resides")
	FIND_LIBRARY( GLEW_RELEASE_LIBRARY
		NAMES glew32s glew32 glew GLEW
		PATHS
		${GLEW_INCLUDE_PATH}/../lib
		$ENV{PROGRAMFILES}/GLEW/lib
		DOC "The GLEW library (glew32s.lib)")
	FIND_LIBRARY( GLEW_DEBUG_LIBRARY
		NAMES glew32sd glew32d glew32s glew32 glew GLEW
		PATHS
		${GLEW_INCLUDE_PATH}/../lib
		$ENV{PROGRAMFILES}/GLEW/lib
		DOC "The debug version of the GLEW library (glew32sd.lib)")
	SET( GLEW_LIBRARY
		optimized ${GLEW_RELEASE_LIBRARY}
		debug ${GLEW_DEBUG_LIBRARY}
	)
ELSE (WIN32)
	FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where GL/glew.h resides")
	FIND_LIBRARY( GLEW_LIBRARY
		NAMES GLEW glew
		PATHS
		${GLEW_INCLUDE_PATH}/../lib
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The GLEW library")
ENDIF (WIN32)
ADD_DEFINITIONS( -DGLEW_STATIC )

# We need GLBlaat
# TODO: cleanup GLBlaat for easier use as an external library
# For now, use 'svn checkout http://glblaat.googlecode.com/svn/trunk/ GLBlaat'
ADD_SUBDIRECTORY( GLBlaat )
SET( GLBlaat_lib
	${CMAKE_CURRENT_BINARY_DIR}/GLBlaat/GLBlaat
)

# Dependency libraries and include paths for users of the NQVTK library
SET( ${ProjectName}_Dependency_Libraries
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARY}
	${VTK_lib}
	${GLBlaat_lib}
)
SET( ${ProjectName}_Dependency_Includes
	${OPENGL_INCLUDE_DIR}
	${GLEW_INCLUDE_PATH}
	${CMAKE_CURRENT_BINARY_DIR}
)

# --------------------------------------------------
# Modules

SET( ${ProjectName}_SourceBaseDir
	"${CMAKE_CURRENT_SOURCE_DIR}/NQVTK"
)

SET( ${ProjectName}_Modules
	Math
	Rendering
	Renderables
	ParamSets
	Styles
	Widgets
	Interactors
	GPGPU
)

# Friendly names for the modules
SET( ${ProjectName}_ModuleNames
	"Math"
	"Rendering"
	"Renderables"
	"Object parameter sets"
	"Render styles"
	"Widgets"
	"Interactors"
	"GPGPU"
)

# --------------------------------------------------
# Classes for each of the modules

SET( ${ProjectName}_Math_Classes
	Vector3
	Matrix3x3
)

SET( ${ProjectName}_Rendering_Classes
	# Scene and views
	Scene
	View
	SelectionView

	# Cameras
	Camera
	OrbitCamera
	OrthoCamera
	ArcballCamera

	# Renderers
	Renderer
	SimpleRenderer
	SliceRenderer
	LayeredRenderer
	LayeredRaycastingRenderer
	PointFilteringRenderer
	BrushingRenderer
	NestedRenderer
	OverlayRenderer
	CrossEyedStereoRenderer
	ShadowMappingRenderer

	# 3D Images
	Volume
	ImageDataVolume
)

SET( ${ProjectName}_Renderables_Classes
	AttributePointers
	AttributeSet
	Renderable
	VBOMesh
	PolyData
	PointCloud
	PCAPointCorrespondenceGlyphs
)

SET( ${ProjectName}_ParamSets_Classes
	ParamSet
	VolumeParamSet
	FilteredVolumeParamSet
	PCAParamSet
	PCACorrespondenceParamSet
	DualThresholdParamSet
)

SET( ${ProjectName}_Styles_Classes
	RenderStyle
	DepthPeeling
	IBIS
	DistanceFields
	ShadowMap
	Raycaster
	DeformationRaycaster
	LayeredRaycaster
	CustomLayeredRaycaster
)

SET( ${ProjectName}_Interactors_Classes
	MouseEvent
	Interactor
	CameraInteractor
	OrbitCameraInteractor
	ArcballCameraInteractor
	ObjectInteractor
	MainViewInteractor
	BrushingInteractor
	SliceViewInteractor
)

SET( ${ProjectName}_GPGPU_Classes
	VolumeToVolumeFilter
	VolumeGaussianFilter
	VolumeJacobianDeterminant
)

# --------------------------------------------------
# Text resources to process using TextToHeader
# TODO: separate these per style to optimize compilation times

SET( ${ProjectName}_TextHeaders
	Shaders
)

SET( ${ProjectName}_Shaders_Text
	# Libraries
	LibUtility.txt
	LibCSG.txt
	
	# Render styles
	SimplePCAVS.txt
	GenericPainterVS.txt
	DepthPeelingScribeFS.txt
	DepthPeelingPainterFS.txt
	CommonIBISScribeVS.txt
	CommonIBISScribeFS.txt
	CommonIBISPainterFS.txt
	PointFilterVS.txt
	PointFilterFS.txt
	RaycasterScribeVS.txt
	RaycasterScribeFS.txt
	RaycasterPainterFS.txt
	DeformationRaycasterPainterFS.txt
	LayeredRaycasterScribePeelVS.txt
	LayeredRaycasterScribePeelFS.txt
	LayeredRaycasterScribeCastFS.txt
	LayeredRaycasterPainterFS.txt
	
	# GPGPU
	ConvolutionFilter.txt
	JacobianDeterminant.txt
)

# --------------------------------------------------
# Extra source files (e.g. program main)
SET( ${ProjectName}_ExtraSource
)

# ==================================================
# YOU DO NOT NEED TO MODIFY ANYTHING BELOW THIS LINE
# ==================================================

# --------------------------------------------------
# Utilities
ADD_SUBDIRECTORY( Utilities )
GET_TARGET_PROPERTY( TextToHeaderExe TextToHeader LOCATION )

# --------------------------------------------------
# Macros

MACRO( collect_class_source_files module class headers sources )
	# Find header file
	SET( headerfile "${${ProjectName}_SourceBaseDir}/${module}/${class}.h" )
	IF( EXISTS ${headerfile} )
		SET( ${headers} ${${headers}} ${headerfile} )
	ELSE( EXISTS ${headerfile} )
		SET( headerfile "${${ProjectName}_SourceBaseDir}/${class}.h" )
		IF( EXISTS ${headerfile} )
			SET( ${headers} ${${headers}} ${headerfile} )
		ELSE( EXISTS ${headerfile} )
			MESSAGE( SEND_ERROR "Class header not found for '${class}'" )
		ENDIF( EXISTS ${headerfile} )
	ENDIF( EXISTS ${headerfile} )

	# Find source files
	SET( classfile "${${ProjectName}_SourceBaseDir}/${module}/${class}.cpp" )
	IF( EXISTS ${classfile} )
		SET( ${sources} ${${sources}} "${classfile}" )
	ELSE( EXISTS ${classfile} )
		SET( classfile "${${ProjectName}_SourceBaseDir}/${module}/${class}.cxx" )
		IF( EXISTS ${classfile} )
			SET( ${sources} ${${sources}} "${classfile}" )
		ELSE( EXISTS ${classfile} )
			SET( classfile "${${ProjectName}_SourceBaseDir}/${class}.cpp" )
			IF( EXISTS ${classfile} )
				SET( ${sources} ${${sources}} "${classfile}" )
			ELSE( EXISTS ${classfile} )
				SET( classfile "${${ProjectName}_SourceBaseDir}/${class}.cxx" )
				IF( EXISTS ${classfile} )
					SET( ${sources} ${${sources}} "${classfile}" )
				ELSE( EXISTS ${classfile} )
					# Allow header-only implementations
					# MESSAGE( SEND_ERROR "Class implementation not found for '${class}'" )
				ENDIF( EXISTS ${classfile} )
			ENDIF( EXISTS ${classfile} )
		ENDIF( EXISTS ${classfile} )
	ENDIF( EXISTS ${classfile} )
ENDMACRO( collect_class_source_files )

MACRO( text_to_header target )
	ADD_CUSTOM_COMMAND(
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}.h" "${CMAKE_CURRENT_BINARY_DIR}/${target}.cpp"
		COMMAND ${TextToHeaderExe}
		ARGS ${target} ${ARGN}
		DEPENDS ${TextToHeaderExe} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" ${ARGN}
		COMMENT "TextToHeader - Generating ${target}"
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		VERBATIM
	)
	SET( ${ProjectName}_GeneratedSource
		${${ProjectName}_GeneratedSource}
		"${CMAKE_CURRENT_BINARY_DIR}/${target}.h"
		"${CMAKE_CURRENT_BINARY_DIR}/${target}.cpp"
	)
ENDMACRO( text_to_header )

# --------------------------------------------------
# Prepare sets

SET( ${ProjectName}_Headers )
SET( ${ProjectName}_Source ${${ProjectName}_ExtraSource} )
SET( ${ProjectName}_GeneratedSource )

# --------------------------------------------------
# Collect text files
FOREACH( textheader ${${ProjectName}_TextHeaders} )
	# Prepend source dir to input files
	SET( TextFiles )
	FOREACH( textfile ${${ProjectName}_${textheader}_Text} )
		SET( TextFiles
			${TextFiles}
			"${CMAKE_CURRENT_SOURCE_DIR}/${textheader}/${textfile}"
		)
	ENDFOREACH( textfile )
	text_to_header( ${textheader} ${TextFiles} )
ENDFOREACH( textheader )

# --------------------------------------------------
# Collect source files and create source groups

# Iterate over the modules
LIST( LENGTH ${ProjectName}_Modules nmodules )
MATH( EXPR nmodules "${nmodules} - 1" )
FOREACH( i RANGE ${nmodules} )

	# Fetch name and label of the module
	LIST( GET ${ProjectName}_Modules ${i} module )
	LIST( GET ${ProjectName}_ModuleNames ${i} modulename )

	# See if there is a separate CMake include for this module
	SET( cmakeincludefile "${${ProjectName}_SourceBaseDir}/${module}/sources.cmake" )
	IF( EXISTS ${cmakeincludefile} )
		# Include the file
		INCLUDE( ${cmakeincludefile} )
	ELSE( EXISTS ${cmakeincludefile} )
		# Iterate over classes
		SET( ${ProjectName}_${module}_Headers )
		SET( ${ProjectName}_${module}_Source )
		SET( ${ProjectName}_${module}_GeneratedSource )

		FOREACH( class ${${ProjectName}_${module}_Classes} )
			collect_class_source_files( 
				${module} 
				${class} 
				${ProjectName}_${module}_Headers 
				${ProjectName}_${module}_Source 
			)
		ENDFOREACH( class )
	ENDIF( EXISTS ${cmakeincludefile} )

	# Create source group for this module
	# Generated sources don't need to show up in this group, headers do
	SOURCE_GROUP( ${modulename} 
		FILES ${${ProjectName}_${module}_Source} ${${ProjectName}_${module}_Headers} 
	)
	# Append files to complete sets for the project
	SET( ${ProjectName}_Headers 
		${${ProjectName}_Headers} 
		${${ProjectName}_${module}_Headers} 
	)
	SET( ${ProjectName}_Source
		${${ProjectName}_Source} 
		${${ProjectName}_${module}_Source} 
		${${ProjectName}_${module}_Headers} # Add headers as well
	)
	SET( ${ProjectName}_GeneratedSource
		${${ProjectName}_GeneratedSource}
		${${ProjectName}_${module}_GeneratedSource}
	)
ENDFOREACH( i )

# Add generated sources and give them a source group
SET( ${ProjectName}_Source
	${${ProjectName}_Source}
	${${ProjectName}_GeneratedSource}
)
SOURCE_GROUP( "Generated files"
	FILES ${${ProjectName}_GeneratedSource}
)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${${ProjectName}_SourceBaseDir} )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${${ProjectName}_Dependency_Includes} )

# --------------------------------------------------
# Generate files for using NQVTK from external cmake projects

SET( ${ProjectName}_INCLUDE_DIRS_CONFIG
	${${ProjectName}_SOURCE_DIR} # NQVTK includes
	${${ProjectName}_BINARY_DIR} # Generated files
	${${ProjectName}_Dependency_Includes} # Dependencies
)
SET( ${ProjectName}_LIBRARY_DIRS_CONFIG
	${${ProjectName}_BINARY_DIR}
	${${ProjectName}_Dependency_Libraries}
)
SET( ${ProjectName}_LIBRARIES_CONFIG
	${ProjectName}
	${${ProjectName}_Dependency_Libraries}
)
SET( ${ProjectName}_USE_FILE
	"${${ProjectName}_BINARY_DIR}/UseNQVTK.cmake"
)
CONFIGURE_FILE(
	${${ProjectName}_SOURCE_DIR}/UseNQVTK.cmake.in
	${${ProjectName}_BINARY_DIR}/UseNQVTK.cmake
	COPYONLY IMMEDIATE
)
CONFIGURE_FILE(
	${${ProjectName}_SOURCE_DIR}/NQVTKConfig.cmake.in
	${${ProjectName}_BINARY_DIR}/NQVTKConfig.cmake
	@ONLY IMMEDIATE
)

# --------------------------------------------------
# Library target

ADD_LIBRARY( 
	${ProjectName}
	${${ProjectName}_Source}
)
