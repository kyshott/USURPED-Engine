<div align="center">

# USURPED! Engine

<img src="bin/assets/textures/OLPORTRAIT.png" width="200">

---

## Created using C++ and Raylib, with level creation done in Tiled.

+ ### This project was created/started as part of my final project for the Game Engine Development course at WIT. 

+ ### All assets and code are included in this repository. As of September 2026, this is a preview of what the engine is capable of, demonstrated through a complete demo for a game.

+ ### I do intend to expand on USURPED! in the future with a variety of new features.

---

## Stack and External Sources/Credit

---

> ### [Raylib](https://www.raylib.com/)
> **Raylib was used for all rendering and sound components of the engine. Most other features outside of rendering and sound went unused.**

<br>

> ### [JSON for Modern C++](https://github.com/nlohmann/json)
> **The json single include from Niels Lohmann. Extremely important for loading enemy data, item data, and level data.**

<br>

> ### [Bxfr](https://www.bfxr.net/)
> **Used for the game sound design. Very intuitive and fun to use.**

<br>

> ### [Tiled](https://www.mapeditor.org/)
> **Used to create and export levels as usable json files to the engine.**

<br>

> ### [Tileson](https://github.com/SSBMTonberry/tileson)
> **Another extremely useful single include that streamlined the process of loading Tiled maps. Used in conjunction with the json single include.**

<br>

> ### [Aseprite](https://www.aseprite.org/)
> **The primary asset creation software used for USURPED!. All assets were created by myself in Aseprite.**

<br>

> ### [ImGui](https://github.com/ocornut/imgui)
> **Used for the "Test Room", allowing for UI-based debugging and messing around with different systems.

---

## Features

---

+ <ins>**Almost full ECS**:</ins> With a few exceptions. Every moving part of this engine is an "entity" in the entity component system.
<br>

+ <ins>**Full 2D AABB collision**:</ins> implemented with custom logic outside of Raylib's default implementation.
<br>

+ <ins>**"Room" camera system**:</ins> determines player position based on a room grid and positions the camera accordingly.
<br>

+ <ins>**Complete asset loading system**:</ins> for textures, sounds/music and JSON. All assets can be specified in a .txt file, along with any specifics such as size/scale, animation frames, etc.
<br>

+ <ins>**Dynamic Entity Management**:</ins> Entities are dynamically created and removed while avoiding iterator invalidation.
<br>

+ <ins>**Scene system**:</ins> A system that allows for easily swapping to and from "scenes", each with their own unique registered input, process flows, physics and more.
<br>

+ <ins>**"Action" system**:</ins> An action system that processes user inputs; registering new inputs to a scene is trivial with this system.
<br>

+ <ins>**Dynamic rendering**:</ins> Only textures visible by the engine camera are rendered at runtime, saving heavily on memory usage.




## 

## FONT CREDIT:
"Alagard" by Hewett Tsoi, 2013

## CONTROLS
SPACE - Select/interact
<br>
TAB - Open menu
<br>
BACKSPACE - Back
<br>
W/A/S/D - Up/Left/Down/Right
<br>
C - Change camera view (room/follow)

