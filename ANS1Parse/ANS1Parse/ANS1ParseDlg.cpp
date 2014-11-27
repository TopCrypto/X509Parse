// ANS1ParseDlg.cpp : 实现文件
//

#include  "stdafx.h"
#include  "ANS1Parse.h"
#include  "ANS1ParseDlg.h"
#include  "x509.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CANS1ParseDlg 对话框




CANS1ParseDlg::CANS1ParseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CANS1ParseDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CANS1ParseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}

BEGIN_MESSAGE_MAP(CANS1ParseDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CANS1ParseDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CANS1ParseDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CANS1ParseDlg 消息处理程序

BOOL CANS1ParseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//m_Font.CreateFont(
	//	15,							// nHeight
	//	0,							// nWidth
	//	0,							// nEscapement
	//	0,							// nOrientation
	//	FW_BLACK,				// nWeight
	//	FALSE,						// bItalic
	//	FALSE,						// bUnderline
	//	0,							// cStrikeOut
	//	GB2312_CHARSET,				// nCharSet
	//	//ANSI_CHARSET,
	//	OUT_DEFAULT_PRECIS,			// nOutPrecision
	//	CLIP_DEFAULT_PRECIS,		// nClipPrecision
	//	DEFAULT_QUALITY,			// nQuality
	//	DEFAULT_PITCH  | FF_SWISS,	// nPitchAndFamily
	//	_T("仿宋")					// lpszFacename
	//	);

	//m_edit.SetFont(&m_Font);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CANS1ParseDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CANS1ParseDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CANS1ParseDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CANS1ParseDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CFile fp;	
	if(!(fp.Open((LPCTSTR)m_strFilePath.GetBuffer(m_strFilePath.GetLength()),	//open
		CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate ))
		)
	{
		MessageBox("Open File Fail");
		return;
	}
	fp.SeekToEnd();					//file pointer to the end
	DWORD fplength = (DWORD)fp.GetLength();	//get file data length
	UCHAR *FileDataBuffer = new UCHAR[fplength * 2 + 1];
	UCHAR *pTempHeader=FileDataBuffer,*pTempTail=FileDataBuffer;
	int i=0;
	memset(FileDataBuffer, 0x00, fplength * 2 + 1);
	fp.SeekToBegin();
	UINT len = fp.Read(FileDataBuffer, fplength);	//read data for data length
	fp.Close();		//close file
    

	int error_code;
    CString csMsg;
	signed_x509_certificate certificate;
    SetDlgItemText(IDC_EDIT1, "");
     
	// now parse it
	init_x509_certificate( &certificate );

	if ( !( error_code = parse_x509_certificate( FileDataBuffer, len, &certificate ) ) )
	{
		printf( "X509 Certificate:\n" );
		display_x509_certificate( &certificate, csMsg);
		// Assume it's a self-signed certificate and try to validate it that
		switch ( certificate.algorithm )
		{
		case md5WithRSAEncryption:
		case shaWithRSAEncryption:
			if ( validate_certificate_rsa( &certificate,
				&certificate.tbsCertificate.subjectPublicKeyInfo.rsa_public_key ) )
			{
				printf( "Certificate is a valid self-signed certificate.\n" );
			}
			else
			{
				printf( "Certificate is corrupt or not self-signed.\n" );
			}
			break;
		case shaWithDSA:
			if ( validate_certificate_dsa( &certificate ) )
			{
				printf( "Certificate is a valid self-signed certificate.\n" );
			}
			else
			{
				printf( "Certificate is corrupt or not self-signed.\n" );
			}
		}
	}
	m_edit.ReplaceSel(csMsg);
	free_x509_certificate( &certificate );
	
}

void CANS1ParseDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here

	CString lpstrName;
	CString	strFilePath;
	char cLocalDir[1000];
	GetModuleFileName(NULL,   cLocalDir,   sizeof(cLocalDir));	//get local folder path
	CFileDialog	dlgFile(TRUE, NULL, NULL,  OFN_HIDEREADONLY, "Cer Files(*.cer)|*.cer|All Files(*.*)|*.*|| ", NULL);

	dlgFile.m_ofn.lpstrInitialDir = cLocalDir ;
	dlgFile.m_ofn.lpstrTitle =_T("请选择文件");

	if (dlgFile.DoModal() == IDOK)
	{
		lpstrName =  dlgFile.GetPathName( ); 
		char PathName[1024  + 1]={0};
		memcpy(PathName,lpstrName,strlen(lpstrName));	//store file path 

		strFilePath.Format((char *)PathName);	//change format
		SetDlgItemText(IDC_EDIT2,strFilePath.GetBuffer(strFilePath.GetLength()));	//show the path
		m_strFilePath=strFilePath;
	}else
	{
	   SetDlgItemText(IDC_EDIT1 ,"");
       SetDlgItemText(IDC_EDIT2, "");
	}
}
