* NX2/Nexus FDX Network protocol

This is a work in progress to read data from the Nexus / NX(2) network directly from a WSI Box.
The bus can be decoded to USB by the Garmin GND10 box. The protocol has be partly decoded by https://github.com/lkarsten/fdxread.
But i want to listen directly to the NX bus. The protocol directly on the NX bus is different to what the GND10 outputs.
The NX bus is a RS485 bus at 9600 baud, 8 data bits, one parity bit and one stop bit. I often heard 2 stop bits, but it seems to fit one stop bit better. It can also be viewed as a 9 bit bus, because the parity bit is not used for parity, but to mark the first byte of a message.
More on the format later..

I had some difficulty to read all 9 bytes/8 bytes + the parity bit. It can be done quite well, by using a rs485 transceiver (e.g. max485) and a logic analyzer. I then export the 9 bits into 2 bytes and decode after that with some python for now.
Reading with an usb rs485 transceiver did not work yet, because i did found no way to read the parity bit reliably.
I plan to read the protocol with an esp32 at the end. Since i can identify packages correctly (Thanks to correct checksums), i plan to read with an esp32 and software serial as a next step using https://github.com/plerup/espsoftwareserial. The library should allow reading the parity bit - this is not possible with the hardware serial on most mcus.


