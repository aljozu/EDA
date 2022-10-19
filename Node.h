//
// Created by lojaz on 14/10/2022.
//

#ifndef LINES_NODE_H
#define LINES_NODE_H
#include "MBB.h"
#include "Figure.h"
#include <iostream>
struct Node{
    //attributes
    MBB boundingBox;
    std::vector<Node*> children;
    Node* father = nullptr;

    //constructors
    Node() = default;
    Node(std::vector<Node*> nodes){
        copy(nodes.begin(), nodes.end(), back_inserter(children));
        mergeBoundingBoxes();
    }
    Node(MBB mbb) : boundingBox{mbb} {}

    //methods
    virtual bool isLeaf(){
        if(children.empty() || (!children.empty() && children[0]->children.empty()))
            return true;
        else{
            return false;
        }
    }

    virtual void draw(RenderWindow &window){
        boundingBox.draw(window);
    }

    void mergeBoundingBoxes(){
        /*
        vector<Vector2f> topLeftPoints;
        vector<Vector2f> bottomRightPoints;

        for(auto &x: children) {
            topLeftPoints.push_back(x->boundingBox.topLeft);
            bottomRightPoints.push_back(x->boundingBox.bottomRight);
        }
        sort(topLeftPoints.begin(), topLeftPoints.end(), sortPointsX);
        sort(bottomRightPoints.begin(), bottomRightPoints.end(), sortPointsY);

        Vector2f topLeft = {topLeftPoints[0].x, bottomRightPoints[bottomRightPoints.size()-1].y};
        Vector2f bottomRight = {topLeftPoints[topLeftPoints.size()-1].x, bottomRightPoints[0].y};
        boundingBox = MBB(topLeft, bottomRight);
         */
        auto auxFig = children[0]->boundingBox;
        for(auto x: children){
           auxFig = mergeTwoBoxes(auxFig, x->boundingBox);
        }
        boundingBox = auxFig;
    }
    MBB mergeTwoBoxes(MBB BB, MBB auxBB){
        Vector2f topleft = {min(BB.topLeft.x, auxBB.topLeft.x),
                            min(BB.topLeft.y, auxBB.topLeft.y)};
        Vector2f bottomRight = {max(BB.bottomRight.x, auxBB.bottomRight.x),
                                max(BB.bottomRight.y, auxBB.bottomRight.y)};
        return {topleft, bottomRight};
    }

    void mergeBB(MBB auxBB){
        if(children.size() == 1){
            boundingBox = auxBB;
        } else
        {
            Vector2f topleft = {min(boundingBox.topLeft.x, auxBB.topLeft.x),
                                min(boundingBox.topLeft.y, auxBB.topLeft.y)};
            Vector2f bottomRight = {max(boundingBox.bottomRight.x, auxBB.bottomRight.x),
                                    max(boundingBox.bottomRight.y, auxBB.bottomRight.y)};
            boundingBox = MBB(topleft, bottomRight);
        }
    }
};

struct leafNode: Node{
    //attributes
    Figure figure;

    //constructors
    leafNode() = default;
    leafNode(Figure figure1) {
        figure=figure1;
        boundingBox = figure1.getBoundingBox();
    }

    //methods
    bool isLeaf() override {return true;}
    void draw(RenderWindow &window) override {
        boundingBox.draw(window);
        figure.draw(window);
    }

    Figure getFigure() {return figure;}
};

#endif //LINES_NODE_H
