Marquette Roadmap
=================

Version 0
---------
"Lets get something out there"

* Basic functionality and send and receive MQTT messages
* Two Tile types:
  - Button (send)
  - Text (receive)


Version 1
---------
"Make everything less hacky"

* Multiple sizes of tiles
* Modular and extensible Tile Types system
* New built-in tile types:
  - Push button
    - style - icon?
    - topic
    - message
  - Toggle switch
    - 
  - Dial 
  - Slider control
    - Min
    - Max
  - Title
    - topic
  - Text
  - Numeric
    - type
    - topic
    - units
* Add a test suite


Version 2
---------

* Theming engine
* Support direct communication with Node-RED, without using a MQTT broker

