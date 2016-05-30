# QML Wrappers

This directory contains classes which wrap Cap'n Proto types in QObjects which expose the wrapped class's functionality
in a QML-friendly way, i.e. by defining properties to read and write the fields

### These classes are implemented using an old design pattern

These classes were designed to store their data in a capnp Message type, and convert that data to something scrutable
to QML on the fly. I have rejected this pattern as overly complex and premature optimization, instead favoring the
pattern of storing the data on these classes normally, as QObject properties stored in Qt-native data types. This data
should be populated from a capnp Reader, or serialized to a capnp Builder on demand rather than maintained in that
format.

As these classes are updated to the new pattern, move them to the DataStructures folder/naming scheme/namespace.
