set(LIB_SOURCES "database.c" "bitmap.c")

add_library("simdb" SHARED ${LIB_SOURCES})
set_property(TARGET "simdb" PROPERTY SONAME ${VERSION})
install(TARGETS "simdb" LIBRARY DESTINATION "lib")

if (WITH_TOOLS)
  add_executable("simdb-1to2" "simdb-1to2.c")
  install(TARGETS "simdb-1to2" RUNTIME DESTINATION "bin")

  add_executable("simdb-query" "simdb-query.c")
  target_link_libraries("simdb-query" "simdb")
  install(TARGETS "simdb-query" RUNTIME DESTINATION "bin")

  find_package(ImageMagick COMPONENTS MagickCore MagickWand)

  add_executable("simdb-write" "simdb-write.c" "sample.c")
  target_link_libraries("simdb-write" "simdb")
  target_link_libraries("simdb-write" ${ImageMagick_MagickCore_LIBRARY})
  target_link_libraries("simdb-write" ${ImageMagick_MagickWand_LIBRARY})
  install(TARGETS "simdb-write" RUNTIME DESTINATION "bin")
endif ()
