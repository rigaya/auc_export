#include <Windows.h>
#include <stdio.h>
#pragma warning( push )
#pragma warning( disable: 4706 )
#include "..\auc.h"
#pragma warning( pop )

static char *USAGE = "auc_export.exe [#window] #plugin <filename>";

//lsmashmuxerのダイアログは親が設定されない
//タイトル名とプロセスIDから特定する
static HWND getdlghwnd_muxer(HWND hwnd) {
	HWND hwnd_result = NULL;
	DWORD pid_main_window = 0;
	GetWindowThreadProcessId(hwnd, &pid_main_window);

	Sleep(100);
	for (int i = 0; i < 20; i++) {
		HWND hwndd = FindWindowEx(NULL, NULL, NULL, "lsmashmuxer");
		if (NULL != hwndd) {
			DWORD pid_window = 0;
			GetWindowThreadProcessId(hwndd, &pid_window);
			if (pid_main_window == pid_window) {
				hwnd_result = hwndd;
				break;
			}
		}
		Sleep(100);
	}
	if (NULL == hwnd_result)
		error_exit("could not get dialog window.");

	Sleep(100);

	return hwnd_result;
}

int main(int argc, char **argv) {
	if (argc < 3 || argc > 4) error_exit(USAGE);
	if (argv[argc - 2][0] < '0' || argv[argc - 2][0] > '9') error_exit(USAGE);

	int num = atoi(argv[argc - 2]);

	HWND hwnd = (argc == 4) ? gethwnd_str(argv[1]) : gethwnd();

	//エクスポートプラグインをクリック
	PostMessage(hwnd, WM_COMMAND, 15031 + num, 0);

	//L-SMASH Works Muxerのダイアログを探してみる
	HWND hwnd_muxer_dlg = getdlghwnd_muxer(hwnd);
	if (NULL != hwnd_muxer_dlg) {
		//Muxボタンを押す
		HWND hwnd_mux_button = FindWindowEx(hwnd_muxer_dlg, NULL, "Button", "Mux");
		PostMessage(hwnd_mux_button, WM_LBUTTONDOWN, MK_LBUTTON, 0);
		PostMessage(hwnd_mux_button, WM_LBUTTONUP, MK_LBUTTON, 0);
		Sleep(100);
		if (IsWindow(hwnd_muxer_dlg) && GetFocus() == hwnd_mux_button) {
			char buf[1024] = { 0 };
			GetWindowText(hwnd_muxer_dlg, buf, _countof(buf));
			if (0 == strcmp(buf, "lsmashmuxer")) {
				PostMessage(hwnd_mux_button, WM_LBUTTONDOWN, MK_LBUTTON, 0);
				PostMessage(hwnd_mux_button, WM_LBUTTONUP, MK_LBUTTON, 0);
			}
		}
	}
	//保存ダイアログを探す
	HWND hwnd_savefile_dlg = getdlghwnd(hwnd);

	//あとはaji様と同じように
	HWND hwnd_save_button = FindWindowEx(hwnd_savefile_dlg, NULL, "Button", "保存(&S)");
	HWND hwnd_comboboxex = FindWindowEx(hwnd_savefile_dlg, NULL, "ComboBoxEx32", NULL);
	if (hwnd_comboboxex == NULL) {
		hwnd_comboboxex = FindWindowEx(hwnd_savefile_dlg, NULL, "DUIViewWndClassName", NULL);
		if (hwnd_comboboxex) hwnd_comboboxex = FindWindowEx(hwnd_comboboxex, NULL, "DirectUIHWND", NULL);
		if (hwnd_comboboxex) hwnd_comboboxex = FindWindowEx(hwnd_comboboxex, NULL, "FloatNotifySink", NULL);
	}
	if (hwnd_save_button == NULL || hwnd_comboboxex == NULL) error_exit("Unexpected dialog.");
	HWND hwnd_combobox  = FindWindowEx(hwnd_comboboxex, NULL, "ComboBox", NULL);
	HWND hwnd_save_filename = FindWindowEx(hwnd_combobox, NULL, "Edit", NULL);
	DeleteFile(argv[argc - 1]);
	SendMessage(hwnd_save_filename, WM_SETTEXT, 0, (LPARAM)argv[argc - 1]);
	SendMessage(hwnd_save_button, WM_LBUTTONDOWN, 0, 0);
	SendMessage(hwnd_save_button, WM_LBUTTONUP, 0, 0);

	return 0;
}
