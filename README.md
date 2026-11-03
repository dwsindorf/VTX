# VTX

**Virtual Terrain Explorer: a procedural universe generator and renderer.**

VTX generates a universe of galaxies, star systems, planets and moons, then lets you fly from deep space down to a planet's surface. Scenes are built almost entirely from procedural techniques, so detail keeps increasing as you get closer.

![Volumetric clouds over a rocky lake](Images/volumetric-clouds-lake.jpg)

| | |
|:---:|:---:|
| ![A ringed planet rising over a moon's surface](Images/ringed-planet-rise.jpg) | ![A shoreline with rocks and plants](Images/rocky-shoreline.jpg) |
| ![Spiral galaxy](Images/spiral.jpg) | ![Pine forest with rocks](Images/pines-rocks.jpg) |

More screenshots are in the [Images](Images) directory.

## Features

- **Objects:** galaxies, stars, planets, moons, rings and asteroids
- **Terrain:** multiple layers, craters, erosion, oceans and snow
- **Surface detail:** 3D rocks, plants and sprites
- **Atmosphere:** sky, haze, fog, and 2D, 3D and ray-marched volumetric clouds
- **Lighting:** shadows, eclipses, water reflections and HDR
- **Scene editing:** a graphical editor, or a text-based scene language you can edit by hand

VTX is written in C++ using OpenGL and GLSL for graphics and wxWidgets for the UI.

## Getting started

### Requirements

- Memory: 8 GB or more
- Graphics: NVIDIA GTX 980 or better (Intel integrated graphics will not work)
- Operating system: tested on Windows 10 and 11, and Ubuntu 18.04
- [Git Bash](https://git-scm.com/downloads) on Windows

### Run VTX

```sh
git clone https://github.com/dwsindorf/VTX
cd VTX/binaries
./vtx          # Windows
./vtx-linux    # Linux
```

You should see the startup window:

![VTX startup window](Resources/Screenshots/VTX-Startup-window.jpg)

### Visit a new star system

1. Select a star in the star field and press **Go to Orbit** in the toolbar.

   ![Selecting a star](Resources/Screenshots/select-star-and-go-to-orbit.jpg)

2. Open the Scene Tree dialog.

   ![A new star system](Resources/Screenshots/new-star-system-view.jpg)

3. Select an object in the system to view it.

   ![Setting the view in the Scene Tree dialog](Resources/Screenshots/scenetree-dialog-set-view.jpg)

4. View or edit the selected object.

   ![A randomly generated gas giant](Resources/Screenshots/new-random-gas-giant.png)

You can repeat this from any star in the new system.
