set(LIB_SOURCES "database.c" "bitmap.c")

add_library("imgdup" SHARED ${LIB_SOURCES})
set_property(TARGET "imgdup" PROPERTY SONAME ${VERSION})
install(TARGETS "imgdup" LIBRARY DESTINATION "lib")

add_executable("imdb-1to2" "imdb-1to2.c")
install(TARGETS "imdb-1to2" RUNTIME DESTINATION "bin")

add_executable("imdb-query" "imdb-query.c")
target_link_libraries("imdb-query" "imgdup")
install(TARGETS "imdb-query" RUNTIME DESTINATION "bin")

find_package(ImageMagick COMPONENTS MagickCore MagickWand)

add_executable("imdb-write" "imdb-write.c" "sample.c")
target_link_libraries("imdb-write" "imgdup")
target_link_libraries("imdb-write" ${ImageMagick_MagickCore_LIBRARY})
target_link_libraries("imdb-write" ${ImageMagick_MagickWand_LIBRARY})
install(TARGETS "imdb-write" RUNTIME DESTINATION "bin")
