# cipri-muresan
Arduino controller for Cipri Mureșan sculpture

## Motor controller
The motor controller will set motor speed and direction based on input from remote controller messages and values from motor CS pin and contact sensor. It starts by driving the motor forward until one of the following events happen:

- receives a stop / change direction message from the remote control and acts accordingly
- the contact sensor is triggered, the motor stops and starts going in the oposite direction
- the motor gets stuck or has a hard time going forward, the motor stops and starts going in the oposite direction

Change of speed (stopping, starting) will be made using a linear acceleration / deceleration based on configured parameters.

### Configurable parameters
* **MAX_SPEED** - the top speed value for the motor (value from 0-255)
* **ACCELERATION** - when starting from speed 0, increase speed with this value on an interval until reaching MAX_SPEED (value from 0-255)
* **DECELERATION_PROPORTION** - defines at what percentage from **ACCELERATION** to decrease speed when stoping(value from 0-1, i.e a value of 0.5 will result in deceleration rate of **ACCELERATION**/2)
* **CURRENT_SENSING_TRESHOLD** - value for current sensing pin at which to stop and reverse direction (to fix cases where the motor gets stuck)

### Wireless communication
The controller accepts messages from wireless receiver and acts accordingly. Start/Stop/Change Direction actions will be made following *ACCELERATION* and *DECELERATION_PROPORTION* parameters.

#### Receive messages
* **SET_PARAMS** - sets the values for the configurable parameters as described above, so that next `loop` cycle will use new value
   * payload: 
* **TOGGLE_START_STOP** - toggles current state of motor
* **CHANGE_DIRECTION** - stop and start in oposite direction
* **START** - starts the motor if stopped. If already running, ignore
* **STOP** - stops the motor if running. If not running ignore

#### Send messages
* **ACK** - acknowledges messages.
   * payload: 
* **MOTOR_STATUS** - sends current motor status
   * payload: 

### Contact interrupt pin
Controller should read value for INT0/INT1 interrupt pin and change direction when value is changed. Do this by using `attachInterrupt`.

 ## Remote control controller
*TBD*
 Controller that sends messages to the motor controller array. Handles global start / stop time, sets configuration for each motor and sends individual motor commands.


