# Pico Project Template
As a demo, the code in this template simply turns on the onboard LED and continously prints "Hello, world!" to serial.

## How to Use

1. Click the "Use this template" button in the top-right of this page, and select "Create a new repository"
2. Fill out the "Create a new repository" page
    * Make sure the Owner is cornellrocketryteam
    * Name your project with capital words separated by dashes
    * Give your project a short description
    * Keep your project **Public**
4. Clone your new project!
    * _Make sure to clone with SSH_

## How to Run

### Required Dependencies
* ```cmake```
* ```gcc-arm-embedded``` (for macOS)
* ```gcc-arm-none-eabi``` (for Linux)

_These dependencies can be install via [Homebrew](https://brew.sh/) on macOS or [apt](https://ubuntu.com/server/docs/package-management) on WSL._

### Setup
1. Install the required dependencies
2. Run ```git submodule update --init``` from within ```lib/```
3. Run ```git submodule update --init``` from within ```lib/pico-sdk/```

## Running
1. Create a top-level ```build/``` directory
2. Run ```cmake ..``` from within ```build/```
3. Run ```make```
4. Move ```template.uf2``` to a Pico in boot select mode

_At this point, the onboard LED should turn on. If you open a serial connection port you uploaded over, you should see "Hello, world!" print every second._
