# Conway's Game of Life GBA

## Setup

## Building

```
$ # one-step to get a full cross-compilation toolchain!
$ nix-shell
$ make life.gba
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

![demo.apng](demo.apng)

## TODO (things I might pick up on a rainy day)

* Support other rulesets
  * Other amounts for stay-alive / birth rules
  * Other [neighborhood sets](https://www.conwaylife.com/wiki/Larger_than_Life)
* Rename 'boards' and 'rules' to 'patterns' in the codebase
* Break up ~board~ pattern C data generation into smaller compilation units
* Settings for
  * Toggling tails
  * Set tail length
  * Set colors
* Add editor
* Allow saving edited boards
* Show what pattern you're on (out of the >900 built-in)
* Allow you to 'favourite' patterns
* Menu to quickly browse / find patterns by name / type / size
* Sort patterns by type (oscillator / spaceship / gun / etc.)

If anyone wants to implement these, PRs are welcome.

## Credits

* JShorthouse's [Compiling GBA programs on Linux with GCC](https://gist.github.com/JShorthouse/bfe49cdfad126e9163d9cb30fd3bf3c2)
* Jeff Frohwein's crt0.s
* buZz's ht.pl
* examples: http://www.conwaylife.com/patterns/all.zip
