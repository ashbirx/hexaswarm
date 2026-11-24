<p align="center">
  <img src="screenshots/cover_image.png" width="720" alt="HEXASWARM cover art: a turret at the center firing at green and red hexagon enemies">
</p>

<p align="center">
  <b>Survive the swarm.</b> A minimalist top-down survival shooter built for the <b>raylib 6.x Game Jam</b>.
</p>

<p align="center">
  <a href="https://ashbirx.itch.io/hexaswarm"><img src="https://img.shields.io/badge/Play_on-itch.io-FA5C5C?style=for-the-badge&logo=itchdotio&logoColor=white" alt="Play on itch.io"></a>
  <a href="https://www.raylib.com/"><img src="https://img.shields.io/badge/Made_with-raylib-50C878?style=for-the-badge" alt="Made with raylib"></a>
  <img src="https://img.shields.io/badge/raylib_6.x-Game_Jam-DC3246?style=for-the-badge" alt="raylib 6.x Game Jam">
  <br>
  <img src="https://img.shields.io/badge/platforms-Windows_%7C_Linux_%7C_macOS_%7C_Web-23233A?style=flat-square" alt="Platforms: Windows, Linux, macOS, Web">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-zlib-23233A?style=flat-square" alt="License: zlib"></a>
</p>

## Trailer

https://github.com/user-attachments/assets/550a5588-a3d6-4743-b059-348f0e0484a3

<p align="center"><a href="https://ashbirx.itch.io/hexaswarm"><b>▶ Play it on itch.io</b></a></p>

## About

You control a stationary turret at the center of the arena. Rotate, aim, and shoot down the hexagonal enemies swarming in from every edge before they reach you. Enemies that wander too close to each other **merge** into a larger, angrier hex that stops wandering and heads straight for you. Survive as long as you can, rack up your score, and try to beat your own high score.

## Features

- **Rotate-and-shoot combat**: a single turret gun with full 360° rotation.
- **Emergent enemy merging**: regular hex enemies that stay near each other combine into a bigger, faster, more dangerous merged enemy that hunts you directly.
- **Persistent high score**: your best run is saved locally (desktop builds) and celebrated with a "New High Score" moment.

## Controls

| Key | Action |
| --- | --- |
| <kbd>←</kbd> <kbd>→</kbd> | Rotate turret |
| <kbd>Space</kbd> | Shoot |
| <kbd>P</kbd> | Pause / Resume |
| <kbd>H</kbd> | How To Play / Home (depends on the menu) |
| <kbd>R</kbd> | Restart |
| <kbd>B</kbd> | Back (from the How To Play screen) |
| <kbd>Q</kbd> | Quit |

## Screenshots

<table>
  <tr>
    <td align="center">
      <img src="screenshots/title_screen.png" width="260" alt="Title screen"><br>
      <sub>Title Screen</sub>
    </td>
    <td align="center">
      <img src="screenshots/gameplay.png" width="260" alt="Gameplay"><br>
      <sub>Gameplay</sub>
    </td>
    <td align="center">
      <img src="screenshots/merged_enemy.png" width="260" alt="A merged enemy"><br>
      <sub>Enemy Merge in Action</sub>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="screenshots/how_to_play.png" width="260" alt="How To Play screen"><br>
      <sub>How To Play</sub>
    </td>
    <td align="center">
      <img src="screenshots/pause_screen.png" width="260" alt="Pause overlay"><br>
      <sub>Pause Overlay</sub>
    </td>
    <td align="center">
      <img src="screenshots/new_high_score.png" width="260" alt="New high score screen"><br>
      <sub>New High Score</sub>
    </td>
  </tr>
</table>

## Developer

- Ashish ([ashbirx](https://github.com/ashbirx)): game design, programming, pixel art, and all game systems

## Credits / Third-Party Assets

- Built with [raylib](https://www.raylib.com/) by Ramon Santamaria ([@raysan5](https://github.com/raysan5)), on top of the [raylib gamejam template](https://github.com/raysan5/raylib-gamejam-template)
- Fonts: **m6x11** and **m5x7** by Daniel Linssen ([managore](https://managore.itch.io/))
- SFX: `hurt.ogg`, `select.ogg`, `shoot.ogg` — from Kenney's [Sci-Fi Sounds](https://kenney.nl/assets/sci-fi-sounds) asset pack. _(actual file names have been renamed)_
- SFX: `kill.wav`, `merge.wav` — generated with [rFXGen](https://raylibtech.itch.io/rfxgen) by raylibtech
- VFX: [`vfx.h`](./src/vfx.h) (muzzle flash, hit flash, death poof, merge pulse, hit vignette, heart-pop) — written with Claude

## License

This project's sources are licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

Fonts and other third-party assets retain their own original licenses — see their respective sources linked above.

_Copyright (c) 2026 Ashish ([ashbirx](https://github.com/ashbirx))_
