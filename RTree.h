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


static float distanceMBB(Vector2f p, MBB node){
    auto dx = max({node.topLeft.x - p.x, float(0), p.x - node.bottomRight.x});
    auto dy = max({node.topLeft.y - p.y, float(0), p.y - node.bottomRight.y});
    return sqrt(dx*dx + dy*dy);
}

struct LessThanByMBB {
    bool operator()(Figure* f1, Figure*f2, Vector2f p) const
    {
        MBB m1 = f1->getBoundingBox();
        MBB m2 = f2->getBoundingBox();
        return distanceMBB(p, m1) < distanceMBB(p, m2);
    }
};

struct compareNodes{
    bool operator()(pair<Node*, float> n1, pair<Node*, float> n2){
        return n1.second > n2.second;
    }
};

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
float distanceNodePoint(Node* node, Vector2f p){
    MBB box = node->boundingBox;
    float centerX = (box.topRight.x - box.topLeft.x) / 2;
    float centerY = (box.bottomLeft.y - box.topLeft.y) / 2;
    return sqrt((centerX - p.x)*(centerX - p.x) + (centerY - p.y) * (centerY - p.y));
}

float distancePointFigure(Vector2f p, Node* node){
    auto dx = max({node->boundingBox.topLeft.x - p.x, float(0), p.x - node->boundingBox.bottomRight.x});
    auto dy = max({node->boundingBox.topLeft.y - p.y, float(0), p.y - node->boundingBox.bottomRight.y});
    return sqrt(dx*dx + dy*dy);
}

class RTree
{
    Node* root;
    float dk = 0;
    float minRad = 100000;

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
        for(auto x: n2)
        for(auto x: auxNode->children)

        return new Node(n2);
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
                handleOverflow(w);
                w->mergeBoundingBoxes();
            }
        }
    }

    Node* chooseSubTree(Node* node, Figure figure){
        float minPerimeter = std::numeric_limits<float>::max();
        auto ans = new Node();
        for(auto n: node->children){
            auto auxBB = figure.getBoundingBox();
            auxBB.merge(n->boundingBox);
            auto p = auxBB.perimeter();
            if(p == 0) return n;
            if(p < minPerimeter){
                minPerimeter = p;
                ans = n;
            }
        }
        return ans;
    }

    Node* insertRect(Node* node, Figure figure){
        if(node->isLeaf()){
            auto auxNode = new leafNode(figure);
            (node->children).push_back(auxNode);
            auxNode->father = node;
            node->mergeBoundingBoxes();
            if(node->children.size() == M+1) {
                handleOverflow(node);
            }
        } else {
            auto bestSubTree = chooseSubTree(node, figure);
            insertRect(bestSubTree, figure);
            node->mergeBoundingBoxes();
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

    vector<Figure> knnSearch(Vector2f point, int n){
        priority_queue<pair<Node*, float>, vector<pair<Node*, float>>, compareNodes> pq;
        pq.push({root, distancePointFigure(point, root)});
        vector<Figure> ans;
        int i = 0;
        while(!pq.empty()){
            auto top = pq.top();
            if(top.first->isLeaf()){
                ++i;
                for(auto z: top.first->children)
                {
                    //leafNode *ln;
                    auto ln = dynamic_cast<leafNode *> (z);
                    ans.push_back(ln->getFigure());
                }
            }
            if(i == n) break;
            pq.pop();
            for(auto &x: top.first->children){
                if(x != nullptr)
                    pq.push({x, distancePointFigure(point, x)});
            }
        }
        return ans;
    }



    template<typename cmp>
    void k_depthFirst(std::priority_queue<Figure , std::vector<Figure>, cmp> &pq, int &k, Node* node, Vector2f p){


        if(node->isLeaf()){
            for(auto f: node->children){
                if(dk + distanceNodePoint(f, p) < distancePointFigure(p, f))
                    continue;
                else
                {
                    pq.push(dynamic_cast<leafNode *>(f)->getFigure());
                    if (distancePointFigure(p, f) > dk)
                        dk = distancePointFigure(p, f);
                    if (pq.size() > (unsigned) k) pq.pop();
                }
            }
            return;
        }


        for(auto r: node->children){
            auto rad = r->getRadio();
            if(rad < minRad)
                minRad = rad;
            if(dk + distanceNodePoint(r, p) < minRad)
                continue;
            else
                k_depthFirst(pq,k,r, p);
        }
    }

    vector<Figure> depthFirst(Vector2f p, int k){
        std::vector<Figure> ans;
        auto cmp  = [&p](Figure f1, Figure f2){
            MBB m1 = f1.getBoundingBox();
            MBB m2 = f2.getBoundingBox();
            return distanceMBB(p, m1) < distanceMBB(p, m2);
        };
        std::priority_queue<Figure , std::vector<Figure>, decltype(cmp) > pq(cmp);
        k_depthFirst(pq,k,root, p);
        while(!pq.empty()){
            auto t = pq.top();
            ans.push_back(t);
            pq.pop();
        }
        return ans;
    }


    void drawLinesToFoundFigures(vector<Figure> figures, RenderWindow& renderWindow, CircleShape circle){
        for (int i = 0; i < figures.size(); ++i)
        {
            sf::Vertex vertices[2] = {Vertex(circle.getPosition()), Vertex(figures[i].getBoundingBox().topLeft, Color::Blue)};
            renderWindow.draw(vertices, 2, sf::Lines);
        }
    }

    void bfs(RenderWindow &renderWindow){
        queue<Node*> q;
        q.push(root);
        int i = 0;
        while(!q.empty()){
            ++i;
            Node* no = q.front();
            if(no != root)
               no->draw(renderWindow);
            q.pop();
            for(auto &x: no->children){
                if(x != nullptr)
                    q.push(x);
            }
        }
    }
};


#endif //LINES_RTREE_H
