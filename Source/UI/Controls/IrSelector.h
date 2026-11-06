#pragma once
#include "DeliberateSelector.h"

namespace m3::ui::controls
{
// IR selection is deliberate: wheel movement must not reassign a response.
// Invocation capture stays shared; the binding owns target/session validation.
class IrSelector final:public DeliberateSelector {};
}
