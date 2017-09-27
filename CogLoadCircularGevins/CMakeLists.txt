###########################################################################
## $Id:  $
## Authors: Kedar Prabhudesai for SSPACISS at Duke University
## Description: Build information for the CogLoadTaskCircular application module
##              
##              
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME CogLoadCircularGevins )

# Set the project specific sources
SET( SRC_PROJECT
  # MyNewCustomFilter.cpp    # Add new filters here to specify the module's behaviour 
  CogLoadCircularGevinsTask.cpp
)

SET( HDR_PROJECT
  # MyNewCustomFilter.h      # Don't forget the corresponding headers
  CogLoadCircularGevinsTask.h
  nbacktestCircGevins.h
)

# Wrap the Qt files for MOC'ing and UIC'ing
QT4_WRAP_UI( GENERATED_UI
  nbacktestCircGevins.ui
)

QT4_WRAP_CPP( GENERATED_MOC 
  nbacktestCircGevins.h
)

SET( GENERATED
  ${GENERATED_UI}
  ${GENERATED_MOC}
)

# Set the Project Source Groups
SOURCE_GROUP( Source\\Project FILES ${SRC_PROJECT} )
SOURCE_GROUP( Headers\\Project FILES ${HDR_PROJECT} )

# Set Generated Source Groups
SOURCE_GROUP( Generated FILES ${GENERATED} )

# We're done. Add the application module to the Makefile or compiler project file:
BCI2000_ADD_APPLICATION_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT};${GENERATED}" "${HDR_PROJECT}" 
)


