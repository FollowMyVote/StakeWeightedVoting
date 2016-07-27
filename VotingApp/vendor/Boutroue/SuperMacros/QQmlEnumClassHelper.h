#ifndef QQMLENUMCLASS
#define QQMLENUMCLASS

#include <QObject>

#ifdef Q_ENUM
#   define QML_EXPORT_ENUM Q_ENUM
#else
#   define QML_EXPORT_ENUM Q_ENUMS
#endif

#define QML_ENUM_CLASS(name, ...) \
    class name : public QObject { \
        Q_GADGET \
    public: \
        enum Type { __VA_ARGS__ }; \
        QML_EXPORT_ENUM (Type) \
    };

class _Test_QmlEnumClass_ { Q_GADGET }; // NOTE : to avoid "no suitable class found" MOC note

#endif // QQMLENUMCLASS

