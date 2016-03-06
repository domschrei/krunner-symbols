# krunner-symbols

A little krunner plugin to retrieve unicode symbols from their description.

![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/demonstration.png)

E.g. you can type `lambda` and krunner will yield the proper unicode symbol λ. 

Which descriptions / symbols are supported depends on an open-ended text file `krunner-symbols` which must be placed inside the `~/.config` folder.

As I do not have any experience with Plasma development in general, I took the skeleton of naraesk's project [krunner-translator](https://github.com/naraesk/krunner-translator) to get the general idea of how a krunner plugin can look like. If you are interested in the project, have any recommendations or suggestions, feel free to contact me or post an issue.

## Installation

You can execute the following commands (which are also described in the `INSTALL` file) in order to install the plugin. However, please note that this is in an early stage of development, which is why you have to copy the config file yourself (see 3rd last line).
```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins`
make 
sudo make install
cp ../krunner-symbols ~/.config/krunner-symbols
kquitapp krunner
krunner
```

## Configuration

By editing `~/.config/krunner-symbols`, you can change and/or extend the recognized symbols. You can also use the plugin as some more general form of alias-resolver (i.e. entering some keyword in order to get the corresponding text).
