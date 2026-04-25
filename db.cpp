#include<iostream>
#include<string>
#include<vector>
#include <memory>
#include <algorithm>


struct Node {
    bool isLeaf;
    std::vector<int> keys;
    std::vector<std::shared_ptr<Node>> children;  
    std::shared_ptr<Node>next_leaf;
    std::weak_ptr<Node> parent;
    Node(bool leaf = false) : isLeaf(leaf), next_leaf(nullptr){};
};

class Tree {
    private:
        int order;
        std::shared_ptr<Node> root;
    public:
        Tree(int m): order(m), root(std::make_shared<Node>(true)) {};
        void checkInvariant(const std::shared_ptr<Node>& node) {
        #ifndef NDEBUG
            if (!node->isLeaf) {
                if (node->children.size() != node->keys.size() + 1) {
                    throw std::logic_error("Invariant violated");
                }
            }
        #endif
        }
        std::shared_ptr<Node> findParent(std::shared_ptr<Node> curr, std::shared_ptr<Node>child){
            if (curr->isLeaf || curr->children.empty()) return nullptr;
            for (size_t i=0; i<curr->children.size(); i++){
                if (curr->children[i]==child)return curr;
                std::shared_ptr<Node>res=findParent(curr->children[i], child);
                if (res) return res;
            }
            return nullptr;
        }
        void splitInternal(std::shared_ptr<Node> node) {
            int mid = order / 2;
            auto newInternal = std::make_shared<Node>(false);
            int promoteKey = node->keys[mid];
            newInternal->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
            node->keys.erase(node->keys.begin() + mid, node->keys.end());

            newInternal->children.assign(node->children.begin() + mid + 1, node->children.end());
            node->children.erase(node->children.begin() + mid + 1, node->children.end());
            for (auto &child : newInternal->children) {
                child->parent = newInternal;
            }
            newInternal->parent = node->parent;
            #ifndef NDEBUG
            if (node->children.size() != node->keys.size() + 1) {
                throw std::logic_error("Invariant violated (left internal)");
            }
            if (newInternal->children.size() != newInternal->keys.size() + 1) {
                throw std::logic_error("Invariant violated (right internal)");
            }
            #endif
            promoteToParent(node, promoteKey, newInternal);
        }
       
        void promoteToParent(std::shared_ptr<Node> left, int key, std::shared_ptr<Node> right){
            if (left == root){
                auto newRoot = std::make_shared<Node>(false);
                newRoot->keys.push_back(key);
                newRoot->children.push_back(left);
                newRoot->children.push_back(right);
                left->parent = newRoot;
                right->parent = newRoot;
                root = newRoot;
                return;
            }

            auto parent = left->parent.lock();
            if (!parent) {
                throw std::runtime_error("Broken parent pointer");
            }
            auto it = std::upper_bound(parent->keys.begin(), parent->keys.end(), key);
            int index = it - parent->keys.begin();
            parent->keys.insert(it, key);
            parent->children.insert(parent->children.begin() + index + 1, right);
            right->parent = parent;

            #ifndef NDEBUG
            if (!parent->isLeaf) {
                if (parent->children.size() != parent->keys.size() + 1) {
                    throw std::logic_error("Invariant violated: children != keys+1");
                }
            }
            #endif

            if (parent->keys.size() == order){
                splitInternal(parent);
            }
        }
       
        void splitLeaf(std::shared_ptr<Node> node){
            int mid = (order + 1) / 2;
            auto newNode = std::make_shared<Node>(true);
            newNode->keys.assign(node->keys.begin() + mid, node->keys.end());
            node->keys.erase(node->keys.begin() + mid, node->keys.end());
            newNode->next_leaf = node->next_leaf;
            node->next_leaf = newNode;
            newNode->parent = node->parent;
            int promoteKey = newNode->keys.front();
            promoteToParent(node, promoteKey, newNode);
        }
        void insert(int key){
            std::shared_ptr<Node> leaf=search(root, key);
            auto it = std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key);
            leaf->keys.insert(it, key);
            if (leaf->keys.size() == order){
                splitLeaf(leaf);
            }
        }
        std::shared_ptr<Node> search(const std::shared_ptr<Node>& root, int key){
            //TODO:Implement the search function
            if (root->isLeaf){return root;}
            for (size_t i=0; i<root->keys.size(); i++){
                if (key < root->keys[i]) {
                   return search(root->children[i], key);
                }
            } 
            return search(root->children.back(), key);
        }
        void printTree(const std::shared_ptr<Node>& node, int level=0){
            if (!node) return;
            for (int i = 0; i < level; i++) {
                std::cout << "  ";
            }
            for (size_t i = 0; i < node->keys.size(); i++) {
                std::cout << node->keys[i] << " ";
            }
            std::cout << std::endl;
            if (!node->isLeaf) {
                for (const auto& child : node->children) {
                    printTree(child, level + 1);
                }
            }
        }
        void display() {
            printTree(root, 0);
        }
};

int main() {
    Tree tree(3); 
    int values[] = {10, 20, 5, 15, 25, 30, 1};
    for(int v : values) {
        std::cout << "Inserting " << v << "..." << std::endl;
        tree.insert(v);
    }
    
    std::cout << "\nB+ Tree structure (Level Orderish):" << std::endl;
    tree.display();
    return 0;
}