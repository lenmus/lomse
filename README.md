# Lomse: A free open source library for rendering music scores

Lomse objective is provide software developers with a library to add capabilities to any program for rendering, editing and playing back music scores. It is written in C++ and it is free open source and platform independent. Lomse stands for "LenMus Open Music Score Edition Library".

Please be aware that Lomse is a work in progress, not having yet reached version 1.0. This means that the library is operative but with limitations. In particular, support for music scores is currently limited. You are welcome to join this non-commercial project and collaborate in its development. 


## License
Lomse is distributed under the BSD 2-clause license, a permissive open source license to allow Lomse to be used in any projects, whether open source or proprietary. This license is a GPL compatible license, so you can use Lomse in GPL licensed projects.


## Features

* Platform independent
* Can be safely used in commercial closed source projects
* MusicXML import
* Renderization of full documents (paragraphs, tables, lists, scores, images, etc.).
* Support for play back of scores, by generating "midi events" in real time
* Support for play back visual effects (i.e. moving cursor) in sync. with midi events
* Support for music scores edition
* Music renderization is based on fonts compliant with the Standard Music Font Layout (SMuFL) specification


## Installation

~~~~
# download lomse repository
cd <projects-path>
git clone -b master --single-branch https.//github.com/lenmus/lomse.git

# create folder for building and move there
mkdir build
cd build

#build lomse and testlib
cmake -G "Unix Makefiles" ../lomse
make

#run tests
./bin/testlib

#install library
sudo make install
~~~~

For more info, packages, or other options see [this](http://www.lenmus.org/en/lomse/install)



## Contributing

Lomse is a free open source project and contributions are welcome. Get involved in making it better! Any code submission must be under the terms of the BSD 2-clause license. 

Here’s the suggested way for proposing a change to this project:

1. [Fork this project][fork] to your account.
2. [Create a branch][branch] for the change you intend to make.
3. Make your changes to your fork.
4. [Send a pull request][pr] from your fork’s branch to our `master` branch.

Using the web-based interface to make changes is fine too, and will help you
by automatically forking the project and prompting to send a pull request too.

[fork]: https://help.github.com/articles/fork-a-repo/
[branch]: https://help.github.com/articles/creating-and-deleting-branches-within-your-repository
[pr]: https://help.github.com/articles/using-pull-requests/



## More info
* [Lomse homepage](http://www.lenmus.org/en/lomse/intro)
* [Lomse PPA](https://launchpad.net/~lomse/+archive/ubuntu/ppa/+packages)
* [Installation](http://www.lenmus.org/en/lomse/install)
* [Tutorials](http://www.lenmus.org/en/lomse/documents)


