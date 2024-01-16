# PAGER LIBRARY/SUBMODULE

Sorry, it's a pretty lame README for now.

`pager` is a relatively self-contained library that takes control
of the screen to present a scrolling interface to sets of data.

It includes library source files, test source files, a script for
comparing dialog::menu to the interface of `pager` and source code
for a man page.

## DEPENDENCIES

The man page uses a macro file found at
my github project[grofflib](https://www.github.com/cjungmann/grofflib.git).
Download and install.

## BUILD AND INSTALL

~~~sh
git clone git@github.com/cjungmann/lib_pager.git
cd lib_pager
make
sudo make install
make test
~~~

## DOCUMENTATION

The documentation is in the man page (__pager__(3)).  The man page is
installed when the library is installed.

## DESIGN CONSIDERATIONS

This project is a library distilled out of a project to stand
on its own.  It is a design experiment: the separate operational
domains into self-contained libraries

It takes a bit longer than I expected to do this.  Multiple passes
have been needed to pare down the feature list to a point where it
doesn't need anything from another library.

There is a script called `menu` that uses the `dialog` command to
open a `menu` dialog.  I am using its behavior as a model and
justification for how the pager lib behaves.
