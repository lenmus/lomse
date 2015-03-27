#-------------------------------------------------------------------------------------
# This is part of CMake configuration file for building makefiles and installfiles
# for the Lomse library
#-------------------------------------------------------------------------------------
# This moule is for defining the folders for grouping source files on IDEs
#
#-------------------------------------------------------------------------------------

# Adds folders for Visual Studio and other IDEs
source_group( "${LOMSE_GROUP_FOLDER}agg"             FILES ${AGG_FILES} ${PLATFORM_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}document"        FILES ${DOCUMENT_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}exporters"       FILES ${EXPORTERS_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}file_system"     FILES ${FILE_SYSTEM_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}graphic_model"   FILES ${GRAPHIC_MODEL_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}gui_controls"    FILES ${GUI_CONTROLS_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}internal_model"  FILES ${INTERNAL_MODEL_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}module"          FILES ${MODULE_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}mvc"             FILES ${MVC_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}parser"          FILES ${PARSER_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}parser/lmd"      FILES ${LMD_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}render"          FILES ${RENDER_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}score"           FILES ${SCORE_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}sound"           FILES ${SOUND_FILES} )
source_group( "${LOMSE_GROUP_FOLDER}packages"        FILES ${LOMSE_PACKAGES_FILES} )

