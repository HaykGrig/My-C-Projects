

#include "stdafx.h"
#include "AWS_Srv.h"
#include "AWS_SrvExitDlg.h"
#include "afxdialogex.h"



IMPLEMENT_DYNAMIC(AWS_SrvExitDlg, CDialogEx)

AWS_SrvExitDlg::AWS_SrvExitDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(AWS_SrvExitDlg::IDD, pParent)
{

}

AWS_SrvExitDlg::~AWS_SrvExitDlg()
{
}

void AWS_SrvExitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(AWS_SrvExitDlg, CDialogEx)
	ON_BN_CLICKED(IDC_Hide, &AWS_SrvExitDlg::OnBnClickedHide)
	ON_BN_CLICKED(IDC_Close, &AWS_SrvExitDlg::OnBnClickedClose)
END_MESSAGE_MAP()



BOOL AWS_SrvExitDlg::OnInitDialog()
{
	SetWindowText(L"Confirm Action");
	return true;
}

void AWS_SrvExitDlg::OnBnClickedHide()
{
	EndDialog(IDC_Hide);
}


void AWS_SrvExitDlg::OnBnClickedClose()
{
	EndDialog(IDC_Close);
}
