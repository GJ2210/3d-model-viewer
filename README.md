HOW TO RUN
----------

1. Open Terminal and go to the build folder:

   cd build

2. Build and run:
   
   cmake ../
   make && ./ModelViewer ../models/teapot.obj

   To load a layout:
   ./ModelViewer ../models/cornellbox

   If no model is given it defaults to models/cube.obj


CONTROLS
--------

Mouse:
  Left click + drag      Orbit camera around model
  Right click + drag     Pan camera
  Scroll wheel           Zoom in / out

Shading:

  1                      Flat shading
  2                      Gouraud shading
  3                      Phong shading (default)
 
Other:

  W                      Toggle wireframe overlay on/off
  P                      Switch between Perspective and Orthographic projection
  R                      Reset camera to default position

Light:

  Arrow keys             Move light left / right / forward / back
  = (equals)             Move light up
  - (minus)              Move light down

  ESC                    Quit


NOTES
-----
- Current shading mode, projection, and wireframe status are shown in the window title bar
- If the window doesnt appear, check if it opened behind other windows (Cmd+Tab)
- Run from a real Terminal window, not the VS Code terminal
