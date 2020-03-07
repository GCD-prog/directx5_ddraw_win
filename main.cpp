#include <windows.h>
#include <ddraw.h>

LPDIRECTDRAWSURFACE lpPrimary;   //基本サーフェス（画面表示と同じもの）
LPDIRECTDRAWSURFACE lpBackbuffer;  //バックバッファサーフェス（描画対象と同じもの）
LPDIRECTDRAWSURFACE lpScreen;

LPDIRECTDRAW lpDDraw;
LPDIRECTDRAWCLIPPER lpDDClipper;

HWND hwnd;

void DrawFrame(HWND hwnd)
{

	RECT Scrrc={0,0,640,480};   //画面のサイズ 送り側の矩形

	RECT Winrc;   //画面のサイズ 送り側の矩形

	POINT p;
	p.x = p.y = 0;

	//　背景画像を転送する処理
	lpBackbuffer->BltFast(0,0,lpScreen,&Scrrc,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);

	// クライアント領域をスクリーン座標で取得
	ClientToScreen(hwnd, &p);
	GetClientRect(hwnd, &Winrc);
	OffsetRect(&Winrc, p.x, p.y);

	// フリップのような処理
	lpPrimary->Blt(&Winrc,lpBackbuffer,&Scrrc,DDBLT_WAIT,NULL);

}

void LoadBMP(LPDIRECTDRAWSURFACE lpSurface,char *fname)
{
		HBITMAP hBmp=NULL;
		BITMAP bm;
		HDC hdc,hMemdc;
		LPDIRECTDRAWPALETTE lpPal;
		RGBQUAD rgb[256];
		PALETTEENTRY pe[256];
		int i;

		hBmp=(HBITMAP)LoadImage(GetModuleHandle(NULL),fname,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		GetObject(hBmp,sizeof(bm),&bm);

		hMemdc=CreateCompatibleDC(NULL);
		SelectObject(hMemdc,hBmp);

		GetDIBColorTable(hMemdc,0,256,rgb);
		
		for(i=0;i<256;i++){
			pe[i].peRed=rgb[i].rgbRed;
			pe[i].peGreen=rgb[i].rgbGreen;
			pe[i].peBlue=rgb[i].rgbBlue;
			pe[i].peFlags=PC_RESERVED|PC_NOCOLLAPSE;
		}

		lpDDraw->CreatePalette(DDPCAPS_8BIT,pe,&lpPal,NULL);
		lpPrimary->SetPalette(lpPal);

		lpSurface->GetDC(&hdc);
		BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hMemdc,0,0,SRCCOPY);
		lpSurface->ReleaseDC(hdc);

		DeleteDC(hMemdc);
		lpPal->Release();
		DeleteObject(hBmp);
}

LRESULT APIENTRY WndFunc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(msg){
	
		case WM_CREATE:

				break;

		case WM_KEYDOWN:
			switch(wParam){
			//ESCキーで終了
			case VK_ESCAPE:
				//プログラム終了
				//画面モードを元に戻す

				lpScreen->Release();   //プライマリーを開放すればバックバッファも消える

				lpBackbuffer->Release();  //プライマリーを開放すればバックバッファも消える

				lpPrimary->Release();  //プライマリーを開放すればバックバッファも消える

				lpDDraw->Release();

				PostQuitMessage(0);

				return 0;
			}
		case WM_DESTROY:

				lpScreen->Release();   //プライマリーを開放すればバックバッファも消える

				lpBackbuffer->Release();  //プライマリーを開放すればバックバッファも消える

				lpPrimary->Release();  //プライマリーを開放すればバックバッファも消える

				lpDDraw->Release();

		PostQuitMessage( 0 );
		return 0;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR lpszCmdParam,int nCmdshow)
{
		MSG msg;
		DDSURFACEDESC Dds;

		HWND hwnd;
		WNDCLASS wc;
		char szAppName[]="Generic Game SDK Window";

		wc.style=CS_DBLCLKS;
		wc.lpfnWndProc=WndFunc;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=hInst;
		wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName=NULL;
		wc.lpszClassName=szAppName;

		RegisterClass(&wc);

		hwnd=CreateWindowEx(
							0,
							szAppName,
							"Direct X",
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							640,480,
							NULL,NULL,hInst,
							NULL);


		if(!hwnd)return FALSE;

		ShowWindow(hwnd,nCmdshow);
		UpdateWindow(hwnd);
		SetFocus(hwnd);

		DirectDrawCreate(NULL,&lpDDraw,NULL);

		lpDDraw->SetCooperativeLevel(hwnd,DDSCL_NORMAL);

		//基本サーフェスとバッファ１つを作成
		Dds.dwSize=sizeof(Dds);
		Dds.dwFlags=DDSD_CAPS;
		Dds.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
		lpDDraw->CreateSurface(&Dds,&lpPrimary,NULL);

		//背景サーフェスを作成
		Dds.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		Dds.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
		Dds.dwWidth=640;
		Dds.dwHeight=480;
		lpDDraw->CreateSurface(&Dds,&lpBackbuffer,NULL);
		
		//背景サーフェスを作成
		Dds.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		Dds.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
		Dds.dwWidth=640;
		Dds.dwHeight=480;
		lpDDraw->CreateSurface(&Dds,&lpScreen,NULL);

		// クリッパー
		lpDDraw->CreateClipper( 0, &lpDDClipper, NULL );
		lpDDClipper->SetHWnd( 0, hwnd );
		lpPrimary->SetClipper( lpDDClipper );

		//各サーフェスに画像を読み込む
		LoadBMP(lpScreen,"test.BMP");  //背景


		while(1){
			if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
				{
					if(!GetMessage(&msg,NULL,0,0))
						return msg.wParam;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}else{
						//ここにやりたい処理を入れる
						//ウインドウメッセージを殺さないため
							DrawFrame(hwnd);
						
						//一瞬だけOSに制御を戻しておく
							timeEndPeriod(1);
					}
			}
			return msg.wParam;
}