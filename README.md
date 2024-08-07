
# krunner-symbols

<hr/>

**Note:** This is the Plasma 5 version of the plugin. You can find a Plasma 6 port [here](https://github.com/domschrei/krunner-symbols/tree/plasma6), which will replace this version when Plasma 6 becomes more widespread across distributions.

<hr/>

A little krunner plugin (Plasma 5) to retrieve unicode symbols, or any other string, based on a corresponding keyword.

![Demonstration "lambda"](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration1.png)

E.g. you can type `lambda` and krunner will yield the proper unicode symbol λ. It can be used to quickly get a symbol when writing some text, or to retrieve often-needed texts (or links, or emotes ...) by typing a certain abbreviation.

![Demonstration "loremipsum"](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration2.png)
![Demonstration "fliptable"](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration3.png)

By default, the plugin searches the full Unicode database for potential matches. It is also possible to let the plugin open some file or URL or to execute some command when selecting a result. 

![Demonstration "kde"](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration4.png)
![Demonstration "snooze"](https://raw.githubusercontent.com/domschrei/krunner-symbols/master/img/demonstration5.png)

The definitions for all these use cases can be freely configured (see *Configuration*).


## Installation

There are two ways to install the plugin in packaged form (i.e., without building it yourself).

### Debian-based

Use the latest .deb package:

`wget https://github.com/domschrei/krunner-symbols/releases/download/1.1.0/krunner-symbols-1.1.0.deb && sudo dpkg -i krunner-symbols-1.1.0.deb`

### Generic Linux

Use this script (which fetches the latest release package files):

`curl https://raw.githubusercontent.com/domschrei/krunner-symbols/master/install.sh | bash`

### Uninstallation

For uninstalling the plugin, use `apt` if you installed via .deb:

`sudo apt remove krunner-symbols`

and use the script `uninstall.sh` if you installed via the generic script.


## Building

**Quick build and install** after installing the required depencencies (see below):

```
git clone https://github.com/domschrei/krunner-symbols.git; cd krunner-symbols; bash build_and_install.sh
```

This executes the plugin's install script inside the `krunner-symbols` directory. Some dependencies are required, see below for your distribution. If your distribution is not listed and you get an error, the output from `cmake` will give an indication for which package is missing.

**Arch Linux**

`sudo pacman -S ki18n krunner qt5-base cmake extra-cmake-modules`

_Note that there is a `plasma5-runners-symbols` [package from AUR](https://aur.archlinux.org/packages/plasma5-runners-symbols/ "link to AUR package"), but it is unfortunately not kept up to date._

**Debian / Ubuntu**

`sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext`

**openSUSE**

`sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools`

**Fedora**

`sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext qt5-qtdeclarative-devel`


## Usage

Open krunner and enter the keyword for what you want to copy. Click on the desired entry tagged with *Symbols* or press Enter while focusing it (navigating with arrow keys). krunner disappears and the result has been copied to your clipboard. With `Ctrl+V`, you can paste it anywhere.

Use double quotation marks to enclose a string with empty spaces that you want to match exactly. Otherwise, inexact matches will be shown as well based on a simple heuristic.

You can use the plugin as some more general form of alias-resolver (i.e. entering some keyword in order to get the corresponding text) and as a general shortcut app by editing the configuration file appropriately.


## Configuration

**TL;DR:** Look at the file `krunner-symbolsrc` and you will get the idea. Store custom stuff in `~/.config/krunner-symbolsrc`.

Additional to the primary source `/usr/share/config/krunner-symbolsrc`, you can create a second file `~/.config/krunner-symbolsrc` for custom, additional definitions. Those definitions will override those inside the global config, so that you can give a new meaning to globally defined symbols, if you wish. Keep in mind that you will have to restart krunner (`bash restart-krunner.sh`) for configuration changes to take effect.

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

This plugin supports the Unicode database, i.e. [all symbols inside the official UnicodeData.txt](http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt) can be retrieved by entering (parts of) the corresponding description for the symbol. This feature is enabled by default, however there is some performance overhead of searching all the unicode domain that may be noticeable on weaker systems. You can disable it by setting `UseUnicodeDatabase` to `false` inside the `[Preferences]` group of your config file (and then restarting `krunner`):

```
[Preferences]
UseUnicodeDatabase=false
```

This large database also contains a lot of emojis and pictograms (try `thumbs up`, `cat face` or `snowman`).


## Remarks

krunner is a great tool because it's always accessible, very fast, and really unintrusive, as it temporarily gets focused when called but then returns the focus to the previously focused view when finished. Because of this, a quick krunner query can be perfectly integrated into a user's workflow, adding efficiency and convenience. This little tool is meant to support this idea of usage.

If you are interested in the project, have any recommendations or suggestions, feel free to contact me or post an issue.
