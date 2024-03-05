// Direct Show �J�����̈ꗗ�Ɖ𑜓x��\��

#include <windows.h>
#include <tchar.h>
#include <dshow.h>
#include <stdio.h>
#include <conio.h>

#pragma comment(lib, "strmiids.lib")

int cameraMax=0;	//	�J������

//	�J�����ꗗ���擾
int cameraList(FILE* fp);

//	�T�|�[�g���Ă���𑜓x�E�t���[�����[�g���擾����
void resList(IBaseFilter *pbf , FILE* fp);

//	�𑜓x�̏����t�@�C���ɏo��
void res_fput(FILE* fp,VIDEOINFOHEADER* video);


int WINAPI _tWinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, TCHAR* CmdLine, int nCmdShow){
	// COM�̏�����
	if(FAILED(CoInitialize(NULL))){
		MessageBox(0,_TEXT("COM�̏������Ɏ��s���܂���"),_TEXT("�G���["),MB_OK);
		return 1;
	}
	FILE* fp;
	if (_tfopen_s(&fp, _TEXT("camera.txt"), _TEXT("w"))){
		MessageBox(0, _TEXT("���O�t�@�C�����쐬�ł��܂���ł���"), _TEXT("�G���["), MB_OK);
		return 1;
	}
	::cameraMax=cameraList(fp);
	if(::cameraMax==0){
		_ftprintf(fp,_TEXT("�J�����͌�����܂���ł����B\n"));
	}
	CoUninitialize(); // COM���I��
	return 0;
}

//	�J�����ꗗ���擾

int cameraList(FILE* fp){

	// �V�X�e���f�o�C�X�񋓎q�̍쐬
	ICreateDevEnum *pDevEnum = NULL;
	CoCreateInstance(CLSID_SystemDeviceEnum, 
	NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **)&pDevEnum);

	// �񋓎q�̎擾
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
		//	�f�o�C�X�����t�@�C���֏�������
		_ftprintf(fp,_TEXT("%s\n"),var.bstrVal);
		VariantClear(&var);

		pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pbf); // ���j�J���t�B���^�I�u�W�F�N�g�Ƀo�C���h
		resList(pbf,fp);

		pMoniker->Release();
		++n;
	}	
	pDevEnum->Release(); // �Ȍ�ɕs�v�ȃ������[�������[�X
	pClassEnum->Release();
	return n;
}

//	�T�|�[�g���Ă���𑜓x�E�t���[�����[�g���擾����

void resList(IBaseFilter *pbf , FILE* fp){
	HRESULT hr;

	// �L���v�`���O���t�쐬
	ICaptureGraphBuilder2 *pCapture = NULL;
	CoCreateInstance(CLSID_CaptureGraphBuilder2, 
	NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **) &pCapture);

	// �r�f�I�t�H�[�}�b�g�̎擾
	IAMStreamConfig *pConfig = NULL;
	hr = pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, 
	0, pbf, IID_IAMStreamConfig, (void**)&pConfig); // �C���^�[�t�F�C�X�̃|�C���^���擾

	int iCount=0;
	int	iSize=0;
	hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if(iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)){// VIDEO_STREAM_CONFIG_CAPS�\���̂��T�C�Y���m�F
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

//	�𑜓x�̏����t�@�C���ɏo��

void res_fput(FILE* fp,VIDEOINFOHEADER* video){
	double ns=100*1.0e-9;
	double frame=1/(double(video->AvgTimePerFrame)*ns);
	_ftprintf(fp,_TEXT("%5d * %5d %5.1ffps\n"), video->bmiHeader.biWidth , video->bmiHeader.biHeight, frame );
}
