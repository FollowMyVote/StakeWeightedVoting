# QML Data Structures

This directory contains classes which implement data structures readable by QML. These structures may or may not be
serializable to capnp structs, but if they are, they should provide method(s) to read from an appropriate capnp Reader
and update the properties accordingly and/or write out the properties to an appropriate capnp Builder.

They shall not hold Readers or Builders and convert data to/from them on-the-fly, nor may they inherit from a
Reader/Builder. Some older classes worked this way, but I've decided that design pattern is overly complex and is
premature optimization.

A data structure goes in the swv::data namespace. Naming is simple; if the structure represents a widget, the class
should be named swv::data::Widget. If the structure serializes to/from a capnp struct, it should have the same name as
that struct, i.e. swv::data::Coin serializes to/from ::Coin::Reader/Builder. The files are named as Widget.{c,h}pp.
