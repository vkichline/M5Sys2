# ButtonSizeTest

This is a test program intended to determine what the minimum reliable size is for a Touch Button.
It uses RopG's PR of the M5Core2 library (not yet integrated into the M5Core2 release) which is
located here: https://github.com/ropg/M5Core2.  I used checkin c96c4b5.
Note: Rop is working on an M5Sound branch on the same fork. This app doesn't work with the M5Sound fork.)

The test is a kind of a game: you try to find the smallest size buttons you can reliably and quickly press.
The size is initially 60 X 60, with no space between the buttons.
You can change the width, height and spacing by using the A, B and C buttons.

Press A to select "Width", "Height", or "Spacing."
Press B to increase the selected property (up to the maximum.)
Press B to decrease the selected property (down to the minimum.)

Six buttons are randomly selected and colored red. Press the red buttons only.
The app keeps score by printing results to Serial in pseudo-json format
(there is no opening/closing [] or {} around the collection.)

For example:
```
{ "date": "2020/10/17 15:20:53", "width": 60, "height": 60, "spacing":  0, "seconds": 4.30, "hits": 6, "misses": 0 }
{ "date": "2020/10/17 15:21:12", "width": 50, "height": 50, "spacing":  0, "seconds": 4.39, "hits": 6, "misses": 0 }
{ "date": "2020/10/17 15:21:25", "width": 40, "height": 40, "spacing":  6, "seconds": 4.91, "hits": 6, "misses": 1 }
```

I'm interested in learning what minimums various people find usable. Leave me a comment in Issues if you have some data for me.
