# --------------------------------------------------
# Project name
SET( ProjectName NQVTK )

# TODO: create test project instead
PROJECT( ${ProjectName} )

# --------------------------------------------------
# Dependencies

cmake_minimum_required( VERSION 2.4.0 )

# We need VTK for its datatypes
FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )
# Required VTK modules
SET( VTK_lib
    vtkFiltering
    vtkIO
)

# We need Qt
FIND_PACKAGE( Qt4 REQUIRED )
SET( QT_USE_QTMAIN TRUE )
SET( QT_USE_QTOPENGL TRUE )
INCLUDE( ${QT_USE_FILE} )
# Required Qt modules
SET( QT_lib
    ${QT_LIBRARIES}
    ${QT_QTMAIN_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
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
    ${QT_lib}
    ${OpenGL_lib}
    ${GLEW_lib}
    ${GLBlaat_lib}
)

# --------------------------------------------------
# Modules

SET( ${ProjectName}_Modules
	Math
    Rendering
    Widgets
)

# Friendly names for the modules
SET( ${ProjectName}_ModuleNames
	"NQVTK Math"
    "NQVTK Rendering"
    "Widgets"
)

# --------------------------------------------------
# Classes for each of the modules

SET( ${ProjectName}_Math_Classes
	Vector3
)

SET( ${ProjectName}_Rendering_Classes
    Renderable
    AttributePointers
    VBOMesh
    PolyData
    Camera
    Renderer
) 

# Widgets has its own sources.cmake (for moc / uic)

# --------------------------------------------------
# Extra source files (e.g. program main)
SET( ${ProjectName}_ExtraSource
    main.cpp
)

# ==================================================
# YOU DO NOT NEED TO MODIFY ANYTHING BELOW THIS LINE
# ==================================================

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

# --------------------------------------------------
# Collect source files and create source groups

SET( ${ProjectName}_Headers )
SET( ${ProjectName}_Source ${${ProjectName}_ExtraSource} )
SET( ${ProjectName}_GeneratedSource )

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

# --------------------------------------------------
# Executable target

ADD_EXECUTABLE( 
    ${ProjectName}
    ${${ProjectName}_Source}
)

TARGET_LINK_LIBRARIES(
    ${ProjectName} 
    ${${ProjectName}_Libraries}
)
