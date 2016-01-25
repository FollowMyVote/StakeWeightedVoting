// This file adapted from http://gitlab.unique-conception.org/qt-qml-tricks/qt-supermacros

#ifndef QQMLENUMCLASS
#define QQMLENUMCLASS

#include <qobjectdefs.h> // for Q_GADGET

#define QML_ENUM_CLASS(name, ...) \
    class name { \
        Q_GADGET \
    public: \
        enum Type { __VA_ARGS__ }; \
        Q_ENUM (Type) \
    };

class _QmlEnumClass_ { Q_GADGET }; // NOTE : to avoid "no suitable class found" MOC note

#endif // QQMLENUMCLASS

