# IR-Fan-Control
Turns on and off a fan with IR LED

**ir_recv_raw.ino** is used to get the IR signal from the remote. 
Source for that code is [this YouTube video](https://www.youtube.com/watch?v=JL821Ng-6vA) by Sayanee Basu

After the raw IR signal is obtained, that is put into data.h so it can be sent in the **ir_fan_ctrl.ino** file.

##Things ir_fan_ctrl.ino does:

* When the arduino boots up, it asks you to calibrate the keypad (16-button). Because it uses 1 analog pin in the arduino instead of 8 digital pins, it requires specific-ish thresholds to know what button to press, and I was getting different signal values each time it started up. Calibrating solved that.

* Once the program starts up, you can press the **A** button to set the *temp_on* value. Simply enter the value you want (default is °F, but you can switch the code to be °C instead), then press the **D** button to save the temperature.
  * Now if that temperature value is passed, the fan will be turned on automatically.
  * If you make a mistake, you can press **C** to erase the last digit you entered.

* Similarly, you can do the same to set the *temp_off* value, by pressing **B** instead.

* There is a 16x2 LED display that will show the current temperature on the top row, the *temp_on* value on the bottom left, and the *temp_off* value on the bottom right.  
![Image of LCD Display](https://github.com/Brendon-K/IR-Fan-Control/images/lcd.jpg)

* There is also a button that you can press manually to toggle the fan, but keep in mind that if you turn it off while the temperature is higher than *temp_on*, or turn it on while the temperature is lower than *temp_off*, then it will toggle back immediately after. (It's just following your directions.)

* Lastly, there is an LED that will turn on while the fan is on, and off while the fan is off. This is because the IR line of sight may not be perfect, and if the light and fan don't match up, that will let you know that your IR isn't reaching the fan correctly and you need to make adjustments.