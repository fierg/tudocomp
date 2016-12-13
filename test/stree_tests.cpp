
#include "gtest/gtest.h"

#include "test/util.hpp"

#include "tudocomp/ds/SuffixTree.hpp"


//#include "tudocomp/ds/SuffixTreeEdge.hpp"
//#include "tudocomp/ds/SuffixTreeNode.hpp"

//using namespace tdc;
using tdc::SuffixTree;

TEST(stree, st_node_test){
    //tdc::SuffixTree*
    tdc::SuffixTree* stree = new tdc::SuffixTree();

//abcabxabcd$
    stree->add_string("abcabxabcd$");
    ASSERT_EQ(stree->get_text(), "abcabxabcd$");

    tdc::SuffixTree::STNode* root = stree->get_root();
    ASSERT_EQ(6, root->child_nodes.size());
    //check sanity of STREE
    tdc::SuffixTree::STNode* a_node = root->child_nodes['a'];
    ASSERT_EQ(2,a_node->child_nodes.size());
    ASSERT_EQ("ab", stree->get_string_of_edge(a_node));


    tdc::SuffixTree::STNode* b_node = root->child_nodes['b'];

    //sl of a_ndoe should point at b_node:
    ASSERT_EQ(a_node->suffix_link,b_node);


    tdc::SuffixTree::STNode* abc_node = a_node->child_nodes['c'];
    tdc::SuffixTree::STNode* bc_node = b_node->child_nodes['c'];

    ASSERT_EQ(abc_node->suffix_link,bc_node);


    std::vector<tdc::SuffixTree::STNode*> leaves = stree->get_leaves();
    tdc::SuffixTree::STNode* leaf;
    for(uint i =0; i<leaves.size(); i++){
        leaf = leaves.at(i);
        //DLOG(INFO)<<stree->get_text().substr(i);
        DLOG(INFO)<<"edge label of leaf " <<i << ": "<< stree->get_string_of_edge(leaf);
    }


    tdc::SuffixTree::STNode* check;
    tdc::SuffixTree::STNode* check_l;
    tdc::SuffixTree::STNode* check_r;
    //check $
    check =root->child_nodes['$'];
    ASSERT_EQ("$", stree->get_string_of_edge(check));

    ////////////////////////////////////////////////
    //check a

    check =root->child_nodes['a'];
    ASSERT_EQ("ab", stree->get_string_of_edge(check));
    ASSERT_EQ(2, check->child_nodes.size());

    //check children of a

    check_l=check->child_nodes['c'];
    check_r=check->child_nodes['x'];

    ASSERT_EQ("c", stree->get_string_of_edge(check_l));
    ASSERT_EQ("xabcd$", stree->get_string_of_edge(check_r));

    // check children of c of a:
    check = check_l;

    check_l=check->child_nodes['a'];
    check_r=check->child_nodes['d'];

    ASSERT_EQ("abxabcd$", stree->get_string_of_edge(check_l));
    ASSERT_EQ("d$", stree->get_string_of_edge(check_r));

    ////////////////////////////////////////////////
    //check b of root:

    check =root->child_nodes['b'];

    ASSERT_EQ("b", stree->get_string_of_edge(check));
    ASSERT_EQ(2, check->child_nodes.size());

    //check children of b

    check_l=check->child_nodes['c'];
    check_r=check->child_nodes['x'];

    ASSERT_EQ("c", stree->get_string_of_edge(check_l));
    ASSERT_EQ("xabcd$", stree->get_string_of_edge(check_r));

    // check children of c of b:
    check = check_l;

    check_l=check->child_nodes['a'];
    check_r=check->child_nodes['d'];

    ASSERT_EQ("abxabcd$", stree->get_string_of_edge(check_l));
    ASSERT_EQ("d$", stree->get_string_of_edge(check_r));

    ////////////////////////////////////////////////
    //check c of root:

    check =root->child_nodes['c'];

    ASSERT_EQ("c", stree->get_string_of_edge(check));
    ASSERT_EQ(2, check->child_nodes.size());

    //check children of c

    check_l=check->child_nodes['a'];
    check_r=check->child_nodes['d'];

    ASSERT_EQ("abxabcd$", stree->get_string_of_edge(check_l));
    ASSERT_EQ("d$", stree->get_string_of_edge(check_r));

    ////////////////////////////////////////////////
    //check d of root:

    check =root->child_nodes['d'];

    ASSERT_EQ("d$", stree->get_string_of_edge(check));
    ASSERT_EQ(0, check->child_nodes.size());

    ////////////////////////////////////////////////
    //check x of root:

    check =root->child_nodes['x'];

    ASSERT_EQ("xabcd$", stree->get_string_of_edge(check));
    ASSERT_EQ(0, check->child_nodes.size());


}
