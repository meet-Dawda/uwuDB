#include<iostream>
#include<string>
#include<vector>
#include <memory>
#include <algorithm>
#include<variant>

using cell = std::variant<int, double, std::string>;

struct Row {
    std::vector<cell> columns;
    bool isActive = true; //tombstone
};

struct Node {
    bool isLeaf;
    std::vector<int> keys;
    std::vector<Row> data; //only for leaf node;


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
                checkInvariant(root);
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
            newNode->data.assign(node->data.begin() + mid, node->data.end());
            node->data.erase(node->data.begin() + mid, node->data.end());
            newNode->parent = node->parent;
            int promoteKey = newNode->keys.front();
            promoteToParent(node, promoteKey, newNode);
        }
        void insert(int key, Row data){
            std::shared_ptr<Node> leaf=search(root, key);
            auto it = std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key);
            int index= it-leaf->keys.begin();
            leaf->data.insert(leaf->data.begin() + index, data);
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
        Row dataSearch(int key){
            std::shared_ptr<Node>leaf=search(root,key);
            for (size_t i=0; i<leaf->keys.size(); i++){
                if (leaf->keys[i]==key){
                    if (!leaf->data[i].isActive) {
                        throw std::runtime_error("Key has been deleted!");
                    }
                    return leaf->data[i];
                }
            }
            throw std::runtime_error("Key not found!");
        }

        bool updateRow(int key, Row data){
            std::shared_ptr<Node> leaf = search(root, key);
            for (size_t i=0; i<leaf->keys.size(); i++){
                if (leaf->keys[i] == key){
                    leaf->data[i] = data;
                    return true;
                }
            }
            return false;
        }
        bool remove(int key){
            std::shared_ptr<Node> leaf = search(root, key);
            for (size_t i=0; i<leaf->keys.size(); i++){
                if (leaf->keys[i] == key){
                    leaf->data[i].isActive = false; 
                    return true;
                }
            }
            return false;
        }

};
int main() {
    Tree tree(5); 
    
    Row r1; r1.columns.push_back("Meet");
    tree.insert(10, r1);
    
    std::cout << "Original: " << std::get<std::string>(tree.dataSearch(10).columns[0]) << std::endl;
    
    Row r1_new; r1_new.columns.push_back("Meet Dawda");
    tree.updateRow(10, r1_new);
    std::cout << "Updated:  " << std::get<std::string>(tree.dataSearch(10).columns[0]) << std::endl;
    
    tree.remove(10);
    std::cout << "Deleted key 10." << std::endl;
    
    try {
        tree.dataSearch(10);
    } catch (const std::exception& e) {
        std::cout << "Search result: " << e.what() << std::endl; 
    }

    return 0;
}