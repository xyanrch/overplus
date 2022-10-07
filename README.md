# overplus
It is a proxy server that support trojan protocol.
Overplus is another implement of trojan protocal with better perfermance and stability

## One-click deployment
Run the script and follow the assistant:

``` curl -O https://raw.githubusercontent.com/xyanrch/overplus/master/install.sh && chmod +x install.sh && sudo ./install.sh ```
 
 It is recommend to enable BBR to accelerate the network speed.

## Build
The project depend on boost and openssl libraries, please make sure install boost and openssl before build the project.

### how to build
``` 
mkdir build && cd build
cmake ..
make

```
### how to run

``` 
sudo ./overplus Server.json
```
Server.json is a config file which you can custom the config in.


 ## Client side 
 Overplus fully support trojan protocol, So it is Ok to use trojan's client. Please make sure disable certificate verify if you use a self issued certificate.

