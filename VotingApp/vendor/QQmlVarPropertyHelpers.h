// This file taken from http://gitlab.unique-conception.org/qt-qml-tricks/qt-supermacros/raw/master/QQmlVarPropertyHelpers.h

#ifndef QQMLVARPROPERTYHELPERS
#define QQMLVARPROPERTYHELPERS

#include <QObject>

#define QML_WRITABLE_VAR_PROPERTY(type, name) \
    protected: \
        Q_PROPERTY (type name READ get_##name WRITE set_##name NOTIFY name##Changed) \
    private: \
        type m_##name; \
    public: \
        type get_##name (void) const { \
            return m_##name ; \
        } \
    public Q_SLOTS: \
        bool set_##name (type name) { \
            bool ret = false; \
            if ((ret = m_##name != name)) { \
                m_##name = name; \
                emit name##Changed (m_##name); \
            } \
            return ret; \
        } \
    Q_SIGNALS: \
        void name##Changed (type name); \
    private:

#define QML_READONLY_VAR_PROPERTY(type, name) \
    protected: \
        Q_PROPERTY (type name READ get_##name NOTIFY name##Changed) \
    private: \
        type m_##name; \
    public: \
        type get_##name (void) const { \
            return m_##name ; \
        } \
        bool update_##name (type name) { \
            bool ret = false; \
            if ((ret = m_##name != name)) { \
                m_##name = name; \
                emit name##Changed (m_##name); \
            } \
            return ret; \
        } \
    Q_SIGNALS: \
        void name##Changed (type name); \
    private:

#define QML_CONSTANT_VAR_PROPERTY(type, name) \
    protected: \
        Q_PROPERTY (type name READ get_##name CONSTANT) \
    private: \
        type m_##name; \
    public: \
        type get_##name (void) const { \
            return m_##name ; \
        } \
    private:

class _QmlVarProperty_ : public QObject {
    Q_OBJECT
    QML_WRITABLE_VAR_PROPERTY (int,     var1)
    QML_READONLY_VAR_PROPERTY (bool,    var2)
    QML_CONSTANT_VAR_PROPERTY (QString, var3)
};

#endif // QQMLVARPROPERTYHELPERS
