// UnloopLinkList.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


struct node
{
    node() : next(NULL) {}
    virtual ~node() { next = NULL; }
    node* nextNode() const { return this ? next : NULL; }
    node* lastNode(const node *endNode = NULL) const;
    void remove();
    int count(const node *endNode = NULL);
    node *next;
};

int node::count(const node *endNode /* = NULL */)
{
    node *root(this);
    int nNode(0);
    while (root && root != endNode) {
        ++nNode;
        root = root->nextNode();
    }
    return nNode;
}
void node::remove()
{
    node *root(this);
    while (root)
    {
        node *nn(root->nextNode());
        delete root;
        root = nn;
    }
}
node* node::lastNode(const node *endNode /* = NULL */) const
{
    node *lastNode((node*)this);
    while (lastNode && lastNode->nextNode() != endNode)
        lastNode = lastNode->nextNode();
    return lastNode;
}

#define SKIP_NNODES(r,n) {int s=n; while(r&&s-->0) r=r->nextNode();}

node* hasLoop(node *root)
{
    // Standard loop detection algo by taking two pointers and moving the second with double speed
    // till they meet
    node *doubleS(root->nextNode()->nextNode());
    while (root != doubleS) {
        root = root->nextNode();
        doubleS = doubleS->nextNode()->nextNode();
    }
    // return non-null if the given link list contains loop otherwise null
    return root;
}

node* unloop(node *root)
{
    node *loopingNode(hasLoop(root));

    if (loopingNode != NULL) {
        // We found a loop
        // Split the list into two parts
        // one: root to loopingNode
        // second: loopingNode->next to loopingNode
        // count nodes in first list (root)
        int rc(root->count(loopingNode));
        node *preVNode(loopingNode);
        loopingNode = loopingNode->nextNode();
        // count second list
        int lc(loopingNode->count(preVNode));
        // Skip first if it is greater
        SKIP_NNODES(root, rc - lc);
        // skip second if it is greater and keep track of previous node
        for (int c(lc - rc); c > 0; c--) {
            preVNode = loopingNode;
            loopingNode = loopingNode->nextNode();
        }
        // traverse both first and second list till they point to same node
        while (loopingNode != root)
        {
            root = root->nextNode();
            preVNode = loopingNode;
            loopingNode = loopingNode->nextNode();
        }
        // previous node in the second list is the actual last pointer in the list
        preVNode->next = NULL;
        loopingNode = preVNode;
    }
    return loopingNode;
}

struct DataNode : public node
{
    DataNode(char ch=0) : data(ch) {}
    char data;
    DataNode* find(char ch);
    DataNode* add(char ch);
};

DataNode* DataNode::find(char ch)
{
    DataNode *root(this);
    while (root)
        if (root->data == ch)
            return root;
        else
            root = (DataNode*)root->nextNode();
    return NULL;
}
DataNode* DataNode::add(char ch)
{
    DataNode *root((DataNode *)lastNode());
    DataNode *newData = new DataNode(ch);
    if (root)
        root->next = newData;
    return newData;
}

static DataNode* buildFromString(const char *data)
{
    DataNode *root(NULL);
    while (*data) {
        DataNode *dn(root->find(*data));
        if (dn) {
            root->lastNode()->next = dn;
            break;
        }
        dn = root->add(*data++);
        if (!root)
            root = dn;
    }
    return root;
}
static void printDataNode(const DataNode *dn, const TCHAR *title = _T("DataNode"))
{
    _tprintf(_T("%s\n"), title);
    while (dn) {
        _tprintf(_T("%c"), dn->data);
        dn = (const DataNode *)dn->nextNode();
    }
    _tprintf(_T("\n"));
}

int _tmain(int argc, _TCHAR* argv[])
{
    while (1) {
        _tprintf(_T("Enter linklist:"));
        char ll[256] = { 0 };
        scanf_s("%s", ll, (int)_countof(ll));
        DataNode *dn(buildFromString(ll));
        DataNode *ln((DataNode *)unloop(dn));
        _tprintf(_T("loop node: %c\n"), ln ? ln->data : ' ');
        printDataNode(dn);
        dn->remove();
        dn = NULL;
    }
    return 0;
}

