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
- [Qt](https://qt.io) 5.7+ including QBS
- [Botan](https://botan.randombit.net] 1.11.30+
- Python 2 with PyYAML (to fetch icons)
- [Graphene](https://github.com/followmyvote/graphene) (Optional)
 - If building with graphene, be sure to set the `GRAPHENE_PATH` environment variable to the path graphene is installed to

These video tutorials show how to build and test this project:
 - [Graphene blockchain testnet setup](https://youtu.be/7ETrFkZ9LM0)
 - [Stake-weighted voting system setup](https://youtu.be/IkXC0-Mp3vg)

Alternatively, for testing/developing only the app, an all-in-one testnet Docker image is available [here](https://hub.docker.com/r/followmyvote/docker-testnet/). This may be used simply by starting the testnet container, following the [voting system setup instructions](https://youtu.be/IkXC0-Mp3vg) from the beginning to 3:15 (you will not need the GrapheneBackend, as this is in the docker image), then building by clicking the hammer button in the lower left corner of Qt Creator. While that builds, follow the instructions from 7:35 to 10:05 to set up the wallet (the private key to paste in is `5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3`). Finally, pick up the instructions at 15:00 and follow to the end of the video.

Once the dependencies are installed, building the application is as simple as running `qbs` in the top-level directory. If this doesn't work, probably qbs is not configured yet or Cap'n Proto is not in PATH. If you use the QtCreator IDE, you can skip configuring qbs manually as QtCreator does this automatically. QtCreator also allows you to customize PATH (and other environment variables) in the Projects pane.

To configure qbs, make sure your compiler is in PATH and run `qbs-setup-toolchains --detect`. This should configure your toolchain(s) automatically. Next, point it at Qt. If Qt 5.5 is in PATH, you may simply `qbs-setup-qt --detect`; otherwise, use `qbs-setup-qt /path/to/qt5.5/bin/qmake qt5`. It may also be necessary to run `qbs-config profiles.qt5.baseProfile gcc` to direct qbs to use `gcc` when using the qt5 profile, and/or run `qbs-config defaultProfile qt5` to direct qbs to use the qt5 profile by default.

Binaries will be placed in a build folder created by qbs, probably named `qt5-debug/install-root` or similar, in the top level directory.

### Troubleshooting:
If you get any errors like "import qbs.Environment not found", make sure you're using Qt Creator 4

On Mac, it may be necessary to update the PATH variable if CapnProto's binaries are not in PATH. In Qt Creator, you can update this from the Projects view, available in the left nagivation panel. If you've installed CapnProto through Homebrew, you will need to add /usr/local/bin to PATH.

Also on Mac, if Graphene is linked against OpenSSL from Homebrew, linking GrapheneBackend may fail unless the `LIBRARY_PATH` environment variable is set to `/usr/local/lib`.

### Running:
You may use qbs to run the app:

	qbs run -p VotingApp
	
It's recommended to run the VotingApp through qbs, as qbs will automatically configure the environment so that the app can find the chain adaptor library.
