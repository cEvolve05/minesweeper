#pragma once
#include "base.h"

template <typename SlintWidgetState>
class StateAccessor {
    slint::ComponentHandle<UiEntry> uiEntry;

  public:
    StateAccessor(slint::ComponentHandle<UiEntry>& uiEntry) : uiEntry(uiEntry) {}
    const SlintWidgetState* operator->() { return &uiEntry->global<SlintWidgetState>(); }
};