# overplus
Overplus is another implement of trojan protocal with better perfermance and stability.

**Comapared with trojan**:
- [x] It don't need to setup a http server on port 80 in your linux server.
- [x] It don't need to apply a domain name.
- [x] More readable and cleaner code.




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
sudo ./overplus server.json
```
server.json is a config file which you can custom the config in.

#### exmaple config file
```
{
    "run_type": "server",
    "local_addr": "0.0.0.0",
    "local_port": "443",
    "allowed_passwords": [
        "testpsswd"
    ],
    "log_level": "NOTICE",
    "log_dir":"",
    "ssl": {
        "cert": "path_to_cert",
        "key": "path_to_key"
    }
}
```


 ## Client side 
 Overplus fully support trojan protocol, So it is Ok to use trojan's client. Please make sure disable certificate verification if you use a self issued certificate.

