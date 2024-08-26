//---------------------------------------------------------------------------

#ifndef InoTreeNodeH
#define InoTreeNodeH
#include <vcl.h>
#include "..\fda\usr\src\include\dirent.h"
#include "Driver\DirEntry.h"
class InoTreeNode : public TTreeNode {
public:
	int inode;
	minixfs::DirEntry *entry;
	int tam;
	char nombre[DIRSIZ];
	int tipo;
	long mod;
	InoTreeNode *hijos;
    std::filesystem::path path;
};
//---------------------------------------------------------------------------
#endif
