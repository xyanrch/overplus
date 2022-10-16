# overplus
Overplus is another implement of trojan protocol with better perfermance and stability.

[![Azure DevOps Status](https://dev.azure.com/overplusProxy/overplus/_apis/build/status/xyanrch.overplus?branchName=master)](https://dev.azure.com/overplusProxy/overplus/_build/latest?definitionId=3&branchName=master)

**Compared with trojan**:
- [x] Enhanced Security
- [x] It doesn't need to set up a http server on port 80 in your linux server.
- [x] It doesn't need to apply a domain name.
- [x] More readable and cleaner code.




## One-click deployment
Run the script and follow the assistant:

``` commandline 
curl -O https://raw.githubusercontent.com/xyanrch/overplus/master/install.sh && chmod +x install.sh && sudo ./install.sh
 ```
 
 It is recommended to enable BBR to accelerate the network speed.

## Build
The project depend on boost and openssl libraries, please make sure install boost and openssl before build the project.

### How to build
``` commandline
mkdir build && cd build
cmake ..
make

```
### How to run

``` commandline
sudo ./overplus server.json
```
server.json is a config file which you can customize.

#### Example config file
```json
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
## Windows platform build
Windows use vcpkg to manage library

**Dowload vcpkg**
```commandline
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
```
**Install dependence**
```commandline
.\vcpkg\vcpkg.exe install --triplet x64-windows
```
**Build project**
```commandline
make -B build -S . -DCMAKE_TOOLCHAIN_FILE="\vcpkg\scripts\buildsystems\vcpkg.cmake"
```
## How does overplus work?
Trojan protocol is a socks5 like protocol.Trojan request is formed as follows:

        +----+-----+-------+------+----------+----------+-------+
        |Command | password| DST.ADDR | DST.PORT |packed payload
        +----+-----+-------+------+----------+----------+-------+
        | unit8  |  string  | string |   string  | string
        +----+-----+-------+------+----------+----------+-------+
![flow chart](asset/flow.png)

 ## Client side 
 Overplus fully support trojan protocol, So it is Ok to use trojan's client. Please make sure disable certificate verification if you use a self issued certificate.


## Roadmap
- [ ] Implement a web console to manage overplus
- [ ] Design a new protocol to replace trojan protocol