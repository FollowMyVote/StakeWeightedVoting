# QML APIs

This directory contains classes which implement various APIs for QML. These APIs generally wrap a capnp interface, but
this is not necessarily the case. APIs may be stateless, simply passing calls and results back and forth; or they may
be stateful and hold data between calls. Some APIs expose no calls to QML directly, but instead maintain data from the
server as a QObject property with update notification signals.

Unlike Data Structures, APIs may hold capnp objects like Clients, Readers and Builders. APIs are generally constructed
around a capnp interface Client. In many cases, the API will be the only object holding that Client, thus when the API
is destroyed, the server-side object will be freed as well.

APIs are placed in the swv namespace. All API names end in Api, and their files will be named FooApi.{c,h}pp.
