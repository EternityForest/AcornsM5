## Acorns M5: App loader for Squirrel on M5Stack


## Requirements

This needs the M5Ez library, Acorns, and the M5Stack library itself.

## App structure

An app is just a folder that contains an a.nut file. App names should be kept short.  Put apps in the sqapps directory on the SD card or in SPIFFS. Boot the device up, and you'll be able to select them! Any output from print() is buffered and shown when the app exits. Any errors immediately are shown on the screen.

If you want your app to have an icon, just add a file "a.jpg" to the app directory, and it will use that icon in the menu.

Try this example code for your first app:

`print(lorem())`

## UI Bindings in Squirrel
Most of these are pretty self explanatory

### drawString(str,[ x, y])
Set cursor position and draw string. If X and Y are not given, uses current cursor position.

### foregroundColor(r,g,b)
### backgroundcolor(r,g,b)

### buttonEvent([time])

Returns nil if there hasn't been an event, or "xy" where x is a, b, c fir the button name, and y is p,r, or l for press, release, or longpress.
Only returns one event per call. If you give a time param, blocks for that many ms or until at least one event is available

### menu(title, options)
Shows a menu with the list of options(Must be list of strings), and returns the index of the selected item.

### fillScreen(r,g,b)
### textBox(title, text)