# krunner-symbols

A little krunner plugin to retrieve unicode symbols from their description.

E.g. you can type `lambda` and krunner will yield the proper unicode symbol λ. Which descriptions / symbols are supported depends on an open-ended text file `krunner-symbols` which must be placed inside the `~/.config` folder.

As I do not have any experience with Plasma development in general, I took the skeleton of naraesk's project [krunner-translator](https://github.com/naraesk/krunner-translator) to get the general idea of how a krunner plugin can look like.

At this very early stage of development, there are no "official" installation instructions, though you can look through the code and follow the steps described in the `INSTALL` file.
