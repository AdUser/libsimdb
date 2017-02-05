Library usage
-------------

Database know nothing about location of the source images,
records addressed only by some numeric id.

Example usage:

    #include <stdbool.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    
    #include <simdb.h>
    
    int main() {
      simdb_t *sdb = NULL;
      simdb_search_t search;
      simdb_match_t *match = NULL;
      const char *path = "/tmp/test.sdb";
      const char *files[] = {
        "/path/to/file/a.jpg",
        "/path/to/file/b.png",
        "/path/to/file/c.bmp",
      };
      int ret = 0;
    
      if (!simdb_create(path)) {
        perror("can't create test database");
        return 1;
      }
    
      sdb = simdb_open(path, SIMDB_FLAG_WRITE | SIMDB_FLAG_LOCKNB, &ret);
      if (!sdb) {
        fprintf(stderr, "can't open test simdb: %s\n", simdb_error(ret));
        return 1;
      }
    
      for (int i = 0; i < sizeof(files); i++) {
        ret = simdb_record_add(sdb, i, files[i], 0);
        if (ret < 0) {
          fprintf(stderr, "can't add file from %s: %s\n", files[i], simdb_error(ret));
        }
      }
    
      simdb_search_init(&search);
      /* tune search parameters */
      search.d_ratio  = 0.1;  /* max difference in ratio -- 10% */
      search.d_bitmap = 0.08; /* max difference in ratio --  8% */
      /* compare given file against database */
      const char *sample = "/path/to/file/d.png";
      simdb_search_file(sdb, &search, sample);
      /* show search results if any */
      if (search.found) {
        printf("file %s similar to:\n", sample);
        for (int i = 0; i < search.found; i++) {
          match = &search.matches[i];
          printf("- %s (%d %%)\n", files[match->num], (int) match->d_bitmap * 100);
        }
        /* free search results */
        simdb_search_free(&search);
      }
    
      simdb_close(sdb);
      unlink(path);
    
      return 0;
    }

You may build this example with next command:

gcc -Wall -std=c99 -O0 -pedantic -lsimdb -o simdb-usage-test test.c
