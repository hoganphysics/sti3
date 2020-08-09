#ifndef COSBINDINGNODE_H
#define COSBINDINGNODE_H

#ifndef __CORBA_H_EXTERNAL_GUARD__
#include <omniORB4/CORBA.h>
#endif

#include <vector>
#include <string>
#include <memory>


//COS (Common Object Services) binding node tree object

namespace STI
{
namespace Network
{

class COSBindingNode;

class COSBindingNode
{
public:

	COSBindingNode(const std::string& nodeName);
	COSBindingNode(const std::string& nodeName, CosNaming::NamingContext_var& nodeContext);
	~COSBindingNode();

//	COSBindingNode(const COSBindingNode& copy);
//	COSBindingNode& operator= (const COSBindingNode& other);

	bool hasBranches() const;
//	unsigned int branches() const;

//	COSBindingNode& operator[] (unsigned int i) const;
//	std::unique_ptr<COSBindingNode>& operator[] (unsigned int i) const;

	std::vector<std::unique_ptr<COSBindingNode>>& branches() { return _branches; }

	std::string getName() const;
	bool isDead() const;
	bool isLeaf() const;

	void prune();

	std::string printTree();

private:

	void walkBranches(CosNaming::NamingContext_var& nodeContext);

	std::string printNode(unsigned int offset);
	
	bool allBranchesDead();

	void addBranch(const std::string& nodeName, CosNaming::NamingContext_var& nodeContext);
	void addBranch(const std::string& nodeName);

	std::vector<std::unique_ptr<COSBindingNode>> _branches;

	std::string name;
	bool _isDead;
	bool _isLeaf;

	CosNaming::NamingContext_var context;

};


} //Network
} //STI


#endif

