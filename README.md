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
--write-xyz-cloud <file path> : write an XYZ point cloud
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

## Version

0.1

## Author

Eric Pelzer (ecstatic.coder@gmail.com).

## License

This project is licensed under the GNU General Public License version 3.

See the [LICENSE.md](LICENSE.md) file for details.
