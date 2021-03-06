## Motor controller
The motor controller will set motor speed and direction based on input from remote controller messages and values from motor CS pin and contact sensor. It starts by driving the motor forward until one of the following events happen:

- receives a stop / change direction message from the remote control and acts accordingly
- the contact sensor is triggered, the motor stops and starts going in the oposite direction
- the motor gets stuck or has a hard time going forward, the motor stops and starts going in the oposite direction

Change of speed (stopping, starting) will be made using a linear acceleration / deceleration based on configured parameters.

### Configurable parameters
* **MAX_SPEED** - the top speed value for the motor (value from 0-255)
* **ACCELERATION** - when starting from speed 0, increase speed with this value on an interval until reaching MAX_SPEED (value from 0-255)
* **DECELERATION_PERCENTAGE** - defines how fast to slow down compared to **ACCELERATION** value, (e.g. for value 10, deceleratio will last 10% of the acceleration)
* **CURRENT_SENSING_THRESHOLD** - value for current sensing pin at which to stop and reverse direction (to fix cases where the motor gets stuck)

### Wireless communication
The controller accepts messages from wireless receiver and acts accordingly. Start/Stop/Change Direction actions will be made following **ACCELERATION** and **DECELERATION_PERCENTAGE** parameters.

#### Receive messages
* **SET_PARAMS** - sets the values for the configurable parameters as described above, so that next `loop` cycle will use new value
   * payload: 
* **TOGGLE_START_STOP** - toggles current state of motor
* **CHANGE_DIRECTION** - stop and start in oposite direction
* **START** - starts the motor if stopped. If already running, ignore
* **STOP** - stops the motor if running. If not running ignore
* **MOTOR_STATUS** - sends current motor status

#### Send messages
* **ACK** - acknowledges messages.
   * payload: 
        - same as initial message
        - in some cases (**MOTOR_STATUS**) parameters for the initial message are changed before responding (see below)
* **MOTOR_STATUS** - sends current motor status
   * payload: 
       - speed
       - direction
       - current pin 1
       - current pin 2
       

### Contact interrupt pin
Controller should read value for INT0/INT1 interrupt pin and change direction when value is changed. Do this by using `attachInterrupt`.

 ## Remote control controller

 Controller that sends messages to the motor controller array. Handles global start / stop time, sets configuration for each motor and sends individual motor commands.