# I2C_SPI_ESP32
A small project for the SY23 course about designing a ESP32 client which sends data from I2C and SPI sensors to a webserver on Raspberry Pi. The I2C sensor is a PmodHygro temperature&humidity, and the SPI sensor is a PmodNav which records temperature and air pressure. 

The webserver on Raspberry Pi operates with the CGI script within the cgi-bin, for more documentation: https://alirumane.github.io/2016/04/10/setting-lighttpd-on-raspberry-pi (You need a static IP on RPi) The logs are stored within the path precised in the CGI script, change it to wherever you need it and make sure you have permission to execute the script and write to the log folder.
