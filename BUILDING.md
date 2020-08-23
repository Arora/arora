# Step-by-Step Instructions to Run the latest Arora

Arora is still in development, which means that there are no compiled installable packages yet. To try the latest code, follow this guide.

### Step 1: Make sure git is installed.
To see if git is installed, open up a terminal window and type `git`. If it is not installed, the terminal will report back command not found. If it is installed, you'll receive the git help text. To install git: [Windows](https://gitforwindows.org/), [Mac](https://git-scm.com/download/mac), and for Linux, use your distro's package manager.

### Step 2: Download and install Qt 5.14 or greater.
If you already have **Qt 5.14** or greater installed, you can skip this step. To install Qt 5.14, visit the [Download Page](https://www.qt.io/download) or use your distro's package manager on Linux. Be sure to add the correct PATH variable if required in the official install instructions. When you're done, skip the alternative method section, and move onto Step 3.

### Step 3: Download and compile the latest Arora.
Now we will download the latest tree of Arora. Issue these commands:
```
$ git clone git://github.com/AaronDewes/arora.git
$ cd arora
$ mkdir build && cd build
$ cmake ..
$ make -j$(nproc)
```
Arora is now compiled. To run arora, simply run `./arora` from the current directory.

_**Optional**_: To update to the latest Arora in the future, you can issue these commands:
```
$ cd arora
$ git pull
$ cd build
$ make -j$(nproc)
```

If you are looking for an IDE to develop Arora in you can checkout the free IDE Qt Creator that is available with Qt.
