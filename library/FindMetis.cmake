SET(MetisLibNames "metis" "libmetis")
FIND_LIBRARY( METIS_LIB NAMES ${MetisLibNames} )
GET_FILENAME_COMPONENT(METIS_LIB_DIR ${METIS_LIB} PATH CACHE)
LINK_DIRECTORIES(${METIS_LIB_DIR})

SET(METIS_FOUND FALSE)
IF (METIS_LIB)
  SET(METIS_FOUND TRUE)
ENDIF (METIS_LIB)

IF (METIS_FOUND)
  IF (NOT Metis_FIND_QUIETLY)
     MESSAGE(STATUS "Found Metis: ${METIS_LIB_DIR}")
  ENDIF (NOT Metis_FIND_QUIETLY)
ELSE(METIS_FOUND)
  IF (Metis_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find Metis")
  ENDIF (Metis_FIND_REQUIRED)
ENDIF (METIS_FOUND)
