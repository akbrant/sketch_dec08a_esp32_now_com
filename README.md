### Hydraulic Snow Plow with ESP32 wireless NES controller and hipster wooden controller
##HOWTO use software to control stuff that could hurt you
# You are solely responsible for adequate protection of persons and equipment used in connection with any of this software 

I have a F150 that I saved from the crusher after it was declared totaled. 'Free' to me (very long story).    
To match the truck I bought two very used but electrilly still working snow plows for super 'cheap' to match my sweet 'free' truck.
One plow I wired the controller using momentary on-off switches.  The other I got 'fancy' spent more than plows (and truck) on wire, relay boards, ESP32 and connectors.

Sure was fun and super glad it snowed a lot this winter for testing and debugging.

![unnamed](https://user-images.githubusercontent.com/1112030/165968575-66dfdc5e-fccb-4aa6-aa8d-adef67037519.jpg)
The machine!  Winter 2021 Juneau Alaska


![unnamed](https://user-images.githubusercontent.com/1112030/165968589-25de2a69-b3af-44e6-a489-ce0bda411bce.jpg)
Very slippery, steep gravel driveway. The machine's home turf.


![unnamed](https://user-images.githubusercontent.com/1112030/165968483-11024a02-c1eb-4ac4-bf4d-913862ef421e.jpg)
A relay board for the hydraulics (the eight little blue boxes).  The relay board is controlled via the esp32 that talks wirelessly to the NES controller.  
The NES controller is really just a bunch of on-off buttons.  Even the updown left right pad is really just four switches. 
The controller in the truck engine bay is powered by the 12v truck system.  The NES controller can use any USB power source.
Although my dogs really don't like the jerky ness of pushing heavy coastal snow around, sometimes I bring my dogs plowing.  
I definitely need some kind of dog safety or detection system so they don't step on the NES buttons!  
I just pull the power to the controller out of the USB power source in the place where you can light cigs if you smoke.


![unnamed](https://user-images.githubusercontent.com/1112030/165968535-86143e9c-910c-4199-807f-17b132f2f7c0.jpg)
This controller is much more dog safe and it also looks very wooden. It's quickly made from an oak pallet.  
My favorite USD currency,  Copper-Nickel coins are inlaid in the oak with a wire soldered on the back that connects to a capacitance pin on the ESP32.
You can touch the 'Nickels' with your finger (or tongue) and it will move the snow plow up, down, left, right and power down.


![unnamed](https://user-images.githubusercontent.com/1112030/165970403-526d7551-b9a2-4e28-b48c-fd917e648e58.jpg)
The rubbermaid all tosty and dry in the engine bay.  
