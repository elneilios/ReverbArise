#pragma once
#include <UI/WorkspaceTheme.h>
namespace m3::ui::controls
{
// Selection commits are deliberate; wheels retain their surrounding scroll role.
// Invocation capture and domain validation remain with the binding.
class DeliberateSelector:public InvocationComboBox
{
public:DeliberateSelector(){setScrollWheelEnabled(false);}
};
}
