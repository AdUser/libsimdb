set(LIB_SOURCES "database.c" "bitmap.c" "samplers/${SIMDB_SAMPLER}.c")

add_library("simdb" SHARED ${LIB_SOURCES})
set_target_properties("simdb" PROPERTIES
  SOVERSION ${SOVERSION}
  PUBLIC_HEADER "simdb.h"
)

if (${SIMDB_SAMPLER} STREQUAL "magick")
  find_package(ImageMagick COMPONENTS MagickCore MagickWand)
  target_link_libraries("simdb" ${ImageMagick_MagickCore_LIBRARY})
  target_link_libraries("simdb" ${ImageMagick_MagickWand_LIBRARY})
endif ()

install(TARGETS "simdb" LIBRARY DESTINATION "lib" PUBLIC_HEADER DESTINATION "include")

if (WITH_TOOLS)
  add_executable("simdb-upgrade" "simdb-upgrade.c")
  install(TARGETS "simdb-upgrade" RUNTIME DESTINATION "bin")

  add_executable("simdb-tool" "simdb-tool.c")
  set_property(TARGET "simdb-tool" PROPERTY LINK_FLAGS "-Wl,--as-needed")
  target_link_libraries("simdb-tool" LINK_PUBLIC "simdb")
  install(TARGETS "simdb-tool" RUNTIME DESTINATION "bin")
endif ()
