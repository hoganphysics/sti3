
#include <omniORB4/omniURI.h>
#include "COSBindingNode.h"

// #include <iostream>
#include <string>
#include <sstream>
#include <memory>

using STI::Network::COSBindingNode;


COSBindingNode::COSBindingNode(const std::string& nodeName)
: COSBindingNode(nodeName, true)
{
}

COSBindingNode::COSBindingNode(const std::string& nodeName, bool isDead)
{
	//By assumption, this is a leaf, since it has no context.
	name = nodeName;
	_isDead = isDead;
	_isLeaf = true;
}
 
COSBindingNode::COSBindingNode(const std::string& nodeName, CosNaming::NamingContext_var& nodeContext)
{
	name = nodeName;
	_isDead = false;
	_isLeaf = false;

	context = nodeContext;
	
	walkBranches(nodeContext);
}

COSBindingNode::~COSBindingNode()
{
}

bool COSBindingNode::isDead() const
{
	return _isDead;
}

bool COSBindingNode::isLeaf() const
{
	return _isLeaf;
}

void COSBindingNode::prune()
{
	CosNaming::Name_var contextName;

	for (auto& branch : _branches) {
		branch->prune();
	}

	//If all branches are dead, mark this node as dead.  It will later be unbound by its parent node.
	if (allBranchesDead()) {
		_isDead = true;
	}

	//Unbind all dead branches
	for (auto& branch : _branches) {

		if (branch->isDead()) {
			
			contextName = omni::omniURI::stringToName( branch->getName().c_str() );

			context->unbind(contextName);
		}
	}

	//if (isDead()) {
	//	//		unregisterNode();
	//}
}

bool COSBindingNode::allBranchesDead()
{
	bool dead = true;

	for (auto& branch : _branches) {
		dead &= branch->isDead();
	}

	return hasBranches() && dead; // true if all branches of this node are dead
}

void COSBindingNode::walkBranches(CosNaming::NamingContext_var& nodeContext)
{
	CosNaming::NamingContext_var newNodeContext;
	CosNaming::Binding_var binding( new CosNaming::Binding );

	CosNaming::BindingIterator_var biIterVar( new CosNaming::_objref_BindingIterator );
	CosNaming::BindingIterator_out biIter( biIterVar );

	CosNaming::BindingList_var biListVar( new CosNaming::BindingList );
	CosNaming::BindingList_out biList( biListVar );
	
	CORBA::Object_var obj;

	unsigned i = 0;
	
	if(CORBA::is_nil(nodeContext))
	{
		_isLeaf = true;
		
		//No need to iterate through the tree; this is a leaf.
		return;
	}
	
	try {
		nodeContext->list(0, biList, biIter);
	}
	catch(CORBA::TRANSIENT&) {
		_isDead = true;
	}
	catch(CORBA::INV_OBJREF&) {
		_isLeaf = true;
	}
	catch(CORBA::Exception&) {
	}
	catch(...)  {
	//	std::cerr << "Other list exception." << std::endl;
	}

	if( isLeaf() )
	{
		//No need to iterate through the tree; this is a leaf.
		return;
	}

	while(biIter->next_one(binding))
	{
		i++;
		std::string branchName(omni::omniURI::nameToString(binding->binding_name));

		if (binding->binding_type == CosNaming::nobject) {
			addBranch(branchName, false);
			continue;
		}

		try {
			//get the context for this branch and add a new node
			obj = nodeContext->resolve( binding->binding_name );
			newNodeContext = CosNaming::NamingContext::_narrow( obj );
			addBranch(branchName, newNodeContext);
		}
		catch(CORBA::TRANSIENT&)
		{
			addBranch(branchName);
		}
		catch(CORBA::COMM_FAILURE)
		{
			addBranch(branchName);
		}
		catch(CORBA::TIMEOUT&)
		{
			addBranch(branchName);
			//std::cerr << "COSBindingNode CORBA::TIMEOUT" << std::endl;
		}
		catch(CORBA::INV_OBJREF&)
		{
			addBranch(branchName);
		}
		catch(CORBA::Exception&)
		{
			addBranch(branchName);
		}
		catch(...)
		{
			addBranch(branchName);
		}
	}

}

void COSBindingNode::addBranch(const std::string& nodeName, CosNaming::NamingContext_var& nodeContext)
{
	//This is a live leaf

	std::unique_ptr<COSBindingNode> node = std::make_unique<COSBindingNode>(nodeName, nodeContext);

	_branches.push_back(std::move(node));
}

void COSBindingNode::addBranch(const std::string& nodeName)
{
	//This is a dead leaf
	std::unique_ptr<COSBindingNode> node = std::make_unique<COSBindingNode>(nodeName);

	_branches.push_back(std::move(node));
}

void COSBindingNode::addBranch(const std::string& nodeName, bool isDead)
{
	std::unique_ptr<COSBindingNode> node = std::make_unique<COSBindingNode>(nodeName, isDead);

	_branches.push_back(std::move(node));
}

std::string COSBindingNode::getName() const
{
	return name;
}

bool COSBindingNode::hasBranches() const
{
	return ( _branches.size() > 0 );
}



//COSBindingNode& COSBindingNode::operator[] (unsigned int i) const
//{
//	return *( _branches.at(i) );
//}

std::string COSBindingNode::printNode(unsigned int offset)
{
	unsigned i;
	std::stringstream tree;
	
	for (i = 0; i < offset; i++) {
		tree << "*";
	}

	tree << name << " ";
	
	if (isLeaf()) {
		tree << (isDead() ? "(Dead)" : "(Alive)");
	}

	tree << "\n";

	for (auto& branch : _branches) {
		tree << branch->printNode(offset + 1);
	}

	return tree.str();
}

std::string COSBindingNode::printTree()
{
	return printNode(1);
}

void COSBindingNode::getLiveLeafs(const std::string& objectName, std::vector<std::string>& objContexts)
{
	getLiveLeafsFullPath(objectName, "", objContexts);
}

void COSBindingNode::getLiveLeafsFullPath(const std::string& objectName, const std::string& basePath, std::vector<std::string>& objContexts)
{
	if (isLeaf() && !isDead() && getName().compare(objectName) == 0) {
		//found match
		objContexts.push_back(basePath + getName());
	}
	else {
		//check branches recursively
		for (auto& branch : _branches) {
			branch->getLiveLeafsFullPath(objectName, basePath + getName() + "/", objContexts);
		}
	}
}
