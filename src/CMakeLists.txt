set(LIB_SOURCES "database.c" "bitmap.c" "samplers/${SAMPLER}.c")

add_library("simdb" SHARED ${LIB_SOURCES})
set_property(TARGET "simdb" PROPERTY SONAME ${VERSION})

if (${SAMPLER} STREQUAL "magick")
  find_package(ImageMagick COMPONENTS MagickCore MagickWand)
  target_link_libraries("simdb" ${ImageMagick_MagickCore_LIBRARY})
  target_link_libraries("simdb" ${ImageMagick_MagickWand_LIBRARY})
endif ()

install(TARGETS "simdb" LIBRARY DESTINATION "lib")

if (WITH_TOOLS)
  add_executable("simdb-1to2" "simdb-1to2.c")
  install(TARGETS "simdb-1to2" RUNTIME DESTINATION "bin")

  add_executable("simdb-tool" "simdb-tool.c")
  target_link_libraries("simdb-tool" "simdb")
  install(TARGETS "simdb-tool" RUNTIME DESTINATION "bin")
endif ()
