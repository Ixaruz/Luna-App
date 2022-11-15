Luna
=====
<div>
  <a href="https://github.com/Ixaruz/Luna-App">English</a> / <span>German (Deutsch)</span> 
</div>

![Banner](bannerslim.png?raw=true)
=====
[![2.0.0](https://img.shields.io/badge/Version-2.0.0-20306a)](#) [![2.0.1](https://img.shields.io/badge/Version-2.0.1-20306a)](#) [![2.0.2](https://img.shields.io/badge/Version-2.0.2-20306a)](#) [![2.0.3](https://img.shields.io/badge/Version-2.0.3-20306a)](#) [![2.0.4](https://img.shields.io/badge/Version-2.0.4-20306a)](#) [![2.0.5](https://img.shields.io/badge/Version-2.0.5-20306a)](#) [![2.0.6](https://img.shields.io/badge/Version-2.0.6-20306a)](#)  
Luna ist eine Homebrew-App, designiert zum Kopieren von Animal Crossing: New Horizons Inseln.

## Disclaimer & Lizensierung
[![Lizenz](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/Ixaruz/Luna-App/blob/main/LICENSE)
- Luna steht weder in Verbindung mit Nintendo noch dem Team hinter Animal Crossing: New Horzions und wird von diesen auch nicht vertrieben oder genehmigt.
- Luna wird bereitgestellt in der Hoffnung nützlich zu sein, jedoch **OHNE JEGLICHE GARANTIE**, stillschweigender Gewährleistung auf MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN SPEZIFISCHEN ZWECK. *Siehe GNU General Public License für weitere Details.*

## Schnellstartanleitung

### Grundvorraussetzung(en)

- Eine [modifizierte](https://nh-server.github.io/switch-guide/) Nintendo Switch Konsole mit [Atmosphère NX](https://github.com/Atmosphere-NX/Atmosphere)

### Installation

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/Ixaruz/Luna-App?label=Release)](https://github.com/Ixaruz/Luna-App/releases/latest)
[![Actions Status](https://github.com/Ixaruz/Luna-App/workflows/Build/badge.svg)](https://github.com/Ixaruz/Luna-App/actions)

Gehe zur [Release-Seite](https://github.com/Ixaruz/Luna-App/releases/), lade die letzte luna.zip herunter und extrahiere diese auf deine Switch SD Karte.   
Luna nutzt Speichervorlagen. Nutze entweder deinen eigenen Speicherstand<sup>[1]</sup> oder die bereitgestellte [enctemplate.zip](https://github.com/Ixaruz/Luna-App/raw/main/enctemplate.zip) und entpacke diese in   `SD:/config/luna/enctemplate`.

<br>
<sup>[1]</sup><smaller>unter der Vorraussetzung, dass der Speicherstand die gleiche Anzahl (oder mehr) an Bewohnern hat, wie die Insel, die du kopieren möchtest.

## Nutzung

Beim Besuch einer Insel, egal ob es eine Schlummerinsel oder die Insel eines Freundes ist, öffne das Homebrew-Menü (Album) und starte Luna.
Es zeigt die Option zum "Dump" und der Name der Insel, die du gerade besuchst, an.

Betätige nun den Dump-Knopf und gib Luna einige Sekunden zum Arbeiten.
Sollte alles gut gegangen sein, befindet sich nun ein kopierter Speicherstand im dump-Ordner.
Dieser Speicherstand kann nun vom Spiel oder von Tools wie NHSE genutzt und bearbeitet werden.

## Verzeichnisstruktur

      📦SD
       ┣ 📂config
       ┃ ┗ 📂luna
       ┃ ┃ ┣ 📂dump
       ┃ ┃ ┃ ┗ 📂[DA-XXXX-XXXX-XXXX] Inselname               # ein einzelner kopierter Speicherstand
       ┃ ┃ ┃ ┃ ┃ 
       ┃ ┃ ┃ ┃ ┗ 📂DD.MM.YYYY @ hh-mm                        # Datum und Uhrzeit des Traumes
       ┃ ┃ ┃ ┃                                                 Solltest du den Speicherstand laden wollen,
       ┃ ┃ ┃ ┃                                                 lade bitte genau diesen Ordner und nicht den Ordner darüber. :)
       ┃ ┃ ┃ ┃
       ┃ ┃ ┗ 📂enctemplate                                   # Ordner für Vorlagen; muss dem standardisierten
       ┃ ┃                                                     verschlüsselten Format entsprechen, welches du von jeglichen 
       ┃ ┃                                                     Speicherstandverwaltungs-Apps erhältst (Checkpoint, JKSV) 
       ┃ ┃
       ┗ 📂switch
         ┗ 📜luna.nro                                        #Luna homebrew app

<details><summary><h2>Building</summary>
<p>

- Compiler von [devkitpro/devkita64](https://switchbrew.org/wiki/Setting_up_Development_Environment)

</p>
</details>
  
## TO-DO:
- [x] Dokumentieren
- [x] Source Code hochladen
- [ ] ???
- [x] Veröffentlichen
  
## Credits
Vielen Dank an:
- WinterMute, fincs und weiteren Mitwirkenden an [devkitPro](https://devkitpro.org/) und devkitA64
- Yellows8 und alle Maintainer von [switch-examples](https://github.com/switchbrew/switch-examples)
- kwsch und Mitwirkenden an [NHSE](https://github.com/kwsch/NHSE)
- Ninji für deren [CylindricalEarth](https://github.com/Treeki/CylindricalEarth) Untersuchung
- Slattz, weil er eine geile Nutte ist und [LibACNH](https://github.com/Slattz/LibACNH) geschrieben hat
- WerWolv für das Beantworten meiner ständigen Fragerei

Luna basiert auf Atmosphères [Daybreak](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/daybreak), Adubbzs [NanoVG port auf deko3d](https://github.com/Adubbz/nanovg-deko3d) und das File-Handling von J-D-K's [JKSV](https://github.com/J-D-K/JKSV), als auch meine eigenen Untersuchungen zur Mechanik des Besuchens von Schlummer Inseln in Animal Crossing: New Horizons
