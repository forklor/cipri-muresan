# cipri-muresan
Arduino controller for Cipri Mureșan sculpture

# Setup 
Have the follwoing libraries intalled in your library folder:
  * [RF24](https://github.com/nRF24/RF24) - wireless controller library
  * [LCD Library](https://bitbucket.org/fmalpartida/new-liquidcrystal) - library for LCD module (using `LiquidCrystal_I2C`)
  * [Menu Backend](https://github.com/Orange-Cat/MenuBackend) - library for building menus and handle navigation
  * [Keypad](https://github.com/Chris--A/Keypad) - library for handling keypad events

# Running / Building

The entry file is `main.ino`. To configure if you want to build the motor or the remote control version of the code and to specify which motor is the build target, you need to change `config.h`. Comment or uncomment and change the values for the definitions as described below:

* `REMOTE_CONTROL_CONTROLLER` - value `1` or `true` if the build target is the remote control arduino module
* `MOTOR_CONTROLLER` - value `1` or `true` if the build target is a motor arduino module
* `MOTOR_MODULE_NUMBER` - value from `1` to `6` that defines for which motor the build is made. Each motor uses a different address to comunicate with the remote control module