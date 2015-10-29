# QML Wrappers

This directory contains classes which wrap Cap'n Proto types in QObjects which expose the wrapped class's functionality
in a QML-friendly way, i.e. by defining properties to read and write the fields

There are two types of wrappers: read-only wrappers and read-write wrappers. Read-only wrappers inherit the wrapped
type and define properties and accessors for the data. Read-write wrappers can also modify the data, and take a
a WrappedType::Builder in their constructors, thus they do not own the data they manipulate (so that data can belong to
a message being filled prior to transmission). If a read-write wrapper which owns its data is desired, OwningWrapper is
a template which can be applied to the read-write wrappers which cause it to own the underlying data. OwningWrapper
also provides serialization functionality.
