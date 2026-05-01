# 3d-model-viewer

A minimal C/OpenGL starter using GLFW on macOS. The current `main.c` opens a GLFW window, creates an OpenGL context, clears the screen, and keeps running until the window is closed.

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

Close the window to quit.

## VS Code

- Build: `Terminal > Run Build Task`
- Run/debug: open the Run and Debug panel and choose `Run OpenGL viewer`
