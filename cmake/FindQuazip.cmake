# A module to find quazip module. 
#   
# By default, the dynamic libraries of quazip will be found.  
#   
# If quazip is not installed in a standard path, you can use the QUAZIP_DIR  
# CMake variable to tell CMake where quazip is.   
#   
# The module defines the following variables:   
#   
# * QUAZIP_FOUND:       If false, do not try to link to quazip   
# * QUAZIP_LIBRARY:     Where to find quazip 
# * QUAZIP_INCLUDE_DIR: Where to find quazip.h   

unset(QUAZIP_INCLUDE_DIR CACHE)    
unset(QUAZIP_LIBRARY CACHE)    

if(UNIX)    
  # find the quazip include directory 
  find_path(    
    QUAZIP_INCLUDE_DIR quazip/quazip.h 
    PATHS ${QUAZIP_DIR} /usr /usr/local /opt /opt/local    
    PATH_SUFFIXES include)  

  # find the quazip library   
  find_library( 
    QUAZIP_LIBRARY 
    NAMES ${QUAZIP_STATIC} quazip quazipd 
    PATHS ${QUAZIP_DIR} /usr /usr/local /opt /opt/local    
    PATH_SUFFIXES lib64 lib) 
endif()   

if(WIN32)   
  # find the quazip include directory 
  find_path(    
    QUAZIP_INCLUDE_DIR quazip/quazip.h 
    PATHS ${QUAZIP_DIR}
    PATH_SUFFIXES include)  

  # find the quazip library   
  find_library( 
    QUAZIP_LIBRARY 
    NAMES ${QUAZIP_STATIC} quazip    
    PATHS ${QUAZIP_DIR} ${QUAZIP_DIR}/.. 
    PATH_SUFFIXES lib64 lib)    
endif() 

include(FindPackageHandleStandardArgs)  

# handle the QUIETLY and REQUIRED arguments and set QUAZIP_FOUND to TRUE if all    
# listed variables are TRUE 
find_package_handle_standard_args(  
  ${CMAKE_FIND_PACKAGE_NAME} REQUIRED_VARS QUAZIP_INCLUDE_DIR QUAZIP_LIBRARY) 

mark_as_advanced(QUAZIP_INCLUDE_DIR QUAZIP_LIBRARY)