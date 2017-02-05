Build notes
-----------

General build flow as simple as this three commands:

    cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release .
    make
    sudo checkinstall -- make install

Now, cmake options explained:

* `CMAKE_INSTALL_PREFIX` -- is root of place where to install the library
* `CMAKE_BUILD_TYPE` -- tunes build options for warious presets (Release, Debug, ...)

Project-specific options explained:

* `SIMDB_SAMPLER` -- selects a library for use for making image samples. Now available:
  * magick -- for this time is only production-ready sampler (both ImageMagick and GraphicsMagick supported)
  * random -- backend for testing, generates sample with random data
  * dummy -- empty backend, always fails (use only if you don't need to add new image samples)
* `WITH_TOOLS` -- build some usefull tools
  * simdb-tool -- manual manipulation of samples database
  * simdb-upgrade -- upgrades database format to latest known version
* `WITH_HARDENING` -- enable some additional compiler sanity checks

`checkinstall` on last step is optional, but recommended tool, unless you don't care garbage in your system.
