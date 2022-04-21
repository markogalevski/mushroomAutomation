# Mushroom Growth Chamber Automation

This is a simple piece of code to manage my small mushroom growing chamber (affectionately called Martha)
  with the use of an Arduino, a sensor, and a couple of relays switching 2 independent power outlets I mounted
  onto a common PVC electrical housing.

Usage is simple:
 * `initFan` sets the fan to run for x minutes per \<timeframe\> minutes. The timeframes are defined to encompass
 most commonly used frames of reference - every 10, 15, 30, 45, or 60 minutes. Adding new timeframes is very
 simple, one simply adds them to the timeframe\_t enum. They're written to be as humanly readable as possible.
 It helps. If the number of on minutes is greater than the timeframe, it will never turn off.
 e.g. 'initFan(&fanInstance, FAN_RELAY_PIN, 5, PER_15_MINUTES)` defines a fan that runs for 5 minutes every 15
 minutes, and is controlled by whatever pin FAN_RELAY_PIN is.

 * `initHumidifier` sets a lower and upper humidity bound to allow the setting of an acceptable humidity range
 The magic of _hysteresis_. The humidifier turns on when the relative humidity is less than `rhLowerBound` and
 turns off when the humdity is greater than `rhUpperBound`. Rocket science.
 e.g. the call `initHumidifier(&hmdInstance, RELAY_HUMID_PIN, &sensor, 75.F, 80.F)` defines a range of 75-85% 
 humidity.


