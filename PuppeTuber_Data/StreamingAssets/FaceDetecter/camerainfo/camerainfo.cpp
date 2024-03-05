// Direct Show カメラの一覧と解像度を表示

#include <windows.h>
#include <tchar.h>
#include <dshow.h>
#include <stdio.h>
#include <conio.h>

#pragma comment(lib, "strmiids.lib")

int cameraMax=0;	//	カメラ数

//	カメラ一覧を取得
int cameraList(FILE* fp);

//	サポートしている解像度・フレームレートを取得する
void resList(IBaseFilter *pbf , FILE* fp);

//	解像度の情報をファイルに出力
void res_fput(FILE* fp,VIDEOINFOHEADER* video);


int WINAPI _tWinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, TCHAR* CmdLine, int nCmdShow){
	// COMの初期化
	if(FAILED(CoInitialize(NULL))){
		MessageBox(0,_TEXT("COMの初期化に失敗しました"),_TEXT("エラー"),MB_OK);
		return 1;
	}
	FILE* fp;
	if (_tfopen_s(&fp, _TEXT("camera.txt"), _TEXT("w"))){
		MessageBox(0, _TEXT("ログファイルが作成できませんでした"), _TEXT("エラー"), MB_OK);
		return 1;
	}
	::cameraMax=cameraList(fp);
	if(::cameraMax==0){
		_ftprintf(fp,_TEXT("カメラは見つかりませんでした。\n"));
	}
	CoUninitialize(); // COMを終了
	return 0;
}

//	カメラ一覧を取得

int cameraList(FILE* fp){

	// システムデバイス列挙子の作成
	ICreateDevEnum *pDevEnum = NULL;
	CoCreateInstance(CLSID_SystemDeviceEnum, 
	NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum);

	// 列挙子の取得
	IEnumMoniker *pClassEnum = NULL;
	pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if(pClassEnum == NULL){
		pDevEnum->Release();
		return 0;
	}

	ULONG cFetched;
	IMoniker *pMoniker = NULL;
	IBaseFilter *pbf = NULL;
	int n=0;
	while(pClassEnum->Next(1, &pMoniker, &cFetched) == S_OK){
		IPropertyBag *pP = NULL;
		VARIANT var;

		pMoniker->BindToStorage(0,0,IID_IPropertyBag,(void**)&pP);
		var.vt=VT_BSTR;
		pP->Read(_TEXT("FriendlyName"),&var,0);
		//	デバイス名をファイルへ書き込み
		_ftprintf(fp,_TEXT("%s\n"),var.bstrVal);
		VariantClear(&var);

		pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pbf); // モニカをフィルタオブジェクトにバインド
		resList(pbf,fp);

		pMoniker->Release();
		++n;
	}	
	pDevEnum->Release(); // 以後に不要なメモリーをリリース
	pClassEnum->Release();
	return n;
}

//	サポートしている解像度・フレームレートを取得する

void resList(IBaseFilter *pbf , FILE* fp){
	HRESULT hr;

	// キャプチャグラフ作成
	ICaptureGraphBuilder2 *pCapture = NULL;
	CoCreateInstance(CLSID_CaptureGraphBuilder2, 
	NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **) &pCapture);

	// ビデオフォーマットの取得
	IAMStreamConfig *pConfig = NULL;
	hr = pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, 
	0, pbf, IID_IAMStreamConfig, (void**)&pConfig); // インターフェイスのポインタを取得

	int iCount=0;
	int	iSize=0;
	hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if(iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)){// VIDEO_STREAM_CONFIG_CAPS構造体かサイズを確認
		for(int iFormat=0; iFormat<iCount; iFormat++){
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmtConfig;
			hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			VIDEOINFOHEADER *pVih2;
			if(SUCCEEDED(hr)){
				if((pmtConfig->majortype == MEDIATYPE_Video)
					&& (pmtConfig->formattype == FORMAT_VideoInfo)
					&& (pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER))
					&& (pmtConfig->pbFormat != NULL)){

					pVih2 = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					res_fput(fp,pVih2);
				}
			}
		}
	}
	pConfig->Release();
	pCapture->Release();
}

//	解像度の情報をファイルに出力

void res_fput(FILE* fp,VIDEOINFOHEADER* video){
	double ns=100*1.0e-9;
	double frame=1/(double(video->AvgTimePerFrame)*ns);
	_ftprintf(fp,_TEXT("%5d * %5d %5.1ffps\n"), video->bmiHeader.biWidth , video->bmiHeader.biHeight, frame );
}
