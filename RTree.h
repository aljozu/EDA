//
// Created by lojaz on 14/10/2022.
//

#ifndef LINES_RTREE_H
#define LINES_RTREE_H
#define M 3
#define m 2
#include "Node.h"
#include <limits>
#include <queue>

static bool orderByLowestDistance(pair<Node*, double> p1, pair<Node*, double> p2){
    return p1.second < p2.second;
}
static bool orderByHighestDistance(pair<Node*, double> p1, pair<Node*, double> p2){
    return p1.second > p2.second;
}

bool isPowerOfTwo(int n)
{
    if (n == 0)
        return false;

    return (ceil(log2(n)) == floor(log2(n)));
}

MBB pickLowerSum(vector<MBB> boxes){
    MBB temp;
    double sum = 0;
    for(auto x: boxes){
        auto sum2 = x.topLeft.x + x.topLeft.y;
        if(sum2 < sum){
            sum = sum2;
            temp.bottomRight = x.bottomRight;
            temp.topLeft = x.topLeft;
        }
    }
    return temp;
}

MBB pickHighestSum(vector<MBB> boxes){
    MBB temp;
    double sum = 0;
    for(auto x: boxes){
        auto sum2 = x.bottomRight.x + x.bottomRight.y;
        if(sum2 < sum){
            sum = sum2;
            temp.bottomRight = x.bottomRight;
            temp.topLeft = x.topLeft;
        }
    }
    return temp;
}

double getDistance(Vector2f p1, Vector2f p2){
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y) );
}

class RTree
{
    Node* root;

    Node* split(Node* auxNode){
        vector<MBB> boxes;
        vector<Node*> n1, n2;

        for(auto x: auxNode->children)
            boxes.push_back(x->boundingBox);

        auto firstSeed = pickLowerSum(boxes);
        auto secondSeed = pickHighestSum(boxes);
        vector<pair<Node*, double>> lowerEntries;
        vector<pair<Node*, double>> highestEntries;
        for(auto x: auxNode->children){
            lowerEntries.emplace_back(x, getDistance(firstSeed.topLeft, x->boundingBox.bottomRight));
        }
        auxNode->children.clear();
        sort(lowerEntries.begin(), lowerEntries.end(), orderByLowestDistance);
        for(int i = 0; i < lowerEntries.size(); ++i){
            if(i < m){
                auxNode->children.push_back(lowerEntries[i].first);
            }else{
                highestEntries.emplace_back(lowerEntries[i].first, getDistance(secondSeed.bottomRight, lowerEntries[i].first->boundingBox.topLeft));
            }
        }
        sort(lowerEntries.begin(), lowerEntries.end(), orderByHighestDistance);
        for(auto x: highestEntries)
            n2.push_back(x.first);
        cout << "partition 1: \n";
        for(auto x: n2)
            //cout << '{' << x->boundingBox.topLeft.x << x->boundingBox.topLeft.y << '}'
        cout << " {" << x->boundingBox.bottomRight.x << ' ' << x->boundingBox.bottomRight.y << "} \n";
        cout << "partition 2: \n";
        for(auto x: auxNode->children)
           // cout << '{' << x->boundingBox.topLeft.x << x->boundingBox.topLeft.y << '}'
            cout      << " {" << x->boundingBox.bottomRight.x << ' ' <<  x->boundingBox.bottomRight.y << "} \n";
        return new Node(n2);
        //return make_pair(n1, n2);
    }

    void handleOverflow(Node* node){
        Node* u = split(node);
        u->mergeBoundingBoxes();
        node->mergeBoundingBoxes();
        if(!node->father) {
            auto newRoot = new Node();
            node->father = u->father = newRoot;
            newRoot->children.push_back(node);
            newRoot->children.push_back(u);
            newRoot->mergeBoundingBoxes();
        } else {
            auto w = node->father;
            u->father = w;
            w->children.push_back(u);
            w->mergeBoundingBoxes();
            if(w->children.size() == M + 1){
                cout << "entra";
                handleOverflow(w);
                w->mergeBoundingBoxes();
            }
        }
    }

    Node* chooseSubTree(Node* node, Figure figure){
        float minPerimeter = std::numeric_limits<float>::max();
        auto ans = new Node();
        cout << "nodeeee perimeter "<<node->boundingBox.perimeter() << "\n";
        for(auto n: node->children){
            cout << "n perimeter "<<n->boundingBox.perimeter() << "\n";

            auto auxBB = figure.getBoundingBox();
            auxBB.merge(n->boundingBox);
            auto p = auxBB.perimeter();
            cout << "Perimeter : " << p << endl;
            if(p == 0) return n;
            if(p < minPerimeter){
                minPerimeter = p;
                ans = n;
            }
        }
        cout << "ans perimeter "<<ans->boundingBox.perimeter() << "\n";
        return ans;
    }
    Node* insertRect(Node* node, Figure figure){
        if(node->isLeaf()){
            cout << '{' << figure.boundingBox.topLeft.x << " " << figure.boundingBox.topLeft.y << "} \n";
            cout << "node perimeter " << node->boundingBox.perimeter() << endl;
            auto auxNode = new leafNode(figure);
            //cout << '{' << auxNode->boundingBox.topLeft.x << " " << auxNode->boundingBox.topLeft.y << "} \n";
            (node->children).push_back(auxNode);
            //cout << "vector size " << node->children.size() << '\n';
            auxNode->father = node;
            node->mergeBoundingBoxes();
           // node->mergeBB(figure.boundingBox);
           // cout << "Merged Bound: \n";
            //cout << '{' << node->boundingBox.topLeft.x << " " << node->boundingBox.topLeft.y << ' '
                // << node->boundingBox.bottomRight.x << " " << node->boundingBox.bottomRight.y <<  '}' << '\n';
            if(node->children.size() == M+1) {
                handleOverflow(node);
            }
        } else {
            auto bestSubTree = chooseSubTree(node, figure);
            cout << "bestsubtree perimeter : " << bestSubTree->boundingBox.perimeter() << "\n";
            insertRect(bestSubTree, figure);
            node->mergeBoundingBoxes();
            //node->mergeBB(figure.boundingBox);
        }
        if(node->father)
            return node->father;
        return node;
    }
public:

    RTree(){
        root = new Node();
    }

    void insert(Figure figure){
        root = insertRect(root, figure);
    }

    void knnSearch(Vector2f point, int n){

    }

    void bfs(RenderWindow &renderWindow){
        queue<Node*> q;
        q.push(root);
        int i = 0;
        while(!q.empty()){
            ++i;
            Node* no = q.front();
            //cout << '{' << no->boundingBox.topLeft.x << " " << no->boundingBox.topLeft.y << ' '
            //<< no->boundingBox.bottomRight.x << " " << no->boundingBox.bottomRight.y <<  '}' << '\n';
            //if(no->isLeaf())


            //if (isPowerOfTwo(i))
                //no->boundingBox.color = Color(255, 0, i*100);
                if(no != root)
                    no->draw(renderWindow);
            q.pop();
            for(auto &x: no->children){
                if(x != nullptr)
                    q.push(x);
            }
        }
        //cout << "===============================================\n";
    }
};


#endif //LINES_RTREE_H
