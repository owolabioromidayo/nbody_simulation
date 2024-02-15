# N-Body Simulation using OpenGL and OpenMP

This project implements an N-Body simulation using OpenGL for rendering and OpenMP for parallelization. OpenGL rendering is single-threaded, so parallelization efforts are focused on other tasks. 
<br />
The usage of OpenMP increases FPS by 2-3x.

## Usage

To compile the program, use the following command:

```bash
gcc nbody.c -o nsim -lGL -lGLU -lglut -fopenmp -lm -ldl

./nsim [DT] [NUM_BODIES] [USE_ORBITAL]
```

## EXAMPLES

### Orbital Configuration (50 Bodies)
```bash
./nsim 0.001 50 1
```
<video controls>
    <source src="doc/1.mp4" type="video/mp4">
    Your browser does not support the video tag.
</video>

### Random Configuration (1000 bodies) with paths off
```bash
./nsim 0.01 1000 0
```
<video controls>
    <source src="doc/2.mp4" type="video/mp4">
    Your browser does not support the video tag.
</video>

## MOVEMENT
You can move around using WASD for up,down,left,right, Q and E for forwards and
backwards, and click and drag for angular panning.



## CONFIG
You can adjust the following values in the header file (`nbody.h`):

- `G`: Gravitational constant for the simulation.
- `SHOW_PATHS`: Flag to control the display of paths.
- `MAX_POINTS`: Maximum number of points per body.
- `USE_OPENMP`: Flag to enable/disable OpenMP.
- `STAR_MASS`: Mass of the central star.
- `ORBITAL_SPEED_CONSTANT`: Constant for orbital speed.


## NOTES

- Collisions are left as is. As a result, when bodies get very close to each other,
the magnitude of the resulting force slingshots them away. It is very easy to cater
to this but I don't think I want to.

