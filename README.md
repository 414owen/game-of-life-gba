# Game of Life

## Setup

## Building

```
$ nix-shell
$ make game.gba
```

## Running

```
$ mgba game.gba
```

Or put it on a cartridge, for example the [everdrive](https://krikzz.com/store/home/42-everdrive-gba-x5.html), and run it on real hardware!

## Controls

| Control | Action                 |
| ------- | ---------------------- |
| Up      | Load next pattern      |
| Down    | Load previous pattern  |
| R       | Speed up simulation    |
| L       | Slow down simulation   |

## Demo

![demo.gif](demo.gif)

## Credits

* JShorthouse's [Compiling GBA programs on Linux with GCC](https://gist.github.com/JShorthouse/bfe49cdfad126e9163d9cb30fd3bf3c2)
* Jeff Frohwein's crt0.s
* buZz's ht.pl
* examples: http://www.conwaylife.com/patterns/all.zip
