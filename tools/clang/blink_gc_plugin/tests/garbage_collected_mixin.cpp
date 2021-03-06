// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "garbage_collected_mixin.h"

namespace blink {

void Mixin::Trace(Visitor* visitor)
{
    // Missing: visitor->Trace(m_self);
}

void HeapObject::Trace(Visitor* visitor)
{
    visitor->Trace(m_mix);
    // Missing: Mixin::Trace(visitor);
}

}
