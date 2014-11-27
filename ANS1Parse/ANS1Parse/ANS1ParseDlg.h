// ANS1ParseDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CANS1ParseDlg 对话框
class CANS1ParseDlg : public CDialog
{
// 构造
public:
	CANS1ParseDlg(CWnd* pParent = NULL);	// 标准构造函数

	CFont m_Font;
// 对话框数据
	enum { IDD = IDD_ANS1PARSE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

    CString m_strFilePath;
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CEdit m_edit;
};
