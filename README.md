![](https://github.com/senselogic/XENIUS/blob/master/LOGO/xenius.png)

# Xenius

E57 file converter.

## Command line

```bash
nebula <options>
```

### Options

```bash
--position-offset <x> <y> <z> : set the loading position offset
--position-scaling <x> <y> <z> : set the loading position scaling
--position-rotation <x> <y> <z> : set the loading position rotation
--position-translation <x> <y> <z> : set the loading position translation
--color-offset <r> <g> <b> <i> : set the loading color offset
--color-scaling <r> <g> <b> <i> : set the loading color scaling
--color-translation <r> <g> <b> <i> : set the loading color translation
--decimation-count <decimation count> : set the loading decimation count
--read-e57-cloud <file path> : read an E57 point cloud
--write-xyz-cloud <file path> <line format> : write an XYZ point cloud
--write-pts-cloud <file path> <line format> : write a PTS point cloud
```

### Output line format

```
x : -X
y : -Y
z : -Z
X : +X
Y : +Y
Z : +Z
R : red
G : green
B : blue
I : intensity
```

## Installation

### Linux

*   run `get.sh`
*   run `make.sh`

### Windows

*   install [Visual C++ 2019](https://visualstudio.microsoft.com/fr/vs/community/)
*   install [Cmake 3.21](https://github.com/Kitware/CMake/releases/download/v3.21.0-rc3/cmake-3.21.0-rc3-windows-x86_64.msi)
*   install [Boost 1.76.0](https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.7z) in `BOOST/boost_1_76_0`
*   install [Xerces 3.2.3](https://downloads.apache.org//xerces/c/3/sources/xerces-c-3.2.3.zip) in `XERCES/xerces-c-3.2.3`
*   run `XERCES/make.bat`
*   run `make.bat`

## Version

0.2

## Author

Eric Pelzer (ecstatic.coder@gmail.com).

## License

This project is licensed under the GNU General Public License version 3.

See the [LICENSE.md](LICENSE.md) file for details.
