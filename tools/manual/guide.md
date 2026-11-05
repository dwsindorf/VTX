# Introduction

VTX (the Virtual Terrain Explorer) is a program for exploring a procedurally generated universe, from whole galaxies down to the pebbles on a planet's surface. Nothing in a VTX universe is stored as a model or a map. Planets, clouds, rocks and plants are computed from short expressions while you watch, and each one gets more detailed as you get closer.

You can use VTX in three ways:

- **Explore.** Fly between stars, orbit planets and walk across their surfaces.
- **Edit.** Change any object through the Properties dialog, from the height of a mountain range to the color of a sunset, and see the result immediately.
- **Create.** Generate new stars, planets and moons with a single command, then refine them and save them as scenes or reusable objects.

This manual explains how to operate VTX. Part I is a guide to installing the program and to the tasks you will do most often. Part II is a reference to every menu, key and dialog control. Part II is generated from the program's source code, so it always matches the version of VTX it ships with.

How VTX works internally is covered in the separate VTX Theory of Operation document.

## Conventions

- Menu commands are written as **File > Open**.
- Keys are written as they are typed. VTX's keyboard commands are case-sensitive, so **s** and **S** are different commands.
- Values with units in the dialogs use the units shown in the control's label, for example *Day(hrs)* or *Ht(mls)* (miles).

# Installing and starting VTX

## System requirements

- Windows 10 or 11, 64-bit.
- A graphics card and driver with OpenGL support for GLSL and geometry shaders. A dedicated graphics card is strongly recommended.
- About 1 GB of free disk space, more if you generate many worlds or record movies.

## Installing

VTX is distributed in two forms:

- **Installer** (*VTX-win64-version-setup.exe*). Run it and follow the prompts. VTX is installed for the current user in *%LOCALAPPDATA%\Programs\VTX*, and no administrator rights are needed. The installer adds VTX to the Start menu, optionally to the desktop, and to Settings > Apps for uninstalling.
- **Zip archive** (*VTX-win64-version.zip*). Unpack it into any folder you can write to, for example your Documents or Downloads folder.

> **Important:** VTX writes into its own folders while it runs: generated shader files, saved scenes, preferences and generated textures. Don't install or unpack it under *C:\Program Files*, where ordinary users can't write.

## Starting VTX

Start VTX from the Start menu entry or desktop icon (installer), or by double-clicking **VTX.bat** in the VTX folder (zip). Don't start *bin\vtx.exe* directly from Explorer: VTX must start with the *bin* folder as its working directory, and the shortcut and VTX.bat take care of that.

When VTX quits it saves your session as *Saves\last.spx*. The next time you start it the same way, it reopens that session. The first time, it opens the default scene instead.

To pin VTX to the taskbar when using the zip version, create a shortcut whose target is `C:\Windows\System32\cmd.exe /c "C:\path\to\VTX\VTX.bat"`, set its **Run** option to *Minimized* and its icon to *bin\vtx.exe*, then right-click it and choose **Pin to taskbar**.

## Command-line options

VTX.bat passes its arguments to vtx.exe:

| Argument | Effect |
|---|---|
| `--s`, `--m`, `--b` | Small, medium (default) or big window |
| `--w`, `--p` | Wide or panoramic window |
| *name* | Open scene *name* from the Saves folder, for example `alpine` |
| `-`*key* | Apply a single-key command at startup, for example `-s` toggles shadows |

## The VTX folder

| Folder | Contents |
|---|---|
| bin | The program |
| Docs | This manual and the Theory of Operation |
| Saves | Scene files (*.spx*), including *last.spx* and *Default.spx* |
| Objects | Saved objects that can be added to scenes, by type |
| Textures | Image textures, plant images and sprite images |
| Bitmaps | Textures that VTX generates for new worlds (see Generating new worlds) |
| Images | Snapshots saved with **File > Save Image** |
| Movies | Movie frames |
| Resources | Star and cloud sprites, and saved preferences |
| Shaders | The program's GPU shaders |

## Graphics card selection

Many laptops have two graphics processors: a power-saving one built into the processor and a faster dedicated one. VTX asks Windows for the dedicated graphics card. If pictures look coarse or slow, check which one is in use: VTX prints it in its console window when it starts, on a line beginning *OpenGL:*. You can also set it in **Settings > System > Display > Graphics**: add *bin\vtx.exe* and choose **High performance**.

# The main window

[[SCREENSHOT: The main window, with the toolbar, a planet in the view and the on-screen key help showing]]

The main window shows the scene, with a menu bar and toolbar above it. Most toolbar buttons repeat menu commands. The buttons on the right open VTX's three dialogs:

- **Scene Dialog** opens the Properties dialog, where you edit the objects in the scene. The **Tab** key also opens it.
- **Image Editor** opens the dialog for creating and editing image textures.
- **Movie** opens the dialog for recording paths and movies.

## On-screen information and key help

VTX draws status information over the scene. Press **?** (or click the **Key help** toolbar button) to show or hide a list of the keyboard commands available in the current mode.

The function keys **F1** to **F6** switch between modes. Each mode gives the number keys **0** to **9** a different meaning:

| Key | Mode | Number keys |
|---|---|---|
| F1 | All (default) | Display and detail defaults |
| F2 | View | Turn, look and tilt the view |
| F3 | Movie | Record and play paths and movies |
| F4 | Stats | Show statistics about objects, maps and rendering |
| F5 | Colors | Color the terrain by internal properties (diagnostic) |
| F6 | Test | Alternative views for testing |

The full list for each mode is in Part II, Keyboard.

# Moving around

## View types

VTX has three ways of moving, which you choose with **View > Move to**, the toolbar or the keyboard:

- **Orbit** (**U**, that is SHIFT-U). The view circles the current object, typically a planet, at a distance. Use it to look at a whole world.
- **Surface** (**G**). The view moves over the surface of the current object, as if you were walking or flying above the ground.
- **Selection** (**M**). The view moves to the object you have selected.

## Keyboard movement

| Key | Action |
|---|---|
| Up / Down arrow | Move forward / back |
| Left / Right arrow | Turn left / right |
| u / d | Move up / down |
| Ctrl+u / Ctrl+d (also Ctrl+Up / Ctrl+Down) | Tilt the view up / down |
| , / . | Decrease / increase the speed of movement |
| ; | Reset the speed of movement |

In surface view, the F2 (View) mode adds keys to look and turn in fixed steps, and **9** and **0** switch between an overhead view and a normal surface view.

## Mouse movement

| Mouse | Orbit view | Surface view |
|---|---|---|
| Left-drag sideways | Circle around the object | Turn |
| Left-drag up or down | Move closer or farther | Move forward or back |
| Right-drag sideways | Circle around the object | Look to the side |
| Right-drag up or down | Tilt the view | Look up or down |
| Wheel | Zoom in or out | Lower or raise the viewpoint |
| Hold middle button | Fly forward while held | Fly forward while held |

## Selecting objects

Click an object in the view to select it: a star in the star field, a planet or a moon. The selection is shown on screen and in the Properties dialog. Press **Space** to clear the selection.

To travel to the selected object, use **View > Move to > Selection** (**SHIFT-M**). Selecting a star and moving to it is how you explore the universe: when you arrive at a star that has never been visited, VTX generates its planetary system on the spot.

## View history

VTX remembers the views you visit, so you can return to them:

| Key | Action |
|---|---|
| = | Mark the current view |
| - / + | Go to the previous / next view in the history |
| _ | Rewind to the first view |
| z | Undo the last move |
| Z | Clear the view history |

**Edit > Undo** and **Edit > Redo** (and the toolbar buttons) step through the same history. **View > Clear History** clears it.

# Detail, quality and speed

VTX keeps refining what you see until the scene is as detailed as the current settings allow. A scene often sharpens over a second or two after you stop moving.

## Scene quality

**View > Scene Quality** sets the overall level of detail:

| Setting | Key |
|---|---|
| Low | SHIFT-D |
| Medium | SHIFT-C |
| High | SHIFT-B |
| Very High | SHIFT-A |

**]** and **[** (**More Detail** and **Less Detail**) adjust the detail in smaller steps. Higher detail looks better but renders more slowly, so lower it while moving fast and raise it for pictures.

## Render options

The keyboard toggles rendering features on and off. The most useful are:

| Key | Feature |
|---|---|
| s | Shadows |
| f | Fog |
| x | Textures |
| b | Bump maps |
| l | Lighting |
| W | Water |
| a | Anti-aliasing |
| m | High dynamic range |
| S / N / L / P | Draw shaded (normal), solid, lines or points |
| R | Rebuild the scene |

With key help showing (**?**), the F1 list shows which features are on (**[X]**) and off (**[ ]**). The Scene object's tabs in the Properties dialog give finer control over the same settings.

**Rebuild** (**R**, or **Edit > Rebuild**) regenerates everything in view. Use it if the display looks wrong after many edits.

# Time and animation

Planets and moons rotate and orbit, and clouds drift. Time normally stands still, so you can study a view. To make time run:

- **Toolbar > Animate time** (or **View > Animate > Time**) starts and stops the clock.
- **<** and **>** decrease and increase the time rate. **:** resets it.
- **Toolbar > Animate motion** (or **View > Animate > Motion**) makes the view keep moving at the current speed, so you can glide over a surface hands-free.

The Scene object's **Display** tab shows the time and has a multiplier from x1 to x10^7 for fast-forwarding orbits.

# The Properties dialog

[[SCREENSHOT: The Properties dialog, with the object tree on the left and a Planet's Object tab on the right]]

The Properties dialog is where you inspect and change a scene. Open it with the **Scene Dialog** toolbar button or the **Tab** key.

## The object tree

The left side shows the scene as a tree, which mirrors how the universe nests: the Scene contains a Galaxy, which contains a System, which contains a Star and Planets, and a Planet contains its Moons, Sky, Clouds, Rings and a Surface with its layers and features.

Click an object in the tree to show its tabs on the right. Selecting an object in the view also selects it in the tree.

## Changing values

Each object type has its own set of tabs. Part II lists every tab and control. The controls work the same way everywhere:

- **Sliders** have a text box beside them. Drag the slider, or type a value in the box and press **Enter**.
- **Expression sliders** accept either a number or an expression, so a value can vary across a surface instead of being constant.
- **Color sliders** set a color with a color box and its intensity with a slider.
- **Expression fields** hold a complete terrain or color expression. Edit the text and press **Enter** to apply it.

Most changes appear in the view immediately. Changes that affect terrain shape take a moment while the surface is rebuilt.

### Editing parts of an expression

Expressions in a surface can be long. VTX can show the noise, fractal, crater and erosion functions in an expression as short tokens such as *Noise1*, and edit each one in its own dialog:

1. Double-click a token in an expression field to select it.
2. Right-click and choose **Edit**. The Noise or Function editor opens with that function's settings.
3. Change the settings. The expression updates when you apply them.

To insert a new function, right-click in the field with nothing selected and choose **Add > Noise**, **Fractal**, **Craters** or **Erode**.

## The object menu

Right-click an object in the tree for its menu. What appears depends on the object type. The most common commands are:

| Command | Action |
|---|---|
| Set View | Make this object the center of the view |
| Show | Show or hide the object |
| Enable | Turn a feature (such as fog or erosion) on or off without deleting it |
| Delete | Remove the object from the scene |
| Add | Add a child object: a moon to a planet, a cloud layer, a plant, and so on |
| Replace with.. | Replace the object with a saved one or a newly generated one |
| Save.. | Save the object to the Objects library so you can reuse it |
| Randomize | Give the object new random settings of the same kind |
| Default | Reset the object to its default settings |
| Remove file.. | Delete a saved object from the Objects library |

You can also rearrange the tree by dragging objects with the mouse. Dragging with the right mouse button copies an object instead of moving it.

# Generating new worlds

Every star you travel to gets a generated planetary system. You can also generate objects deliberately:

- **Add > *type* > Generate** adds a new generated object, for example a new moon around a planet.
- **Replace with.. > Generate** replaces an object with a newly generated one of the same type. Some types offer sub-types, for example rocky, icy or gas giant planets.
- **Add** or **Replace with..** followed by **<Random>** picks one of your saved objects of that type at random.
- **Randomize** keeps the object's type but gives it new random settings.

If you don't like a result, generate again. Each generation is different.

## Reusing generated textures

Generated worlds use texture images: height maps, erosion maps, layer images and color bands. Creating them takes time, so VTX can save them in the *Bitmaps* folder and reuse them for later worlds. Two settings on the Scene object's **Adapt** tab, in the *Procedural Generation* group, control this:

| Setting | Choices |
|---|---|
| Reuse | **All**: reuse saved images for everything. **2D** (default): reuse saved 2D maps, create new color bands. **None**: create every image new. |
| Keep | Which generated images to keep in *Bitmaps*: **All**, **2D** (default) or **None**. Images that aren't kept are deleted the next time VTX starts. |

With a new installation, *Bitmaps* is empty, so the first worlds you generate create all their own images. As *Bitmaps* fills up, generation gets faster and the 2D maps are drawn from a growing collection.

# Scenes and saved objects

## Opening and saving scenes

A scene is everything in the universe you are exploring, plus the views you have saved. Scenes are stored as text files with the extension *.spx* in the *Saves* folder.

- **File > Open** opens a scene.
- **File > Save** saves the current scene.
- VTX saves the current session to *last.spx* when it quits, and reopens it the next time it starts.

Scene files are plain text that describes the object tree and its expressions, so they can also be read and edited in a text editor. The Theory of Operation describes the format.

## Saved objects

Any object can be saved on its own with **Save..** in its tree menu. Saved objects go into the *Objects* folder, organized by type, and then appear in the **Add** and **Replace with..** menus of matching objects. This is the way to build a library of favorite planets, cloud layers, plants or rock types and reuse them across scenes.

# Pictures and movies

## Snapshots

**File > Save Image** (the **Snapshot** toolbar button) saves the current view as an image in the *Images* folder. **File > View Image** opens a saved image.

For the best result, raise the scene quality and let the view settle before you save.

## The Image Editor

[[SCREENSHOT: The Image Editor, showing the 1D (bands) tab]]

The Image Editor creates and edits the images that surfaces use as textures. Open it with the **Image Editor** toolbar button. Its tabs are:

| Tab | Purpose |
|---|---|
| 1D | Color bands: one-dimensional color gradients, used for layered and striped surfaces |
| 2D | Two-dimensional images generated from expressions |
| Img | Imported images from *Textures\Images* |
| Map | Imported surface maps |
| HMap | Imported height maps |
| Process | Image processing (for example blurring) of images in *Textures\Processed* |

## Movies

[[SCREENSHOT: The Movie dialog]]

VTX records a movie in two steps. First you record a *path* by flying through the scene. Then VTX replays the path and saves a frame at each step. Recording frames from a path gives smooth movement even when rendering each frame is slow.

Press **F3** to switch the number keys to Movie mode:

| Key | Action |
|---|---|
| 2 | Start recording a path. Move around the scene as you like. |
| 1 | Stop |
| 3 / 4 | Rewind / play the path |
| 5 / 6 | Save / load the path |
| 7 | Record movie frames from the path |
| 8 | Record movie frames live, as you move |
| 0 | Clear the path and movie |

The **Movie** toolbar button opens the Movie dialog, which has the same commands as buttons: **Clear**, **Load**, **Record**, **Rewind**, **Play** and **Save** for paths, and **Record**, **Test**, **Save** and **Play** for movies. Frames are recorded as images in the *Movies* folder.

**Save** turns the recorded frames into a video file, and **Play** plays one back. Both use the free FFmpeg tools (*ffmpeg* and *ffplay*), which aren't included with VTX: install FFmpeg and make sure its *bin* folder is on your PATH. In the Play dialog, **Exit at end** closes the player when the video finishes.

# Troubleshooting

**The picture is coarse or slow.** Check that VTX is using the dedicated graphics card (see Graphics card selection). Lower the scene quality while moving.

**Parts of the Properties dialog are cut off.** VTX's dialogs are laid out for 100% display scaling. If Windows display scaling (**Settings > System > Display > Scale**) is 125% or more, some controls may not fit. Setting the scale to 100% avoids this.

**VTX shows a shader error in its console.** The console lists the shader that failed. Make sure the *Shaders* folder is complete and writable. VTX writes a *.debug* copy of each shader there, which shows exactly what was sent to the graphics driver.

**Settings seem wrong after experimenting.** Press **0** in F1 mode to restore the display defaults, and **1** to restore the detail (adapt) defaults. Saved preferences are kept in *Resources\UI\prefs.spx*.

**The scene looks wrong after many edits.** Press **R** to rebuild it.
