// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>
#include <windowsX.h>

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <commctrl.h>
#include <commdlg.h>
#include <math.h>
#include <mmsystem.h>

#include "resource.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

// TODO: プログラムに必要な追加ヘッダーをここで参照してください
