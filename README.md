#  SFML Archer Duel

A simple, fast-paced 2D archery duel game built with C++20 and SFML. 

##  Gameplay Showcase
https://github.com/user-attachments/assets/57179116-de5a-4da6-945c-9971807972ad

##  Features
* **Arena Duels:** 1v1 combat with AI enemies.
* **Progression System:** Earn points for winning and spend them in the shop to upgrade your bow's stats.
* **Custom 2D Physics:** Built from scratch to handle arrow trajectories, collisions, and entity movements.
* **Save/Load System:** Simple file I/O to keep your progress and upgrades safe between sessions.
* **Procedural Map Generation:** Platform layouts are randomly generated for each match.
* **Shootable Power-ups:** Hit baloons with arrows to gain tactical buffs like increased movement speed or higher jumps.

## Technologies & Tools
* **Language:** C++20
* **Graphics & Windowing:** [SFML 3.0.0](https://www.sfml-dev.org/)
* **Build System:** CMake

##  How to Build and Run
Thanks to CMake `FetchContent`, the project automatically downloads and links SFML.

### Prerequisites:
* CMake (3.28 or higher)
* A C++20 compatible compiler

### Build steps:
1. Clone the repository
```bash
git clone https://github.com/Kamil-Karwacki/sfml-archer-duel.git
cd sfml-archer-duel
```

2. Configure the project
```bash
cmake -S . -B build
```

3. Build the game (creates a standalone executable with static SFML)
```bash
cmake --build build --config Release
```
The compiled game, along with the required assets/ and saves/ folders, will be located in the build directory.

## Controls
    [Arrows] - Move

    [Left Mouse Button] - Shoot arrow / Interact with UI
