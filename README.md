This is my chip 8 implementation.

### Prerequisites
To download Allegro, refer to this quick start: https://github.com/liballeg/allegro_wiki/wiki/Quickstart  
To ensure that Allegro is working correctly, you can download and compile their sample hello.c on that page as well.  
You will need to run it on a Windows OS since it includes <windows.h>.     
I used MinGW-w64 on Windows 10 to compile it originally.  

### Build
You should just be able to run ```make``` and it should compile.  
If you have any issues, try using the full path to your gcc on the top-line of the Makefile.

#### Notes
It defaults to loading the Tic-Tac-Toe game in the roms folder. You can edit that to run different programs.

#### Keypad  
+-+-+-+-+  ===  +-+-+-+-+  
|1 |2 |3 |4| &emsp; &ensp; |1|2|3|C|  
|Q|W|E|R| &emsp; &ensp; |4|5|6|D|  
|A |S |D|F| &emsp; &ensp; |7|8|9|E|   
|Z |X |C|V| &emsp; &ensp; |A|0|B|F|  