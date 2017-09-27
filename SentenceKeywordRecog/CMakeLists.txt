###########################################################################
## $Id:  $
## Authors: Kedar Prabhudesai
## Description: Build information for the SentenceKeywordRecog application module
##              
##              
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME SentenceKeywordRecog )

# Set the project specific sources
SET( SRC_PROJECT
  # MyNewCustomFilter.cpp    # Add new filters here to specify the module's behaviour 
  SentenceKeywordRecogTask.cpp
)

SET( HDR_PROJECT
  # MyNewCustomFilter.h      # Don't forget the corresponding headers
  SentenceKeywordRecogTask.h
  SentenceKeywordRecog.h
)

# Wrap the Qt files for MOC'ing and UIC'ing
QT4_WRAP_UI( GENERATED_UI
  SentenceKeywordRecog.ui
)

QT4_WRAP_CPP( GENERATED_MOC 
  SentenceKeywordRecog.h
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