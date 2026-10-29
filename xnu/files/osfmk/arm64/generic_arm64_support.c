/*
 * Routines the open-source tree references but only defines for Apple SoCs.
 * Needed when building for a generic ARMv8 board without a monitor (PPL).
 */
#include <mach/mach_types.h>
#include <arm/machine_routines.h>

/* No PPL / secure monitor to switch on. */
void
ml_enable_monitor(void)
{
}
