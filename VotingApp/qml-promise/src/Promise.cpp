// -*- mode: C++; tab-width: 2; indent-tabs-mode: nil; coding: unix -*-

#include <QEventLoop>
#include <QQmlEngine>
#include <QDebug>

#include "Promise.hpp"

QQmlEngine* Promise::mEngine = nullptr;

Promise::Promise(QObject *parent) :
QObject(parent),
mState(State::PENDING)
{}

void Promise::setEngine(QQmlEngine* engine)
{
  mEngine = engine;
}

void Promise::then(QJSValue onResolve, QJSValue onReject)
{
 mOnResolve = onResolve;
 mOnReject = onReject;
}

QVariantList Promise::wait()
{
  QEventLoop el;
  QVariantList result;

  auto quitter = [&] (QVariantList results) {
      result = std::move(results);
      el.quit();
  };
  connect(this, &Promise::resolved, quitter);
  connect(this, &Promise::rejected, quitter);

  el.exec();
  return result;
}

void Promise::resolve(const QVariantList &arguments)
{
  if (!isSettled()) {
    mState = State::FULFILLED;
    emit resolved(arguments);
    complete(mOnResolve, arguments);
  }
}

void Promise::reject(const QVariantList &arguments)
{
  if (!isSettled()) {
    mState = State::REJECTED;
    emit rejected(arguments);
    complete(mOnReject, arguments);
  }
}

bool Promise::isSettled() const
{
  return mState == State::PENDING ? false: true;
}


void Promise::complete(QJSValue fn, const QVariantList &arguments)
{
  QJSValueList list;
  for (const QVariant &item : arguments) {
    list << mEngine->toScriptValue(item);
  }

  if (fn.isCallable()) {
    fn.call(list);
  }
}
