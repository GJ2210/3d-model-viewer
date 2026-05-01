# 3d-model-viewer

A small desktop 3D model viewer written in C with GLFW and OpenGL.

The app opens a native window, displays a demo cube, and can load Wavefront `.obj` models.

## Requirements

- macOS command line tools: `xcode-select --install`
- GLFW: `brew install glfw`

## Build

```sh
make
```

## Run

```sh
make run
```

To open a specific OBJ file at launch:

```sh
make run-file MODEL=/path/to/model.obj
```

Try the included sample model:

```sh
make run-file MODEL=models/pyramid.obj
```

## Controls

- `L` or `O`: load an `.obj` file with the macOS file picker
- Left drag: orbit camera
- Scroll: zoom
- `P`: toggle perspective/orthographic projection
- `W`: toggle wireframe
- `R`: reset camera
- `Esc`: quit

## VS Code

- Build: `Terminal > Run Build Task`
- Run/debug: open the Run and Debug panel and choose `Run OpenGL viewer`
