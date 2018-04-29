//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TREE_H__
#define __LOMSE_TREE_H__

#include <iostream>
#include <stack>
#include <vector>
#include <iterator>
#include <stdexcept>

#include "lomse_visitor.h"

//---------------------------------------------------------------------------------------
// macro for avoiding warnings when a parameter is not used
#ifdef UNUSED
#elif defined(__GNUC__)
    #define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
    #define UNUSED(x) /*@unused@*/ x
#else
    #define UNUSED(x) /* x */
#endif


namespace lomse
{


template <typename T>
class Tree
{
protected:
	T* m_root;

public:
    Tree() {}
    Tree(T* node) { m_root = node; }

    void set_root(T* node) { m_root = node; }
    T* get_root() { return m_root; }
    T* get_last_node();
    bool is_modified() { return m_root->is_modified(); }
    void clear_modified() { m_root->clear_modified(); }

    //a bidirectional iterator
    class depth_first_iterator
    {
        protected:
            Tree<T>* m_tree;
            T* m_currentNode;

        public:
            depth_first_iterator(Tree<T>* tree)
                : m_tree(tree), m_currentNode(tree->get_root()) {}
            depth_first_iterator(Tree<T>* tree, bool UNUSED(fLast))
                : m_tree(tree), m_currentNode(nullptr)  {}
            depth_first_iterator() : m_tree(nullptr), m_currentNode(nullptr)  {}
            depth_first_iterator(T* node) : m_tree(nullptr), m_currentNode(node)  {}
            virtual ~depth_first_iterator() {}

            void operator =(T* node) { m_currentNode = node; }

	        T* operator *() const { return m_currentNode; }
		    bool operator ==(const depth_first_iterator& it) const {
			    return m_currentNode == it.m_currentNode;
		    }
		    bool operator !=(const depth_first_iterator& it) const {
                return m_currentNode != it.m_currentNode;
            }

            depth_first_iterator& operator ++()
            {
                if (m_currentNode == nullptr)
                {
                    return *this;   //we are at end and will remain there
                }

	            if (m_currentNode->get_first_child() != nullptr)
                {
		            m_currentNode = m_currentNode->get_first_child();
	            }
	            else
                {
                    //proceed with next sibling
                    if (m_currentNode->get_next_sibling())
                        m_currentNode = m_currentNode->get_next_sibling();
                    else
                    {
                        //go back and proceed with next sibling
                        T* node = m_currentNode;
                        while(node && node->get_next_sibling() == nullptr)
                        {
                            node = node->get_parent();
                        }
                        m_currentNode = (node ? node->get_next_sibling() : node);
                    }
                }
	            return *this;
            }

            depth_first_iterator& operator --()
            {
                if (m_currentNode == nullptr)
                {
                    //at end: move to last element
                    if (m_tree != nullptr)
                        m_currentNode = m_tree->get_last_node();
                    return *this;
                }

	            if (m_currentNode->get_prev_sibling() != nullptr)
                {
		            m_currentNode = m_currentNode->get_prev_sibling();
	            }
	            else
                {
                    //proceed with parent
                    m_currentNode = m_currentNode->get_parent();
                }
	            return *this;
            }

    };

    typedef depth_first_iterator iterator;

    depth_first_iterator begin() { return depth_first_iterator(this); }
    depth_first_iterator end() { return depth_first_iterator(this, true); }

    /// Erase a node. Removes from the tree the node and all its descendants.
    /// Returns iterator pointing to next node after the erased one, where 'next'
    /// must be interpreted according traversal method ordering rules
    iterator erase(iterator position);

    iterator replace_node(iterator position, T* newNode);

    /// Insert a node as previous sibling of pointed node. Returns an iterator
    /// that points to the newly inserted element
    iterator insert(iterator position, T* newNode);

    /// Insert a node newNode as previous sibling of given node curNode. Returns an
    /// iterator that points to the newly inserted element
    iterator insert(T* curNode, T* newNode);
};


//---------------------------------------------------------------------------------------
/// A node in the tree. It is a base abstract class from which any tree node must derive.
/// It adds the links to place the node in the tree and provides iterators for traversing
/// the tree.
template<class T>
class TreeNode : public Tree<T>
{
protected:
    T* m_parent;
	T* m_firstChild;
    T* m_lastChild;
	T* m_prevSibling;
    T* m_nextSibling;
    int m_nModified;

    TreeNode() : m_parent(nullptr), m_firstChild(nullptr), m_lastChild(nullptr),
                   m_prevSibling(nullptr), m_nextSibling(nullptr), m_nModified(0) {};

public:
    virtual ~TreeNode() {}

    //getters
    virtual T* get_parent() { return m_parent; }
    virtual T* get_first_child() { return m_firstChild; }
    virtual T* get_last_child() { return m_lastChild; }
    virtual T* get_prev_sibling() { return m_prevSibling; }
    virtual T* get_next_sibling() { return m_nextSibling; }
    bool is_modified() { return m_nModified > 0; }

    //setters
    virtual void set_parent(T* parent) { m_parent = parent; }
    virtual void set_first_child(T* firstChild) { m_firstChild = firstChild; }
    virtual void set_last_child(T* lastChild) { m_lastChild = lastChild; }
    virtual void set_prev_sibling(T* prevSibling) { m_prevSibling = prevSibling; }
    virtual void set_next_sibling(T* nextSibling) { m_nextSibling = nextSibling; }

    virtual void set_parent(TreeNode<T>* parent) { m_parent = dynamic_cast<T*>(parent); }
    virtual void set_first_child(TreeNode<T>* firstChild) { m_firstChild = dynamic_cast<T*>(firstChild); }
    virtual void set_last_child(TreeNode<T>* lastChild) { m_lastChild = dynamic_cast<T*>(lastChild); }
    virtual void set_prev_sibling(TreeNode<T>* prevSibling) { m_prevSibling = dynamic_cast<T*>(prevSibling); }
    virtual void set_next_sibling(TreeNode<T>* nextSibling) { m_nextSibling = dynamic_cast<T*>(nextSibling); }

    void set_modified();
    void reset_modified();
    void clear_modified();

    /// returns 'true' if this node is terminal (doesn't have children)
	virtual bool is_terminal() const { return m_firstChild==nullptr; }

	/// returns true if this node is the root, that is, if there is no parent node
	virtual bool is_root() const { return m_parent == nullptr; }

    //methods related to children
	virtual void append_child(T* child);
    virtual int get_num_children();
    virtual T* get_child(int i);
    virtual void remove_child(T* child);


    class children_iterator
    {
        protected:
            T* m_currentNode;

        public:
            children_iterator() : m_currentNode(nullptr) {}
            children_iterator(T* n) : m_currentNode(n) {}
            children_iterator(TreeNode<T>* n) { m_currentNode =  dynamic_cast<T*>(n); }
            virtual ~children_iterator() {}

	        T* operator *() const { return m_currentNode; }

	        children_iterator& operator ++()
	        {
                if (m_currentNode)
                    m_currentNode = m_currentNode->get_next_sibling();
                return *this;
            }

	        children_iterator& operator --()
	        {
                if (m_currentNode)
                    m_currentNode = m_currentNode->get_prev_sibling();
                return *this;
            }

		    bool operator ==(const children_iterator& it) const {
			    return m_currentNode == it.m_currentNode;
		    }
		    bool operator !=(const children_iterator& it) const {
                return m_currentNode != it.m_currentNode;
            }

    };

    children_iterator begin() { return children_iterator(m_firstChild); }
    children_iterator end() { return children_iterator(); }

};


//---------------------------------------------------------------------------------------
template <class T>
void TreeNode<T>::append_child(T* child)
{
    T* oldLastChild = m_lastChild;

    //links in child
    child->set_parent(this);
    child->set_prev_sibling( m_lastChild );
    child->set_next_sibling( static_cast<T*>(nullptr) );

    //links in parent
    if (!m_firstChild)
        m_firstChild = child;
    m_lastChild = child;

    //links in old last child
    if (oldLastChild)
        oldLastChild->set_next_sibling( child );

    //cout << "Append child ----------------------------------" << endl;
    //cout << "first child: " << m_firstChild << ", last child: " << m_lastChild << endl;
    //cout << "prev sibling: " << m_prevSibling << ", next sibling: " << m_nextSibling << endl;

    //cout << "Added child -----------------------------------" << endl;
    //cout << "first child: " << child->get_first_child() << ", last child: " << child->get_last_child() << endl;
    //cout << "prev sibling: " << child->get_prev_sibling() << ", next sibling: " << child->get_next_sibling() << endl;
    //cout << "-----------------------------------------------" << endl;

    //system("pause");

}

//---------------------------------------------------------------------------------------
template <class T>
int TreeNode<T>::get_num_children()
{
    TreeNode<T>::children_iterator it;
	int numChildren = 0;
    for (it=this->begin(); it != this->end(); ++it)
    {
        //cout << "it=" << (*it).get_p() << endl;
        //cout << "this.end=" << *(this->end()) << endl;
        numChildren++;
    }
    return numChildren;
}

//---------------------------------------------------------------------------------------
template <class T>
T* TreeNode<T>::get_child(int i)
{
    // i = 0..n-1
    children_iterator it(this);
    int numChild = 0;
    for (it=this->begin(); it != this->end() && numChild < i; ++it, ++numChild);
    if (it != this->end() && i == numChild)
    {
        return *it;
    }
    else
        throw std::runtime_error("[TreeNode<T>::get_child]. Num child greater than available children" );
}

//---------------------------------------------------------------------------------------
template <class T>
void TreeNode<T>::remove_child(T* nodeToErase)
{
    //links in previous sibling
    if (nodeToErase->get_prev_sibling())
    {
        T* prevSibling = nodeToErase->get_prev_sibling();
        prevSibling->set_next_sibling( nodeToErase->get_next_sibling() );
    }

    //links in next sibling
    if (nodeToErase->get_next_sibling())
    {
        T* nextSibling = nodeToErase->get_next_sibling();
        nextSibling->set_prev_sibling( nodeToErase->get_prev_sibling() );
    }

    //links in parent
    if (get_first_child() == nodeToErase)
        set_first_child( nodeToErase->get_next_sibling() );
    if (get_last_child() == nodeToErase)
        set_last_child( nodeToErase->get_prev_sibling() );
}

//---------------------------------------------------------------------------------------
template <class T>
void TreeNode<T>::set_modified()
{
    //set new value in all path nodes, from current node to root

    ++m_nModified;
    if (m_parent)
        m_parent->set_modified();
}

//---------------------------------------------------------------------------------------
template <class T>
void TreeNode<T>::reset_modified()
{
    //set new value in all path nodes, from current node to root

    --m_nModified;
    if (m_parent)
        m_parent->reset_modified();
}

//---------------------------------------------------------------------------------------
template <class T>
void TreeNode<T>::clear_modified()
{
    //reset value in all path nodes, downwards from current node

    m_nModified = 0;
    children_iterator it(this);
    for (it=begin(); it != end(); ++it)
    {
        if ((*it)->is_modified())
            (*it)->clear_modified();
    }
}

//---------------------------------------------------------------------------------------
template <class T>
T* Tree<T>::get_last_node()
{
    TreeNode<T>* node = get_root();
    if (node == nullptr)
        return nullptr;

    TreeNode<T>* child = node->get_last_child();
    while (child != nullptr)
    {
        node = child;
        child = node->get_last_child();
    }
    return dynamic_cast<T*>(node);
}

//---------------------------------------------------------------------------------------
template <class T>
typename Tree<T>::depth_first_iterator Tree<T>::erase(depth_first_iterator position)
{
    T* nodeToErase = *position;

    //links in previous sibling
    if (nodeToErase->get_prev_sibling())
    {
        T* prevSibling = nodeToErase->get_prev_sibling();
        prevSibling->set_next_sibling( nodeToErase->get_next_sibling() );
    }

    //links in next sibling
    if (nodeToErase->get_next_sibling())
    {
        T* nextSibling = nodeToErase->get_next_sibling();
        nextSibling->set_prev_sibling( nodeToErase->get_prev_sibling() );
    }

    //links in parent
    if (!nodeToErase->is_root())
    {
        T* parent = nodeToErase->get_parent();
        if (parent->get_first_child() == nodeToErase)
            parent->set_first_child( nodeToErase->get_next_sibling() );
        if (parent->get_last_child() == nodeToErase)
            parent->set_last_child( nodeToErase->get_prev_sibling() );
    }

    //determine next node after deleted one
    if (nodeToErase->get_next_sibling())
        return nodeToErase->get_next_sibling();
    else
    {
        //go up and take next sibling
        if (!nodeToErase->is_root())
            return nodeToErase->get_parent()->get_next_sibling();
        else
        {
            set_root(nullptr);
            return end();
        }
    }
}

//---------------------------------------------------------------------------------------
/// Replaces iterator pointed node (and all its sub-tree) by the received node (and
/// its sub-tree). Return iterator pointing to new node
template <class T>
typename Tree<T>::depth_first_iterator Tree<T>::replace_node(depth_first_iterator position,
                                                             T* newNode)
{
    T* nodeToReplace = *position;

    //links in new node
    newNode->set_parent( nodeToReplace->get_parent() );
    newNode->set_prev_sibling( nodeToReplace->get_prev_sibling() );
    newNode->set_next_sibling( nodeToReplace->get_next_sibling() );

    //links in previous sibling
    if (nodeToReplace->get_prev_sibling())
    {
        T* prevSibling = nodeToReplace->get_prev_sibling();
        prevSibling->set_next_sibling( newNode );
    }

    //links in next sibling
    if (nodeToReplace->get_next_sibling())
    {
        T* nextSibling = nodeToReplace->get_next_sibling();
        nextSibling->set_prev_sibling( newNode );
    }

    //links in parent
    if (!nodeToReplace->is_root())
    {
        T* parent = nodeToReplace->get_parent();
        if (parent->get_first_child() == nodeToReplace)
            parent->set_first_child( newNode );
        if (parent->get_last_child() == nodeToReplace)
            parent->set_last_child( newNode );
    }
    else
        set_root(newNode);

    return newNode;
}


//---------------------------------------------------------------------------------------
/// Insert a node as previous sibling of pointed node. Returns an iterator
/// that points to the newly inserted element
template <class T>
typename Tree<T>::depth_first_iterator Tree<T>::insert(depth_first_iterator position,
                                                       T* newNode)
{
    T* curNode = *position;
    if (curNode->is_root()) return position;

    return insert(curNode, newNode);
}


//---------------------------------------------------------------------------------------
/// Insert a node newNode as previous sibling of given node curNode. Returns an iterator
/// that points to the newly inserted element
template <class T>
typename Tree<T>::depth_first_iterator Tree<T>::insert(T* curNode, T* newNode)
{
    if (curNode->is_root()) return curNode;

    //links in previous sibling
    if (curNode->get_prev_sibling())
    {
        T* prevSibling = curNode->get_prev_sibling();
        prevSibling->set_next_sibling( newNode );
    }

    //links in new node
    newNode->set_parent( curNode->get_parent() );
    newNode->set_prev_sibling( curNode->get_prev_sibling() );
    newNode->set_next_sibling( curNode );

    //links in curNode
    curNode->set_prev_sibling( newNode );

    //links in parent
    T* parent = curNode->get_parent();
    if (parent->get_first_child() == curNode)
        parent->set_first_child( newNode );

    return newNode;
}


}   //namespace lomse

#endif      //__LOMSE_TREE_H__
