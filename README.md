Follow My Vote: Stake-Weighted Voting
========

This is the official repository for Follow My Vote's Stake-Weighted Voting application.

Linux and Mac are both fully supported. Windows support is pending.

## What's in the box
In this repo are 4 components:

- The `shared` folder contains code shared between the other components
- The `StubChainAdaptor` folder contains a shared library implementing a dummy blockchain, used by the application
- The `StubBackend` folder contains a dummy server daemon which the application will connect to. This daemon should be running before launching the application
- The `VotingApp` folder contains the voting application itself

See [Architecture](Architecture.md) for more information on how the components interact.

## Build Instructions
### Dependencies:
- [Cap'n Proto](https://capnproto.org) 0.5.3+
- [Qt](https://qt.io) 5.6+ including QBS
- [V-Play](http://v-play.net) 2.7.0+
 
You will also need to fetch the submodule dependencies by running `git submodule update --init`

Once the dependencies are installed, building the application is as simple as running `qbs` in the top-level directory. If this doesn't work, probably qbs is not configured yet or Cap'n Proto is not in PATH. If you use the QtCreator IDE, you can skip configuring qbs manually as QtCreator does this automatically. QtCreator also allows you to customize PATH (and other environment variables) in the Projects pane.

To configure qbs, make sure your compiler is in PATH and run `qbs-setup-toolchains --detect`. This should configure your toolchain(s) automatically. Next, point it at Qt. If Qt 5.5 is in PATH, you may simply `qbs-setup-qt --detect`; otherwise, use `qbs-setup-qt /path/to/qt5.5/bin/qmake qt5`. It may also be necessary to run `qbs-config profiles.qt5.baseProfile gcc` to direct qbs to use `gcc` when using the qt5 profile, and/or run `qbs-config defaultProfile qt5` to direct qbs to use the qt5 profile by default.

Binaries will be placed in a build folder created by qbs, probably named `qt5-debug/install-root` or similar, in the top level directory.

### Running:
qbs only runs one binary at a time, so run the backend separately:

	./qt5-debug/install-root/bin/StubBackend
	
You may then use qbs to run the app itself:

	qbs run -p VotingApp
	
It's recommended to run the VotingApp through qbs, as qbs will automatically configure the environment so that the app can find the chain adaptor library.
