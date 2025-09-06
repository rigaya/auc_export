#include <stdio.h>
#include <windows.h>
#pragma warning( push )
#pragma warning( disable: 4706 )
#include "..\auc.h"
#pragma warning( pop )

static char *usage = "usage: auc_wait_export [#window]";

static DWORD cpu_core_count() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

UINT64 get_elapsed_time_100ns(FILETIME fin, FILETIME start) {
	return (((UINT64)fin.dwHighDateTime << 32) + fin.dwLowDateTime) - (((UINT64)start.dwHighDateTime << 32) + start.dwLowDateTime);
}

double get_process_cpu_usage(DWORD target_process_id, DWORD measure_time_ms) {
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time_start;
	FILETIME kernel_time_fin;
	FILETIME user_time_start;
	FILETIME user_time_fin;

	HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, target_process_id);

	if (!GetProcessTimes(h_process, &creation_time, &exit_time, &kernel_time_start, &user_time_start))
		return 0;

	Sleep(measure_time_ms);
	
	if (!GetProcessTimes(h_process, &creation_time, &exit_time, &kernel_time_fin, &user_time_fin))
		return 0;

	CloseHandle(h_process);

	UINT64 elapsed_time_100ns = get_elapsed_time_100ns(kernel_time_fin, kernel_time_start) + get_elapsed_time_100ns(user_time_fin, user_time_start);

	return elapsed_time_100ns * 0.0001 / (double)measure_time_ms / cpu_core_count();
}

int main(int argc, char **argv) {
	if (argc < 1 || argc > 2) error_exit(usage);

	HWND hwnd = (argc == 2) ? gethwnd_str(argv[1]) : gethwnd();

	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

	DWORD pid_aviutl = 0;
	GetWindowThreadProcessId(hwnd, &pid_aviutl);

	for (int i = 0; i < 7; i++) {
		if (0.0001 < get_process_cpu_usage(pid_aviutl, 500)) {
			Sleep(5000);
			i = -1;
		} else {
			Sleep(500);
		}
	}

	return 0;
}
