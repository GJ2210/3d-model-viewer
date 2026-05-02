# 3d-model-viewer

A small desktop 3D model viewer written in C with GLFW and OpenGL.

The app opens a native window and loads Wavefront `.obj` models from the `models/` folder.

## Requirements

- macOS command line tools: `xcode-select --install`
- GLFW: `brew install glfw`

## Build

```sh
make
```

## Run

Run these commands from the main project folder, the same folder that contains `Makefile`, `main.c`, and `models/`.

```sh
make run
```

`make run` opens the viewer without a model. Press `L` inside the app to choose an `.obj` file from `models/`.

To open a specific OBJ file at launch:

```sh
make run-file MODEL=models/pyramid.obj
```

You can also pass just the filename when the file is already inside `models/`:

```sh
make run-file MODEL=pyramid.obj
```

## Controls

- `L`: load an `.obj` file from the `models/` folder
- Left drag: orbit camera
- Scroll: zoom
- `P`: toggle perspective/orthographic projection
- `S`: toggle flat, Gouraud, and Phong-style shading
- `W`: toggle wireframe
- `Esc`: quit

## VS Code

- Build: `Terminal > Run Build Task`
- Run/debug: open the Run and Debug panel and choose `Run OpenGL viewer`
