
#include "QQmlIntrospector.h"

#include <QMetaObject>

#ifndef NO_QT_PRIVATE_LIBS
#   include <private/qqmlpropertycache_p.h>
#endif

QQmlIntrospector::QQmlIntrospector (QObject * parent) : QObject (parent) { }

QObject * QQmlIntrospector::qmlSingletonProvider (QQmlEngine * qmlEngine, QJSEngine * jsEngine) {
    Q_UNUSED (qmlEngine)
    Q_UNUSED (jsEngine)
    return new QQmlIntrospector;
}

bool QQmlIntrospector::inherits (QObject * object, QObject * reference) {
    bool ret = false;
    if (object != Q_NULLPTR && reference != Q_NULLPTR) {
        const QString objectClass    = QString::fromLatin1 (object->metaObject ()->className ());
        const QString referenceClass = QString::fromLatin1 (reference->metaObject ()->className ());
        ret = (objectClass == referenceClass);
        if (!ret) {
            ret = object->inherits (reference->metaObject ()->className ());
#ifndef NO_QT_PRIVATE_LIBS
            if (!ret) {
                const QQmlMetaObject qmlMetaObj (object);
                const QQmlMetaObject qmlMetaRef (reference);
                ret = QQmlMetaObject::canConvert (qmlMetaObj, qmlMetaRef);
            }
#endif
        }
    }
    return ret;
}

QQuickWindow * QQmlIntrospector::window (QQuickItem * item) {
    return (item != Q_NULLPTR ? item->window () : Q_NULLPTR);
}
