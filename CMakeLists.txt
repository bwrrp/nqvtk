# --------------------------------------------------
# Project name
SET( ProjectName NQVTK )

# TODO: create test project instead
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
INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} )
SET( OpenGL_lib
	${OPENGL_LIBRARIES}
)

# GLEW
IF( WIN32 )
	IF( CYGWIN )
		FIND_LIBRARY( GLEW_lib 
			NAMES libGLEW GLEW glew32
			PATHS /usr/lib /usr/lib/w32api
		)
	ELSE( CYGWIN )
		# FIND_LIBRARY does not seem to work here (or maybe we need the paths...)
		SET( GLEW_lib
			glew32.lib
		)
	ENDIF( CYGWIN )
ELSE( WIN32 )
	FIND_LIBRARY( GLEW_lib
		NAMES libGLEW GLEW glew32
		PATHS /usr/lib /usr/local/lib
	)
ENDIF( WIN32 )

# We need GLBlaat
SUBDIRS( GLBlaat )
SET( GLBlaat_lib
	GLBlaat
)

# The complete set of libraries required by the project
SET( ${ProjectName}_Libraries
	${VTK_lib}
	${OpenGL_lib}
	${GLEW_lib}
	${GLBlaat_lib}
)

# --------------------------------------------------
# Modules

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
	TransferFunctionParamSet
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
)

SET( ${ProjectName}_Interactors_Classes
	Interactor
	CameraInteractor
	OrbitCameraInteractor
	ArcballCameraInteractor
	ObjectInteractor
	MainViewInteractor
	BrushingInteractor
)

SET( ${ProjectName}_GPGPU_Classes
	VolumeToVolumeFilter
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
	SET( headerfile "${CMAKE_CURRENT_SOURCE_DIR}/${module}/${class}.h" )
	IF( EXISTS ${headerfile} )
		SET( ${headers} ${${headers}} ${headerfile} )
	ELSE( EXISTS ${headerfile} )
		SET( headerfile "${CMAKE_CURRENT_SOURCE_DIR}/${class}.h" )
		IF( EXISTS ${headerfile} )
			SET( ${headers} ${${headers}} ${headerfile} )
		ELSE( EXISTS ${headerfile} )
			MESSAGE( SEND_ERROR "Class header not found for '${class}'" )
		ENDIF( EXISTS ${headerfile} )
	ENDIF( EXISTS ${headerfile} )

	# Find source files
	SET( classfile "${CMAKE_CURRENT_SOURCE_DIR}/${module}/${class}.cpp" )
	IF( EXISTS ${classfile} )
		SET( ${sources} ${${sources}} "${module}/${class}.cpp" )
	ELSE( EXISTS ${classfile} )
		SET( classfile "${CMAKE_CURRENT_SOURCE_DIR}/${module}/${class}.cxx" )
		IF( EXISTS ${classfile} )
			SET( ${sources} ${${sources}} "${module}/${class}.cxx" )
		ELSE( EXISTS ${classfile} )
			SET( classfile "${CMAKE_CURRENT_SOURCE_DIR}/${class}.cpp" )
			IF( EXISTS ${classfile} )
				SET( ${sources} ${${sources}} "${class}.cpp" )
			ELSE( EXISTS ${classfile} )
				SET( classfile "${CMAKE_CURRENT_SOURCE_DIR}/${class}.cxx" )
				IF( EXISTS ${classfile} )
					SET( ${sources} ${${sources}} "${class}.cxx" )
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
	SET( cmakeincludefile "${CMAKE_CURRENT_SOURCE_DIR}/${module}/sources.cmake" )
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
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# --------------------------------------------------
# Library target

ADD_LIBRARY( 
	${ProjectName}
	${${ProjectName}_Source}
)

TARGET_LINK_LIBRARIES(
	${ProjectName} 
	${${ProjectName}_Libraries}
)
