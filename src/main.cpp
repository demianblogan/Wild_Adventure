#include "Application.h"

// On laptops with hybrid graphics (integrated + discrete GPU), Windows lets the
// NVIDIA/AMD driver pick which GPU runs each process from a database of known
// games. A small indie exe not in that database can silently be routed to the
// weak integrated GPU while a discrete one sits idle. Exporting these two
// symbols is the vendor-documented way to force the discrete GPU for this
// process instead. On single-GPU machines it has no effect. DWORD is just
// `unsigned long` here, so this needs no <Windows.h>.
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main()
{
	Application application;
	application.Run();

    return 0;
}