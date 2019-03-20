# krunner-symbols

A little krunner plugin (Plasma 5) to retrieve unicode symbols, or any other string, based on a corresponding keyword.

![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration1.png)

E.g. you can type `lambda` and krunner will yield the proper unicode symbol λ. It can be used to quickly get a symbol when writing some text, or to retrieve often-needed texts (or links, or emotes ...) by typing a certain abbreviation.

![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration2.png)
![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration3.png)

If desired, the plugin supports to search the full Unicode database. It is also possible to let the plugin open some file or URL or to execute some command when selecting a result. 

![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration4.png)
![Demonstration](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration5.png)

The definitions for all these use cases can be freely configured (see *Configuration*).

## Installation

Quick Install:

```
git clone https://github.com/domschrei/krunner-symbols.git; cd krunner-symbols; bash install.sh
```

This executes the plugin's install script inside the `krunner-symbols` directory. You will be prompted for `sudo` after a successful build because some files need to be written into `/usr`. Some dependencies are required, see below for your distribution. If your distribution is not listed and you get an error, the output from `make` will tell you which package is missing.

### Arch Linux 

Arch Linux users can just install the `plasma5-runners-symbols` [package from AUR](https://aur.archlinux.org/packages/plasma5-runners-symbols/ "link to AUR package"), which is based on the most recent release here on Github.

### Debian / Ubuntu

Installing the following dependencies should suffice for Debian and Ubuntu installations with the KDE Plasma desktop:

``` 
sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext
```

### openSUSE

For openSUSE Tumbleweed, these dependencies are sufficient:

```
sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools
```

## Usage

Open krunner and enter the keyword for what you want to copy. Click on the desired entry tagged with *Symbols* or press Enter while focusing it (navigating with arrow keys). krunner disappears and the result has been copied to your clipboard. With `Ctrl+V`, you can paste it anywhere.

You can use the plugin as some more general form of alias-resolver (i.e. entering some keyword in order to get the corresponding text) and as a general shortcut app by editing the configuration file appropriately.

## Configuration

**TL;DR:** Look at the file `krunner-symbolsrc` and you will get the idea. Store custom stuff in `~/.config/krunner-symbolsrc`. Don't forget to activate full Unicode support if you want it!

Additional to the primary source `/usr/share/config/krunner-symbolsrc`, you can create a second file `~/.config/krunner-symbolsrc` for custom, additional definitions. Those definitions will override those inside the global config, so that you can give a new meaning to globally defined symbols, if you wish. Keep in mind that you will have to restart krunner (`kquitapp krunner && krunner`) for configuration changes to take effect.

The syntax is easy (you can just copy from the file `krunner-symbolsrc` from the downloaded git directory). Define custom symbols below a config group called "Definitions":
```
[Definitions]
```
For custom definitions, just create lines like this:
```
Keyword=Output
```
, where `Keyword` is what you have to type and `Output` is what you get when typing it. They can be entire sentences, or just single symbols. Comments with `#` and blank lines are ignored.

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

The plugin is not only able to copy strings to your clipboard, but also to open URLs and documents (with `open:`) and execute commands (with  `exec:`). Here are some examples:

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

If selected, the plugin will try to open or execute the specified location or command, using the corresponding default application. Using these types of definitions is obviously at your own risk, as harmful commands might be executed if the config file is set up accordingly.

### Unicode Support

This plugin now supports the Unicode database, i.e. [all symbols inside the official UnicodeData.txt](http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt) can be retrieved by entering (parts of) the corresponding description for the symbol. At the moment this feature is disabled by default because there is some performance overhead of searching all the unicode domain, and you can enable it by setting `UseUnicodeDatabase` to `true` inside the `[Preferences]` group of your config file (and then restarting `krunner`):

```
[Preferences]
UseUnicodeDatabase=true
```

This very large database also contains a lot of emojis and pictograms (try `thumbs up`, `cat face` or `snowman`).

## Remarks

krunner is a great tool because it's always accessible, very fast, and really unintrusive, as it temporarily gets focused when called but then returns the focus to the previously focused view when finished. Because of this, a quick krunner query can be perfectly integrated into a user's workflow, adding efficiency and convenience. This little tool is meant to support this idea of usage.

If you are interested in the project, have any recommendations or suggestions, feel free to contact me or post an issue.
