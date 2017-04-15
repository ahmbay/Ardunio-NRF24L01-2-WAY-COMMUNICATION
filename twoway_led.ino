  
  /*
 Copyright (C) 2016 Ahmet Bay www.ahmbay.com
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 */
  
  
  
  /*
  / This Example uses the RF24 library written by J. Coliz @ https://github.com/maniacbug/RF24 , 
  / Write this same sketch to two different Nodes. upon power up it will place both nodes into receiving mode 
  / and await a button press. When a button press is detected it will switch to send mode, uppdate the leds on 
  / on other node and return to listening mode.
  / PINS:        NRF24l01         UNO/328p           Mega
  / CE              3        -        9               9
  / CSN             4        -        10              53
  / SCK             5        -        13              52
  / MOSI            6        -        11              51
  / MISO            7        -        12              50
  / IRQ             8        -        n/a             n/a
 */
  
  
  #include <SPI.h>
  #include "nRF24L01.h"
  #include "RF24.h"
  #include "printf.h"
  
  
  
  RF24 radio(9,10); //CE,CSN   uno=(9,10) mega=(9,53)
  
      
   // Buttons: where and how many 
  const uint8_t button_pins[] = { 5 }; // you can add more seperated by comma i.e.. (3, 4, 5)
  const uint8_t num_button_pins = sizeof(button_pins);//this just adds how many pins there are so they can be checked in succession later
  
  // Leds: where and how many
  const uint8_t led_pins[] = { 3 };   // you can add more seperated by comma i.e.. (3, 4, 5)
  const uint8_t num_led_pins = sizeof(led_pins); // same as as above for leds
  
  
  
  // Single radio pipe address for the 2 nodes to communicate. in this case were just havining them take turns on the same address
  const uint64_t pipe = 0xE8E8F0F0E1LL;
  
  
   uint8_t button_states[num_button_pins]; // place to store button values
   uint8_t led_states[num_led_pins]; // place to store led values
  
  
  
  
  // Setup---------------------Setup---------------------Setup
  
  void setup (void)
  {
  
  Serial.begin(9600);
  printf_begin(); // print to serial monitor... 
  
  
  
  printf("ahmbay.com rf2401 two way led test sketch %s\n"); // debug 
  
  radio.begin();  //starts nrf with whatever config settings have been set.
  radio.setRetries(15,15);// optionally, increase the delay between retries & # of retries
  
  
  radio.openReadingPipe(1,pipe); //opens preset pipe/channel for reading.
  radio.startListening();        // starts listen on above pipe/channel
  radio.printDetails();          // dumps the nrf settings to monitor for debugging
  
  int i = num_button_pins;                  //
  while(i--)                                //
    {                                       // Sets buttons pins high
      pinMode(button_pins[i],INPUT);        // Looks for ground when pressed
      digitalWrite(button_pins[i],HIGH);    //
    }  
  
  int L = num_led_pins;                          // Sets Led pins to on
    while(L--)                                   // till it recieves different info...
    {                                            //
      pinMode(led_pins[L],OUTPUT);               // if you wanted them off till there turned on
      led_states[L] = HIGH;                      // change HIGH to LOW
      digitalWrite(led_pins[L],led_states[L]);   //
    }
  
  
  
  }// end of setup=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  
  
  
  
  // Loop---------------------------Loop---------------------------Loop
  void loop (void)
  {
        
     chk_incoming();
     delay(20);
     chk_outgoing();
  
  }          //end of loop-=-=--=--=--==-=--=--=-=-=-===-=-=--=-
    
    
    
    
    
    
    
    void chk_incoming() // incoming fuction call...
    {
       
    if ( radio.available() ) // check if a payload came in....
    {
      // Dump the payloads until we've gotten everything
      bool done = false;
      while (!done)
      {
        // Fetch the payload, 
        done = radio.read( button_states, num_button_pins );

        
        printf("Button pressed\n\r"); // for debugging

        // takes recieved button state and toggles the correct led
        int i = num_led_pins;
        while(i--)
        {
          if ( button_states[i] )
          {
            led_states[i] ^= HIGH;
            digitalWrite(led_pins[i],led_states[i]);
          }
         }
        }
      }
    }
   
   
   
   
   
   void chk_outgoing() // chk buttons states to see if we need to send updated info to other unit... 
  {
    int i = num_button_pins;
    bool different = false;
    while(i--)
    {
      uint8_t state = ! digitalRead(button_pins[i]);
      if ( state != button_states[i] )
      {
        different = true;
        button_states[i] = state;
      }
    }
    if (different)// if button has been pressed then.....
    {
      TX_mode(); //calls Tx_mode function
    }
  } 
  


  
  
  void TX_mode()// TX_mode function......
  {
    radio.stopListening(); // must stop listening before you open a writing pipe  
    radio.openWritingPipe(pipe);
    printf("Sending...",button_states, num_button_pins); // for debuggging info
      bool ok = radio.write( button_states, num_button_pins ); // sends data
      if (ok) {              // checks if data was propperly received..
        printf("Ok\n\r");    // for debugging
        
        
        radio.openReadingPipe(1,pipe); // closes writing pipe and opens Reading pipe
        radio.startListening();        // sets listening mode so were receiving and we start all over again from the beginning...
        delay(200);
      }
      else {
        printf("Failed\n\r"); // for debugging " lets you know other did not received transmittion "
      } 
  }
