# openMerc Client

## Setting IP
The IP address for the server must be changed in two places:
* `main.c` in  the function: `WinMain` - `strcmp(host_addr,"127.0.0.1")`
* `common.h` - `#define MHOST  "127.0.0.1"`


# Enable Sprite Packer
* `common.h` - uncomment `//#define DOCONVERT  // enable sprite packer`

This will take graphics from the `gfx` folder and make the packed `.dat` and `.idx` files
`TODO:` Check to see if it needs ALL graphics, or can append/replace.
