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

<br>

> ### [Alagard Font](https://www.dafont.com/alagard.font)
> **The primary font used for the game itself. Credits to Hewett Tsoi, 2013.**

---

## Engine Features

---

+ <ins>**Almost full ECS**:</ins> With a few exceptions. Every moving part of this engine is an "entity" in the entity component system.
<br>

+ <ins>**Full 2D AABB collision**:</ins> Implemented with custom logic outside of Raylib's default implementation.
<br>

+ <ins>**"Room" camera system**:</ins> Determines player position based on a room grid and positions the camera accordingly.
<br>

+ <ins>**Complete asset loading system**:</ins> For textures, sounds/music and JSON. All assets can be specified in a .txt file, along with any specifics such as size/scale, animation frames, etc.
<br>

+ <ins>**Dynamic Entity Management**:</ins> Entities are dynamically created and removed while avoiding iterator invalidation.
<br>

+ <ins>**Scene system**:</ins> A system that allows for easily swapping to and from "scenes", each with their own unique registered input, process flows, physics and more.
<br>

+ <ins>**"Action" system**:</ins> An action system that processes user inputs; registering new inputs to a scene is trivial with this system.
<br>

+ <ins>**Dynamic rendering**:</ins> Only textures visible by the engine camera are rendered at runtime, saving heavily on memory usage.

---

## USURPED!

---

### USURPED! is a roguelike dungeon-crawler, inspired by early JRPG games such as Dragon Quest and Final Fantasy.

---

<img src="bin/assets/screenshots/screencap7.png" width="600">

### You have been Usurped! Reclaim your dark fortress from the heroes who defeated you.

### Take the reigns as the dungeon boss and battle your way back to your throne. Collect weapons, magic and items along the way to help in your ambitions.

<img src="bin/assets/textures/OLSTANDD.png" width="200">

---

## Game Overview

---

### An "exploration" scene, where the player travels around the dungeon collecting loot and maneuvering around enemies.

<img src="bin/assets/screenshots/gif1.gif" width="600">

---

### A "battle" scene, where the player must strategically select their actions to defeat enemies.

<img src="bin/assets/screenshots/gif2.gif" width="600">

---

### Enemy raycast pathfinding, complete with re-pathing back to a home location and vision blocking.

<img src="bin/assets/screenshots/gif4.gif" width="600">

---

### Over 25 weapons/spells and 16 different enemy types!

<img src="bin/assets/screenshots/gif3.gif" width="600">

---

## Game Features

---

+ <ins>**Turn-based 1-on-1 battles**:</ins> Similarly to Dragon Quest 1, the player battles enemies in a turn-based, 1 on 1 format. Battles use an FSM with a queue to determine what actions happen in battle.
<br>

+ <ins>**Dynamic stat system**:</ins> The player, and enemies, all have stats that determine how much damage they do, how much damage they can take, who goes first and more. 
<br>

+ <ins>**Random floor selection**:</ins> Levels are randomly selected from a pre-generated pool; you can never get the same floor layout twice in a row.
<br>

+ <ins>**Random loot**:</ins> Loot from chests on each floor will always be randomized, selected from a pool of items available for the current floor number. Enemies may also occasionally drop useful items.
<br>

+ <ins>**Scaling difficulty**:</ins> As the player goes higher in the fortress, the enemies get much tougher. However, so does the loot the player can find.
<br>

+ <ins>**Dynamic damage types**:</ins> Both physical weapons and magic attacks have different damage types, which different enemies are susceptible to. Learn each enemy's weakness and master the dark fortress.
<br>

+ <ins>**Special magic**:</ins> Some magic can be used both in and outside of battle to heal the player. Additionally, some spells apply an over-time effect rather than direct damage. 
<br>

+ <ins>**Challenging boss battles**:</ins> There are currently 5 different bosses the player can face. Each boss has their own unique and very challenging battle style.
<br>

## How To Play

---

> <kbd>Space</kbd> - Select/interact
> <br>
> <kbd>Tab</kbd> - Open menu
> <br>
> <kbd>Backspace</kbd> - Back
> <br>
> <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> - Up/Left/Down/Right
> <br>
> <kbd>C</kbd> - Change camera view (room/follow)

> + Select "Start" to begin a new run.
> + Select "Test Room" to go into the debug room and test out various engine features.
> + Select "Quit" to close out of the application.

Once you have started a run, your goal is to reach the highest floor you can. Collect loot from chests and battle enemies to become more powerful. Enemies will get increasingly difficult with each floor, so be ready!
If your HP drops to 0, the run ends and your progress is reset. Your best run is saved, and can be viewed from the title screen.

---

