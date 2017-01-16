# krunner-symbols

A little krunner plugin (Plasma 5) to retrieve unicode symbols, or any other string, based on a corresponding keyword.

![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration1.png)

E.g. you can type `lambda` and krunner will yield the proper unicode symbol λ. It can be used to quickly get a symbol when writing some text, or to retrieve often-needed texts (or links, or emotes ...) by typing a certain abbreviation.

![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration2.png)
![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration3.png)

Which keywords and symbols are supported depends on an open-ended text file (see *Configuration*).

## Why?

krunner is a great tool because it's always accessible, very fast, and really unintrusive, as it temporarily gets focused when called, but then returns the focus to the previously focused view when finished. Because of this, a quick krunner query can be perfectly integrated into a user's workflow, adding efficiency and convenience. This little tool is meant to support this idea of usage.

If you are interested in the project, have any recommendations or suggestions, feel free to contact me or post an issue.

## Installation

You can execute the install script (inside the `krunner-symbols` directory, execute the command `bash install.sh`) in order to install the plugin. Some packages are required, especially `cmake` and `extra-cmake-modules`. Additionally, some packages from the KDE and Qt frameworks are needed, which may be already installed on your system.


### Arch Linux 

Arch Linux users can install the `plasma5-runners-symbols` [package from AUR](https://aur.archlinux.org/packages/plasma5-runners-symbols/ "link to AUR package"), which is based on the most recent release here on Github.

### openSuse

For openSUSE (Tumbleweed), installing these packages is neccessary:

```
sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools
```

### Ubuntu

Installing the following packages works on Kubuntu 16.04.1 LTS:

``` 
sudo apt-get install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev
```


## Configuration

**TL;DR:** Look at the file `krunner-symbolsrc` and you will get the idea. Store custom stuff in `~/.config/krunner-symbolsrc`.

Additional to the primary source `/usr/share/config/krunner-symbolsrc`, you can create a second file `~/.config/krunner-symbolsrc` where you can create custom, additional definitions. Those definitions will dominate those inside the global config, so that you can give a new meaning to globally defined symbols, if you wish. Keep in mind that you will have to restart krunner (`kquitapp krunner && krunner`) for configuration changes to take effect.

The syntax is easy. Define custom symbols below a config group called "Definitions":
```
[Definitions]
```
For custom definitions, just create lines like this:
```
Keyword=Output
```
, where `Keyword` is what you have to type and `Output` is what you get when typing it. They can be entire sentences, or just single symbols. To insert a comment, you can do this: 

```
# Lalala the program can't hear me
```

Blank lines are okay, too.

### Unicode Support

This plugin now supports the Unicode database (i.e. [all symbols inside the official UnicodeData.txt](http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt) can be retrieved by entering the corresponding description for the symbol). At the moment this feature is disabled by default because there is a significant performance overhead of searching all the unicode domain, though you can enable it by setting `UseUnicodeDatabase` to `true` inside the `[Preferences]` group of your config file:

```
[Preferences]
UseUnicodeDatabase=true
```

### Advanced

Multiple keywords for the same result can be expressed like this:
```
ampersand,fancyand,curlyand=&
```
which will internally be expanded to this:
```
ampersand=&
fancyand=&
curlyand=&
```

If you feel like doing just about *everything* with `krunner`, you can even add definitions like this:

```
# Open the KDE homepage in the standard browser
kde=open:https://www.kde.org/

# Open this plugin's configuration in the standard text editor
config=open:~/.config/krunner-symbolsrc

# Open the folder Documents in the standard file browser
doc=open:~/Documents

# Execute the given command (here: restart krunner)
kill=exec:killall krunner && krunner
```

If selected, these types of definitions (beginning with `open:` or `exec:`) will not copy the result to the clipboard, but actually try to open or execute the specified location or command, using the corresponding standard application. Using these types of definitions is obviously at your own risk, as harmful commands might be executed if the config file is set up accordingly.

## Usage

Open krunner and enter the keyword for the symbol you want to copy. Click on the appearing entry tagged with *Symbols* or press Enter while focusing it. krunner disappears and the result has been copied to your clipboard. With `Ctrl+V`, you can paste it anywhere.

You can also use the plugin as some more general form of alias-resolver (i.e. entering some keyword in order to get the corresponding text) and a general shortcut app by editing the configuration file appropriately.
