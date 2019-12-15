# Plasma

Some different C implementations of the classic [plasma](https://en.wikipedia.org/wiki/Plasma_effect) effect.

## Setup

All the demos require the SDL2 library.

### OSX

```sh
brew install sdl2
```

### Linux

```sh
sudo apt-get install libsdl2-dev
```

Or, if on another distribution, use the package manager available.

## Building

All the demos can be built with [Make](https://www.gnu.org/software/make/).

Build all demos:

```sh
make
```

Build a single demo:

```sh
make {{demo_name}}
```

Replace `{{demo_name}}` with any of the following:

* `color_cycling_plasma`

## Demos

### Color cycling plasma

![color-cycling-plasma](previews/color-cycling-plasma-preview.png)

A software rendered Plasma which is precalculated and cycles through a color palette. It is somewhat less dynamic than the other demos, but, runs quite fast at high resolutions by avoiding lots of sin calculations at runtime.

#### Run

Compile the demo:

```sh
make color_cycling_plasma
```

Run it:

```sh
./color_cycling_plasma
```


## References

- https://en.wikipedia.org/wiki/Plasma_effect
- https://lodev.org/cgtutor/plasma.html
- https://www.bidouille.org/prog/plasma
