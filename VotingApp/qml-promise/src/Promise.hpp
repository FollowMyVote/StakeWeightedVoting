// -*- mode: C++; tab-width: 2; indent-tabs-mode: nil; coding: unix -*-

#pragma once

#include <initializer_list>
#include <QJSValue>
#include <QObject>
#include <QVariant>

class QQmlEngine;

class Promise : public QObject
{
  Q_OBJECT
public:
  enum class State {
    PENDING,
    FULFILLED,
    REJECTED
  };

  explicit Promise(QObject *parent = 0);
  virtual ~Promise() {}

  static void setEngine(QQmlEngine* engine);

  Q_INVOKABLE void then(QJSValue onResolve, QJSValue onReject = QJSValue());
  Q_INVOKABLE QVariantList wait();

  void resolve(const QVariantList& arguments);
  void reject(const QVariantList& arguments);

  State state() const {
      return mState;
  }

signals:
  void resolved(QVariantList arguments);
  void rejected(QVariantList arguments);

protected:
  void complete(QJSValue fn, const QVariantList& arguments);
  bool isSettled() const;
private:
  static QQmlEngine* mEngine;
  State mState;
  QJSValue mOnResolve;
  QJSValue mOnReject;
};
