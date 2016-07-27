Follow My Vote: Stake-Weighted Voting
========

This is the official repository for Follow My Vote's Stake-Weighted Voting application.

Linux and Mac are both fully supported. Windows support is pending.

More developers are always welcome! [Get started](https://followmyvote.com/code-contributors/)

## What's in the box
In this repo are 4 components:

- The `shared` folder contains code shared between the other components
- The `StubChainAdaptor` folder contains a shared library implementing a dummy blockchain, backend, and wallet
- The `StubBackend` folder contains a dummy server daemon which the application can connect to, though by default it is unused
- The `VotingApp` folder contains the voting application itself

See [Architecture](Architecture.md) for more information on how the components interact.

## Build Instructions
### Dependencies:
- [Cap'n Proto](https://capnproto.org) 0.5.3+
- [Qt](https://qt.io) 5.6+ including QBS
 - Be sure to get Qt Creator 4 from https://www.qt.io/download-open-source/#section-6
 - V-Play still ships with Qt Creator 3, but it's too old to build this project, so use version 4 instead.
- [V-Play](http://v-play.net) 2.7.0+
- [Graphene](https://github.com/cryptonomex/graphene) (Optional)
 - Be sure to use develop branch!
 - If building with graphene, be sure to set the `GRAPHENE_PATH` environment variable to the path graphene is installed to
 
A video showing how to set up the development environment is available [here](https://youtu.be/lvTyZ0GAT-k)

Once the dependencies are installed, building the application is as simple as running `qbs` in the top-level directory. If this doesn't work, probably qbs is not configured yet or Cap'n Proto is not in PATH. If you use the QtCreator IDE, you can skip configuring qbs manually as QtCreator does this automatically. QtCreator also allows you to customize PATH (and other environment variables) in the Projects pane.

To configure qbs, make sure your compiler is in PATH and run `qbs-setup-toolchains --detect`. This should configure your toolchain(s) automatically. Next, point it at Qt. If Qt 5.5 is in PATH, you may simply `qbs-setup-qt --detect`; otherwise, use `qbs-setup-qt /path/to/qt5.5/bin/qmake qt5`. It may also be necessary to run `qbs-config profiles.qt5.baseProfile gcc` to direct qbs to use `gcc` when using the qt5 profile, and/or run `qbs-config defaultProfile qt5` to direct qbs to use the qt5 profile by default.

Binaries will be placed in a build folder created by qbs, probably named `qt5-debug/install-root` or similar, in the top level directory.

### Troubleshooting:
If you get any errors like "import qbs.Environment not found", make sure you're using Qt Creator 4 and not the Qt Creator 3 that comes with V-Play.

On Mac, it may be necessary to update the PATH variable if CapnProto's binaries are not in PATH. In Qt Creator, you can update this from the Projects view, available in the left nagivation panel. If you've installed CapnProto through Homebrew, you will need to add /usr/local/bin to PATH.

Also on Mac, if Graphene is linked against OpenSSL from Homebrew, linking GrapheneBackend may fail unless the `LIBRARY_PATH` environment variable is set to `/usr/local/lib`.

### Running:
You may use qbs to run the app:

	qbs run -p VotingApp
	
It's recommended to run the VotingApp through qbs, as qbs will automatically configure the environment so that the app can find the chain adaptor library.
