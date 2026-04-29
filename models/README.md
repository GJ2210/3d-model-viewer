# Models

Place Wavefront OBJ files here.

The viewer is launched with:

```
./ModelViewer path/to/model.obj
```

If no path is given it looks for `models/cube.obj` in the working directory.

## Suggested free models

- **Stanford Bunny** – search "Stanford bunny OBJ" on the Stanford 3D repository
- **Utah Teapot** – widely available as teapot.obj
- **Suzanne** – export from Blender (File -> Export -> Wavefront OBJ)

## OBJ requirements

- Triangle or polygon faces (`f` lines); polygons are fan-triangulated.
- Vertex normals (`vn`) are parsed but not currently used — the viewer
  computes its own normals from geometry.
- Texture coordinates (`vt`) are parsed but not yet rendered (stretch goal).
