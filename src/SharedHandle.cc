/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "SharedHandle.h"

namespace aria2 {

SharedCount::SharedCount():refCount_(new RefCount()) {}

SharedCount::SharedCount(const SharedCount& s):refCount_(s.refCount_)
{
  refCount_->addRefCount();
}

SharedCount::~SharedCount()
{
  refCount_->releaseRefCount();
  if(refCount_->getWeakRefCount() == 0) {
    delete refCount_;
  }
}

bool SharedCount::reassign(const SharedCount& s)
{
  s.refCount_->addRefCount();
  refCount_->releaseRefCount();
  if(refCount_->getWeakRefCount() == 0) {
    delete refCount_;
    refCount_ = s.refCount_;
    return true;
  }
  size_t thisCount = refCount_->getStrongRefCount();
  refCount_ = s.refCount_;
  return thisCount == 0;
}

void SharedCount::swap(SharedCount& r)
{
  std::swap(refCount_, r.refCount_);
}

WeakCount::WeakCount(const WeakRef& t):refCount_(new RefCount(t)) {}

WeakCount::WeakCount(const StrongRef& t):refCount_(new RefCount()) {}

WeakCount::WeakCount(const WeakCount& w):refCount_(w.refCount_)
{
  refCount_->addWeakRefCount();
}

WeakCount::WeakCount(const SharedCount& s):refCount_(s.refCount_)
{
  refCount_->addWeakRefCount();
}

WeakCount::~WeakCount()
{
  refCount_->releaseWeakRefCount();
  if(refCount_->getWeakRefCount() == 0) {
    delete refCount_;
  }
}

bool WeakCount::reassign(const SharedCount& s)
{
  s.refCount_->addWeakRefCount();
  refCount_->releaseWeakRefCount();
  if(refCount_->getWeakRefCount() == 0) {
    delete refCount_;
    refCount_ = s.refCount_;
    return true;
  }
  refCount_ = s.refCount_;
  return false;
}

bool WeakCount::reassign(const WeakCount& s)
{
  s.refCount_->addWeakRefCount();
  refCount_->releaseWeakRefCount();
  if(refCount_->getWeakRefCount() == 0) {
    delete refCount_;
    refCount_ = s.refCount_;
    return true;
  }
  refCount_ = s.refCount_;
  return false;
}

} // namespace aria2
