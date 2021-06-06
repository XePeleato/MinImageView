//---------------------------------------------------------------------------

#ifndef InoTreeNodeH
#define InoTreeNodeH
#include "..\fda\usr\src\include\dirent.h"
class InoTreeNode : TTreeNode {
public:
	int inode;
	int tam;
	char nombre[DIRSIZ];
	int tipo;
	long mod;
    InoTreeNode *hijos;
};
//---------------------------------------------------------------------------
#endif
