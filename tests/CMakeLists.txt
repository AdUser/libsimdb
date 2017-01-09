add_executable("test-bitmap" "bitmap.c" "../src/bitmap.c")
add_test("test/bitmap"   "test-bitmap")

add_executable("test-record" "record.c")
add_test("test/record"   "test-record")

add_executable("test-io" "io.c" "../src/database.c" "../src/bitmap.c")
add_test("test/io" "test-io")

add_executable("test-database" "database.c" "../src/database.c" "../src/bitmap.c")
add_test("test/database" "test-database")
