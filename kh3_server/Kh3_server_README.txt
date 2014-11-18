Kh3 server: This is application waits for Khepera 3 commands A-Z received from Bluetooth serial port (/dev/ttyS1)
						Needed when Korebot 2 is mounted on the Khepera 3

K-Team S.A

Version
 1.0 20120120 JT: draft

For running Khepera3 Interface (http://ftp.k-team.com/KheperaIII/applications/Interface_Khepera3_Setup_V1-1_port_com.zip) (or any other program responding to A-Z commands),if the Korebot 2 installed, you must run kh3server on the Korebot2.

- connect to the korebot2 mounted on your Khepera3

- edit the file /etc/inittab and comment line "S1:2345:respawn:/sbin/getty 115200 ttyS1" by adding a # in the front.
  The line may not be present if the Korebot was shipped alone! Don't comment any other line! 
  !!!!!!!!!!!!!!!!!!!!!!! 
  !!!!!!!! CAUTION  !!!!!  Pay attention to comment the right line! Making any error in this file may prevent the board 
  !!!!!!!!!!!!!!!!!!!!!!!  booting!

- reboot your Khepera3 (command reboot)
   
- configure your Bluetooth to connect to the Khepera3 as mentionned on the Khepera3 user's manual, chapter 4.4.

- copy kh3server to the Korebot 2

- make it executable with: chmod +x kh3server

- run it with: ./kh3server

- connect Khepera3 interface (or any other program) and run A-Z commands as usual.

- to stop it, push CTRL and c keys. 


The source code of kh3server is available in the libkorebot source, from version 1.18, in the directory src/tests.
