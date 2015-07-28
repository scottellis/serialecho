## serialecho

Simple serial port test program for new boards.

Run with the tx and rx jumpered.

        root@beaglebone:~# serialecho -h
        
        Usage: serialecho [-d <device>] [-s <speed>] [-h]
          -d <device>    Serial device, default is /dev/ttyO4
          -s <speed>     Speed, default is 115200
          -h             Show this help


        root@beaglebone:~# serialecho
        
        --- ctrl-c to stop ---

        Wrote: ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz
        Read : ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz
        
        Wrote: ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz
        Read : ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz
        
        Wrote: ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz
        Read : ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz
        ^C
