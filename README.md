Luna
====

<div>
  <span>English</span> / <a href="https://github.com/Ixaruz/Luna-App/blob/main/README-de.md">German (Deutsch)</a>
</div>

[![3.0.0](https://img.shields.io/badge/Version-3.0.0-20306a)](#) 

Luna is a homebrew app designed to dump Animal Crossing: New Horizons islands you visit.

## Disclaimer & License

[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/Ixaruz/Luna-App/blob/main/LICENSE)

- Luna is not affiliated with, endorsed or approved by Nintendo or Animal Crossing: New Horizons in any way.
- Luna is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. *See the GNU General Public License for more details.*

## Quick Start Guide

### Pre-Requisites

- A [modded](https://nh-server.github.io/switch-guide/) Nintendo Switch console running [Atmosphère NX](https://github.com/Atmosphere-NX/Atmosphere)

### Installation

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Ixaruz/Luna-App?label=Release)](https://github.com/Ixaruz/Luna-App/releases/latest)
[![Actions Status](https://github.com/Ixaruz/Luna-App/workflows/Build/badge.svg)](https://github.com/Ixaruz/Luna-App/actions)

Head to the [release page](https://github.com/Ixaruz/Luna-App/releases/) and download the latest released luna.zip and extract it onto your Switch's SD card.
Luna requires a save-file-template to work. either use your own save file`<sup>`[1]`</sup>` or use the included [enctemplate.zip](https://github.com/Ixaruz/Luna-App/raw/main/enctemplate.zip) and unzip it in   `SD:/config/luna/enctemplate`.

<br>
<sup>[1]</sup><smaller>under the requirement, that it has the same amount of players (or more) as the island you are trying to dump.

## Usage

When visiting an island, whether it being a dream or a friend's island, open the homebrew menu (album app) and start Luna.
It will show you the option to dump the current island and should also display their name.

Now press the Dump button and let it work for a few seconds.
If everything went right, you now have a dumped island in the dump folder, which can be used by the game and tools like NHSE.

## File Tree

    📦SD
       ┣ 📂config
       ┃ ┗ 📂luna
       ┃ ┃ ┣ 📂dump
       ┃ ┃ ┃ ┗ 📂[DA-XXXX-XXXX-XXXX] IslandName              # a singular dump folder
       ┃ ┃ ┃ ┃ ┃
       ┃ ┃ ┃ ┃ ┗ 📂DD.MM.YYYY @ hh-mm                        # internal date and time of the island you visited.
       ┃ ┃ ┃ ┃                                                 If you plan on using a save manager to load the save,
       ┃ ┃ ┃ ┃                                                 please load this exact folder and not the folder above. :)
       ┃ ┃ ┃ ┃
       ┃ ┃ ┗ 📂enctemplate                                   # folder where the template goes; has to be the standard
       ┃ ┃                                                     encrypted format, which you get from every save file manager
       ┃ ┃                                                     (Checkpoint, JKSV)
       ┃ ┃
       ┗ 📂switch
         ┗ 📜luna.nro                                        #Luna homebrew app

<details><summary><h2>Building</summary>
<p>

- requires [devkitpro/devkita64](https://switchbrew.org/wiki/Setting_up_Development_Environment) for compiling

</p>
</details>

## TO-DO:

- [X] write documentation
- [X] publish source code
- [ ] ???
- [X] release

## Credits

Thanks to:

- WinterMute, fincs and contributors of [devkitPro](https://devkitpro.org/) and specifically devkitA64
- Yellows8 and all the mantainers of [switch-examples](https://github.com/switchbrew/switch-examples)
- kwsch and contributors of [NHSE](https://github.com/kwsch/NHSE)
- Ninji for their [CylindricalEarth](https://github.com/Treeki/CylindricalEarth) research
- Slattz for being a bad bitch and creating [LibACNH](https://github.com/Slattz/LibACNH)
- WerWolv for handling my constant questioning

Luna is based on Atmosphère's [Daybreak](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/daybreak), Adubbz's [NanoVG port to deko3d](https://github.com/Adubbz/nanovg-deko3d) and the filehandling of J-D-K's [JKSV](https://github.com/J-D-K/JKSV), as well as my own arduous research on Animal Crossing: New Horizons' inner workings of visiting other people's islands.
