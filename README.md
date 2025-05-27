# 3D Raylib FPS Engine (Windows Only)

A first-person sandbox engine inspired by VotV, built with C and raylib. Features PSX-style graphics, inventory, weapons, and more!

![Repo Size](https://img.shields.io/github/repo-size/Yubbbey/3D-raylib-FPS-engine?label=Repo%20Size&color=blue)
![Code Size](https://img.shields.io/github/languages/code-size/Yubbbey/3D-raylib-FPS-engine?label=Code%20Size&color=green)

<img src="icon.png" alt="Project Icon" width="128">
![GameCapture1](https://github.com/user-attachments/assets/b8735f80-d9f4-41ca-91b2-0bd0d0be4f76)
![GameCapture2](https://github.com/user-attachments/assets/31a2a973-fdcd-44ad-aec8-1f06491fd1f6)
![GameCapture3](https://github.com/user-attachments/assets/dce022e2-f87f-4dff-831f-fb9eb978b15a)
![GameCapture4](https://github.com/user-attachments/assets/df35f98b-7323-430d-8d10-45d3e07401cb)

# Features

- [x] Easter Eggs (Maxwell or something else)
- [ ] Portals (like in Doom II)
- [x] Inventory System (Maximum Weight)
- [ ] Sandbox Type-Game
- [ ] Picking, launching system
- [ ] 3/4 Guns
- [ ] Train Station in a big map
- [ ] Package delivery, inspection, and theft
- [ ] Event-triggering packages (killer, bomb, monsters, ghosts, etc.)
- [x] PSX Style
- [ ] Vehicles (Truck, Motorcycle, Drivable Drone)
- [x] Human/Robot generated voices for creepy effect
- [x] NPCs

## References & Resources

- [Raylib Models Examples](https://github.com/raysan5/raylib/blob/4.2.0/examples/models)
- [Raylib Shaders Examples](https://www.raylib.com/examples/shaders/loader.html?name=shaders_basic_lighting)
- [Nuklear UI](https://github.com/vurtun/nuklear/issues/683)

## Development Steps

- [x] Player Controller
- [x] Model Loader
- [x] Pixelisation Shader
- [x] Shotgun Animation
- [x] Shotgun Mechanics
- [x] Gravity
- [ ] Other Movement Mechanics
- [ ] Collision (with models)

## Prerequisites

- [raylib](https://github.com/raysan5/raylib) (install and set up for your compiler)
- C/C++ compiler (e.g., MinGW, MSVC)

## Building the Project

1. Open `compiler.bat` to review the build commands.
2. Make sure the paths to raylib and other dependencies are correct.
3. Run `compiler.bat` to compile the project.

## Running the Game

After building, double-click `main.exe` on Windows or run:

```bash
./main
```

## License

[MIT](LICENSE)
